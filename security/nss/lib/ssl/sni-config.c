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

#define PATH_SEP "/"

static char* getExecutableDir() {
    char buffer[PATH_MAX];
    char* path = buffer;
    realpath("/proc/self/exe", buffer);
    return dirname(path);
}

#elif defined(_WIN32)
#include <io.h>
#include <windows.h>

#define CP_GB18030 54936

#define PATH_SEP "\\"

static int getExecutablePath(char* out, int capacity, int* dirname_length) {
    wchar_t buffer1[MAX_PATH];
    wchar_t buffer2[MAX_PATH];
    wchar_t* path = NULL;
    int length = -1;

    for (;;) {
        DWORD size;
        int length_, length__;

        size = GetModuleFileNameW(NULL, buffer1, sizeof(buffer1) / sizeof(buffer1[0]));

        if (size == 0)
            break;
        else if (size == (DWORD)(sizeof(buffer1) / sizeof(buffer1[0]))) {
            DWORD size_ = size;
            do {
                wchar_t* path_;

                path_ = (wchar_t*)realloc(path, sizeof(wchar_t) * size_ * 2);
                if (!path_) break;
                size_ *= 2;
                path = path_;
                size = GetModuleFileNameW(NULL, path, size_);
            } while (size == size_);

            if (size == size_) break;
        } else
            path = buffer1;

        if (!_wfullpath(buffer2, path, MAX_PATH)) break;
        length_ = (int)wcslen(buffer2);
        length__ = WideCharToMultiByte(CP_GB18030, 0, buffer2, length_, out, capacity, NULL, NULL);

        if (length__ == 0)
            length__ = WideCharToMultiByte(CP_GB18030, 0, buffer2, length_, NULL, 0, NULL, NULL);
        if (length__ == 0) break;

        if (length__ <= capacity && dirname_length) {
            int i;

            for (i = length__ - 1; i >= 0; --i) {
                if (out[i] == '\\') {
                    *dirname_length = i;
                    break;
                }
            }
        }

        length = length__;

        break;
    }

    if (path != buffer1) free(path);

    return length;
}

static char* getExecutableDir() {
    int dirname_length;
    char* path = (char*)malloc(MAX_PATH + 1);

    getExecutablePath(path, MAX_PATH, &dirname_length);
    path[dirname_length] = '\0';

    return path;
}

#else

#define PATH_SEP "/"

static char* getExecutableDir() {
    printf("%s", "NotImplemented");
    abort();
}

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
    {true, ".*", DROP},  // drop any hostnames
};

ConfigItem* config = defaultConfig;
int configItemsCount = 1;
bool configRead = false;

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

    int match = slre_match(replaceSNIRegex, str, strlen(str) + 1, caps, 1, 0);

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

    int match = slre_match(hostRegex, str, strlen(str) + 1, caps, 1, 0);

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

static ConfigItem* readConfig() {
    if (config != defaultConfig || configRead) {
        return config;
    }

    char* execPath = getExecutableDir();
    char* configFilePath = strcat(strcat(execPath, PATH_SEP), "sni.config");
    // printf("%s\n", configFilePath);

    if (_access(configFilePath, 0) != -1) {  // file exists

        FILE* stream;
        char* line = NULL;
        size_t len = 0;

        stream = fopen(configFilePath, "r");

        ConfigItem* _config = (ConfigItem*)calloc(configItemsCount, sizeof(ConfigItem));
        memmove(_config, config, configItemsCount * sizeof(ConfigItem));  // copy default config items

        while (getline(&line, &len, stream) != -1) {
            struct slre_cap* caps = calloc(4, sizeof(struct slre_cap));

            int match = slre_match(configRegex, line, strlen(line) + 1, caps, 4, 0);

            if (match > 0) {
                _config = (ConfigItem*)realloc(_config, (configItemsCount + 1) * sizeof(ConfigItem));

                int _method = getConfigMethod(getCapStr(caps[1]));
                char* _hostname = getCapStr(caps[0]);
                char* _hostnameRegexp = getHostnameRegexp(_hostname);

                if (_hostnameRegexp == NULL) {
                    _config[configItemsCount].isRegexp = false;
                    _config[configItemsCount].hostname = _hostname;
                } else {
                    _config[configItemsCount].isRegexp = true;
                    _config[configItemsCount].hostname = _hostnameRegexp;
                }

                _config[configItemsCount].method = _method;
                _config[configItemsCount].replaceSNI = _method == REPLACE ? formatReplaceSNI(getCapStr(caps[2])) : NULL;

                free(caps);

                configItemsCount++;
            }
        }

        config = _config;

        free(line);
        fclose(stream);

    } else {
        // printf("%s\n", "file not found");
    }

    configRead = true;
}

static bool regexpTest(char* regexp, char* teststr) {
    int match = slre_match(regexp, teststr, strlen(teststr) + 1, NULL, 0, 0);
    return match > 0;
}

// export
char* revolter_getSNIStr(char* url) {
    readConfig();

    // copy string
    char* _url = (char*)malloc(strlen(url) + 1);
    strcpy(_url, url);

    for (int i = configItemsCount - 1; i >= 0; i--) {  // reverse, match the last item at first
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

static void _print_config() {
    readConfig();

    printf("length: %d\n", configItemsCount);

    for (int i = configItemsCount - 1; i >= 0; i--) {  // reverse, match the last item at first
        ConfigItem c = config[i];
        printf("isRegexp: %d hostname: '%s' method: %d replaceSNI: '%s' \n", c.isRegexp, c.hostname, c.method, c.replaceSNI);
    }
}
