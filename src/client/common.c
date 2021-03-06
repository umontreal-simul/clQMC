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

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "common.h"

#include <clQMC/clQMC.h>


#ifdef _MSC_VER
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

void check_error(cl_int errcode, const char* msg, ...) {
    if (errcode < 0) {
	char formatted[1024];
	if (msg == NULL) {
	    fprintf(stderr, "Error %d: %s\n", errcode, clqmcGetErrorString());
	}
	else {
	    va_list args;
	    va_start(args, msg);
	    vsprintf(formatted, msg, args);
	    va_end(args);
	    fprintf(stderr, "Error %d: %s\n", errcode, formatted);
	}
	exit(EXIT_FAILURE);
    }
}

int read_file(const char* filename, char** pbuf)
{
	size_t len;
	FILE* file;

#ifdef _MSC_VER
	int err = fopen_s(&file, filename, "rb");
	if (err != 0) {
		perror("Couldn't find the kernel file");
		exit(1);
	}
#else
	file = fopen(filename, "r");
	if (file == NULL) {
		perror("Couldn't find the kernel file");
		exit(1);
	}
#endif
	// get file size
	fseek(file, 0, SEEK_END);
	len = ftell(file);
	fseek(file, 0, SEEK_SET);
	// allocate memory
	*pbuf = (char*)malloc(len + 1);
	(*pbuf)[len] = '\0';
	// read whole file
	if (fread(*pbuf, 1, len, file) != len)
		return -102;
	fclose(file);
	return 0;
}

static char device_name[127];
static char device_version[127];
static char platform_name[127];
static char platform_version[127];

const char* get_device_name(cl_device_id device)
{
    cl_int err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    check_error(err, "cannot read device name");
    return device_name;
}

const char* get_device_version(cl_device_id device)
{
    cl_int err = clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(device_version), device_version, NULL);
    check_error(err, "cannot read device version");
    return device_version;
}

const char* get_platform_name(cl_platform_id platform)
{
    cl_int err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
    check_error(err, "cannot read platform name");
    return platform_name;
}

const char* get_platform_version(cl_platform_id platform)
{
    cl_int err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(platform_version), platform_version, NULL);
    check_error(err, "cannot read platform version");
    return platform_version;
}

size_t get_max_workgroup_size(cl_device_id device)
{
	size_t max_workgroup_size;
	cl_int err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_workgroup_size), &max_workgroup_size, NULL);
	check_error(err, "cannot read max work-group size");
	return max_workgroup_size;
}

void write_build_log(FILE* file, cl_program program, cl_device_id device)
{
    char* log;
    size_t len = 0;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
    log = (char *)malloc(len + 1);
    log[len] = '\0';
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, len + 1, log, NULL);
    fprintf(file, "build log:\n========================================\n%s\n========================================\n", log);
    free(log);
}

static int call_with_opencl_helper(
	cl_context context,
	cl_device_id device,
	int (*task)(cl_context,cl_device_id,cl_command_queue,void*),
	void* data,
	cl_bool echoVersion)
{
    cl_int err;
	if (echoVersion){
		printf("-- Using OpenCL device:   %s\n", get_device_name(device));
		printf("                          %s\n", get_device_version(device));
	}
#ifdef CL_VERSION_2_0
    cl_queue_properties queue_properties[] = {
	CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE,
	0};
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, queue_properties, &err);
#else
    cl_command_queue queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
#endif
    check_error(err, "cannot create command queue");

    int ret_val = task(context, device, queue, data);

    err = clReleaseCommandQueue(queue);
    check_error(err, "cannot release command queue");

    return ret_val;
}

int call_with_opencl(
	int platform_index,
	cl_device_type device_type,
	int device_index,
	int (*task)(cl_context,cl_device_id,cl_command_queue,void*),
	void* data,
	cl_bool echoVersion)
{
    cl_int err;

    cl_platform_id platforms[5];
    cl_uint num_platforms;

    err = clGetPlatformIDs(5, platforms, &num_platforms);
    check_error(err, "cannot find an OpenCL platform");

    if (platform_index >= num_platforms)
	check_error(CLQMC_INVALID_VALUE, "platform_index too large");

    cl_platform_id platform = platforms[platform_index];

    cl_uint num_devices;
    err = clGetDeviceIDs(platform, device_type, 0, NULL, &num_devices);
    check_error(err, "cannot read device count");

    cl_device_id* devices = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
    err = clGetDeviceIDs(platform, device_type, num_devices, devices, NULL);
    check_error(err, "cannot read device ID's");

    if (device_index >= num_devices)
	check_error(CLQMC_INVALID_VALUE, "device_index too large");

    cl_context context = clCreateContext(NULL, num_devices, devices, NULL, NULL, &err);
    check_error(err, "cannot create OpenCL context");

    int ret_val = 0;

	if (echoVersion){
		printf("-- Using OpenCL platform: %s\n", get_platform_name(platform));
		printf("                          %s\n", get_platform_version(platform));
	}
    if (device_index < 0) {
	for (size_t i = 0; i < num_devices; i++)
		ret_val = call_with_opencl_helper(context, devices[i], task, data, echoVersion);
    }
    else {
		ret_val = call_with_opencl_helper(context, devices[device_index], task, data, echoVersion);
    }

    for (size_t i = 0; i < num_devices; i++)
	clReleaseDevice(devices[i]);
    free(devices);
    clReleaseContext(context);

    return ret_val;
}

cl_program build_program_from_file(
	cl_context context,
	cl_device_id device,
	const char* source_file,
	const char* extra_options)
{
    cl_int err;

    char path[255];

    int nbytes;

#ifdef _MSC_VER
    nbytes = sprintf_s(
#else
    nbytes = snprintf(
#endif
	path,
	sizeof(path),
	"%s%s%s",
	clqmcGetLibraryRoot(),
	DIR_SEP,
	source_file);

#ifdef _MSC_VER
    if (nbytes < 0)
#else
    if (nbytes >= sizeof(path))
#endif
	check_error(CLQMC_OUT_OF_RESOURCES, "value of CLQMC_ROOT too long");

    source_file = path;

    char *sources[1];
    err = read_file(path, &sources[0]);
    check_error(err, "cannot read source file\ncheck that the environment variable CLQMC_ROOT set to the library root directory");

    cl_program program = clCreateProgramWithSource(context, 1, (const char**)sources, NULL, &err);
    check_error(err, "cannot create program");

    free(sources[0]);

    const char* includes = clqmcGetLibraryDeviceIncludes(NULL);
    const char* options = includes;
    char* buf = NULL;
    if (extra_options != NULL) {
	buf = (char*)malloc((strlen(includes) + strlen(extra_options)) + 2);
	strcpy(buf, includes);
	strcat(buf, " ");
	strcat(buf, extra_options);
	options = buf;
    }
    err = clBuildProgram(program, 0, NULL, options, NULL, NULL);
    if (extra_options != NULL)
	free(buf);
    if (err < 0)
	write_build_log(stderr, program, device);
    check_error(err, "cannot build program");

    return program;
}
