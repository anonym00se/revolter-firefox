/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "re.h"

#if defined(__linux__)
#include <errno.h>
#include <libgen.h>
#include <unistd.h>

#elif defined(_WIN32)
#include <io.h>
#include <windows.h>

#define CP_GB18030 54936

#endif

#ifndef ssize_t
typedef intptr_t ssize_t;
#endif

#ifndef EINVAL
#define EINVAL 22
#endif

#ifndef getline
ssize_t getline(char** lineptr, size_t* n, FILE* stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = malloc(128);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while (c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char* new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char*)(*lineptr))[pos++] = c;
        if (c == '\n') {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}
#endif

#include "sni-config.h"

ConfigItem defaultConfig[] = {
    {true, "*.ord02.abs.hls.ttvnw.net", BYPASS},
    {false, "play.google.com", REPLACE, "www.google.com"},
    {true, ".*.thepiratebay.org", REPLACE, "ssl778318.cloudflaressl.com"},
    {true, ".*.urbandictionary.com", REPLACE, "d.ssl.fastly.net"},
    {true, ".*.youtube.com", REPLACE, "www.youtubeeducation.com"},
    {true, ".*.googlevideo.com", REPLACE, "www.youtubeeducation.com"},
    {true, ".*.ytimg.com", REPLACE, "www.youtubeeducation.com"},
    {true, ".*.reddit.com", REPLACE, "redditinc.com"},
    {true, ".*.redd.it", REPLACE, "redditinc.com"},
    {true, ".*.redditmedia.com", REPLACE, "redditinc.com"},
    {true, ".*.redditstatic.com", REPLACE, "redditinc.com"},
    {true, ".*.reddituploads.com", REPLACE, "redditinc.com"},
    {true, ".*.twitch.tv", REPLACE, "amazonaws.com"},
    {true, ".*.pinterest.com", REPLACE, "pinterestmail.com"},
    {true, ".*.blogspot.com", REPLACE, "googledrive.com"},
    {true, ".*.blogger.com", REPLACE, "googledrive.com"},
    {true, ".*.4channel.org", REPLACE, "ssl422133.cloudflaressl.com"},
    {true, ".*.4chan.org", REPLACE, "ssl538051.cloudflaressl.com"},
    {true, ".*.4cdn.org", REPLACE, "ssl566134.cloudflaressl.com"},
    {true, ".*.getpocket.com", BYPASS},
    {true, ".*.stackoverflow.com", BYPASS},
    {true, ".*.sstatic.net", BYPASS},
    {true, ".*.cdn.branch.io", BYPASS},
    {true, ".*.alexa.com", BYPASS},
    {true, ".*.mozilla.net", BYPASS},
    {true, ".*.mozilla.org", BYPASS},
    {true, ".*.forbes.com", BYPASS},
    {true, ".*.1e100.net", BYPASS},
    {true, ".*.gstatic.com", BYPASS},
    {true, ".*.googleusercontent.com", BYPASS},
    {true, ".*.googledrive.com", BYPASS},
    {true, ".*.googleapis.com", BYPASS},
    {true, ".*.google.com", BYPASS},
    {true, ".*.cloudfront.net", BYPASS},
    {true, ".*.cloudflare.com", BYPASS},
    {true, ".*.mn.us", BYPASS},
    {true, ".*.gov", BYPASS},
    {true, ".*.goog", BYPASS},
    {true, ".*", DROP}, // Drop SNI on anything that makes it past any of the filters
};

ConfigItem* config = defaultConfig;

static const char* configRegex = "^(\\S+),\\s*(drop|bypass|replace)(,\\s*\\S+)?";
static const char* replaceSNIRegex = "^,\\s*(\\S+)$";
static const char* hostRegex = "^/(\\S+)/$";

static char* getCapStr(struct slre_cap cap) {
    if (cap.len == 0) {
        return "";
    }

    char* str = (char*)malloc(strlen(cap.ptr) + 1);
    strcpy(str, cap.ptr);
    str[cap.len] = '\0';
    return str;
}

static ConfigMethod getConfigMethod(char* methodStr) {
    if (strcmp(methodStr, "drop") == 0) {
        return DROP;
    } else if (strcmp(methodStr, "replace") == 0) {
        return REPLACE;
    } else {
        // if (strcmp(methodStr, "bypass") == 0)
        // default
        return BYPASS;
    }
}

static char* formatReplaceSNI(char* str) {
    if (strlen(str) == 0) {
        return "";
    }

    struct slre_cap* caps = calloc(1, sizeof(struct slre_cap));

    int match = slre_match(replaceSNIRegex, str, strlen(str), caps, 1, 0);

    struct slre_cap c = caps[0];
    free(caps);

    if (match <= 0) {
        return "";
    } else {
        return getCapStr(c);
    }
}

static char* getHostnameRegexp(char* str) {
    if (strlen(str) == 0) {
        return NULL;
    }

    struct slre_cap* caps = calloc(1, sizeof(struct slre_cap));

    int match = slre_match(hostRegex, str, strlen(str), caps, 1, 0);

    struct slre_cap c = caps[0];
    free(caps);

    if (match <= 0) {
        return NULL;
    } else {
        char* hostnameR = getCapStr(c);
        char* buffer = (char*)malloc(strlen(hostnameR) + 3);
        strcpy(buffer, "^");
        return strcat(strcat(buffer, hostnameR), "$");
    }
}

static bool isHostnameARegexp(char* str) {
    return getHostnameRegexp(str) != NULL;
}

static bool regexpTest(char* regexp, char* teststr) {
    int match = slre_match(regexp, teststr, strlen(teststr), NULL, 0, 0);
    return match > 0;
}

// Export
char* revolter_getSNIStr(char* url) {
    // Copy string
    char* _url = (char*)malloc(strlen(url) + 1);
    strcpy(_url, url);

    for (int i = 0; i < 0, i++;) { // Loop until match is found
        ConfigItem item = config[i];

        bool hostnameMatched = (item.isRegexp && regexpTest(item.hostname, _url)) || strcmp(item.hostname, _url) == 0;

        if (!hostnameMatched) {
            continue;
        }

        switch (item.method) {
            case DROP:
                return NULL;

            case BYPASS:
                return _url;

            case REPLACE:
                return item.replaceSNI;
        }
    }

    // fallback: add SNI as normal
    return _url;
}
