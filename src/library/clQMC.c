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

/* @file clQMC.c
* @brief Implementation of functions defined in clQMC.h
*/

#include "clQMC/clQMC.h"
#include "private.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

extern char clqmcErrorString[1024];

const char* clqmcGetErrorString()
{
    return clqmcErrorString;
}


static char lib_path_default1[] = "/usr";
static char lib_path_default1_check[] = "/usr/include/clQMC/clQMC.h";
static char lib_path_default2[] = ".";
static char lib_path_default2_check[] = "./include/clQMC/clQMC.h";

const char* clqmcGetLibraryRoot()
{
    const char* lib_path = getenv("CLQMC_ROOT");

    if (lib_path != NULL && lib_path[0] != 0)
        return lib_path;

	// check if lib_path_default1_check exists
	if (
#ifdef _MSC_VER
	_access(lib_path_default1_check, 0) != -1
#else
	access(lib_path_default1_check, F_OK) != -1
#endif
	)
	   return lib_path_default1;

	// last resort
    if (
#ifdef _MSC_VER
	_access(lib_path_default2_check, 0) != -1
#else
	access(lib_path_default2_check, F_OK) != -1
#endif
	)
	   return lib_path_default2;

	return NULL;
}


static char lib_includes[1024];

const char* clqmcGetLibraryDeviceIncludes(cl_int* err)
{
    int nbytes;
    const char* root = clqmcGetLibraryRoot();

    if (err)
        *err = CLQMC_SUCCESS;

	if (root == NULL) {
		if (err)
			*err = clqmcSetErrorString(CLQMC_INVALID_ENVIRONMENT, "environment variable CLQMC_ROOT not set");
		return NULL;
	}
#ifdef _MSC_VER
    nbytes = sprintf_s(
#else
    nbytes = snprintf(
#endif
	lib_includes,
	sizeof(lib_includes),
	"-I\"%s/include\"",
	root);

#ifdef _MSC_VER
    if (nbytes < 0) {
#else
    if (nbytes >= sizeof(lib_includes)) {
#endif
	if (err)
	    *err = clqmcSetErrorString(CLQMC_OUT_OF_RESOURCES, "value of CLQMC_ROOT too long (max = %u)", sizeof(lib_includes) - 16);
	return NULL;
    }
    return lib_includes;
}
