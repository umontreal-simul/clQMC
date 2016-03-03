/* This file is part of clQMC.
 *
 * Copyright 2015-2016  Pierre L'Ecuyer, Universite de Montreal and Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authors:
 *
 *   David Munger <mungerd@iro.umontreal.ca>        (2015)
 *   Pierre L'Ecuyer <lecuyer@iro.umontreal.ca>     (2015)
 *
 */

/* @file private.c
 * @brief Implementation of functions defined in private.h
 */
#include "clQMC/clQMC.h"
#include "private.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define CASE_ERR_(code,msg) case code: base = msg; break
#define CASE_ERR(code)      CASE_ERR_(CLQMC_ ## code, MSG_ ## code)

char clqmcErrorString[1024]                = "";

static const char MSG_DEFAULT[]                 = "unknown status";
static const char MSG_SUCCESS[]                 = "success";
static const char MSG_OUT_OF_RESOURCES[]        = "out of resources";
static const char MSG_INVALID_VALUE[]           = "invalid value";
static const char MSG_INVALID_ENVIRONMENT[]     = "invalid environment";
static const char MSG_NOT_IMPLEMENTED[]         = "not implemented";


clqmcStatus clqmcSetErrorString(cl_int err, const char* msg, ...)
{
    char formatted[1024];
    const char* base;
    switch (err) {
        CASE_ERR(SUCCESS);
        CASE_ERR(OUT_OF_RESOURCES);
        CASE_ERR(INVALID_VALUE);
        CASE_ERR(INVALID_ENVIRONMENT);
        CASE_ERR(NOT_IMPLEMENTED);
        default: base = MSG_DEFAULT;
    }
    va_list args;
    va_start(args, msg);
    vsprintf(formatted, msg, args);
    sprintf(clqmcErrorString, "[%s] %s", base, formatted);
    va_end(args);
    return (clqmcStatus) err;
}
