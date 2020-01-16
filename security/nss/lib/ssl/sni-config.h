/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is PRIVATE to revolter-firefox.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdbool.h>

#ifndef __revolter_firefox_sni_config_h_
#define __revolter_firefox_sni_config_h_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DROP,
    BYPASS,
    REPLACE,
} ConfigMethod;

typedef struct ConfigItem {
    bool isRegexp;         // whether hostname is a regexp
    char* hostname;        // the hostname to match or a /regexp/ (only implements a subset of regexp, prefix ^ and suffix & will be added when parsing)
    unsigned char method;  // enum: [0] drop [1] bypass [2] replace
    char* replaceSNI;      // required when method == REPLACE
} ConfigItem;

char* revolter_getSNIStr(char* url);

#ifdef __cplusplus
}
#endif

#endif
