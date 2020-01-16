import re
import os
import sys

args = sys.argv[1:]
objdir = args[0] or os.environ.get("TOP_OBJDIR", ".")
topdir = "."

configpath = objdir + "/config.status"
inputpath = topdir + "/mozilla-config.h.in"
outputpath = objdir + "/mozilla-config.h"


def findLineNumber(lines, regex, start=0):
    r = re.compile(regex, re.U)
    for i in range(start, len(lines)):
        l = lines[i]
        match = r.search(l)
        if match:
            return i


with open(configpath, 'rU') as configf:
    lines = configf.readlines()
    bindex = findLineNumber(lines, "^defines = encode\({$") + 1  # 826
    eindex = findLineNumber(lines, "^}, encoding\)$", bindex)  # 940
    lines = lines[bindex:eindex]
    jsondata = "{" + "".join(lines) + "}"
    config = dict(eval(jsondata))  # unsafe

output = []

with open(inputpath, 'rU') as inputf:
    inputlines = inputf.read().split("\n")
    r = re.compile(
        '^\s*#\s*(?P<cmd>[a-z]+)(?:\s+(?P<name>\S+)(?:\s+(?P<value>\S+))?)?', re.U)
    for l in inputlines:
        m = r.match(l)
        if m:
            cmd = m.group('cmd')
            name = m.group('name')
            value = m.group('value')
            if name:
                if name == 'ALLDEFINES':
                    if cmd == 'define':
                        raise Exception(
                            '`#define ALLDEFINES` is not allowed in a '
                            'CONFIGURE_DEFINE_FILE')

                    def define_for_name(name, val):
                        """WebRTC files like to define WINVER and _WIN32_WINNT
                        via the command line, which raises a mass of macro
                        redefinition warnings.  Just handle those macros
                        specially here."""
                        define = "#define {name} {val}".format(
                            name=name, val=val)
                        if name in ('WINVER', '_WIN32_WINNT'):
                            return '#if !defined({name})\n{define}\n#endif' \
                                .format(name=name, define=define)
                        return define
                    defines = '\n'.join(sorted(
                        define_for_name(name, val)
                        for name, val in config.iteritems()))
                    l = l[:m.start('cmd') - 1] \
                        + defines + l[m.end('name'):]

        output.append(l)

with open(outputpath, 'wb+') as outputf:
    outputf.write("\n".join(output))
