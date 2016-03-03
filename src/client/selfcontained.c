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
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CLQMC_SINGLE_PRECISION
#include <clQMC/clQMC.h>
#include <clQMC/latticerule.h>


int main( void )
{
    cl_int err;
    cl_platform_id platform = 0;
    cl_device_id device = 0;
    cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
    cl_context ctx = 0;
    cl_command_queue queue = 0;
    cl_program program = 0;
    cl_kernel kernel = 0;
    cl_event event = 0;
    cl_mem bufIn, bufOut;
    float *out;
    const char *includes;
    char buildLog[4096];
    size_t numWorkItems = 64;
    size_t i, j;
    clqmcLatticeRule *lat = 0;
    size_t latBufferSize = 0;
    size_t kernelLines = 0;

    /* Sample kernel that calls clQMC device-side interfaces to enumerate
       the coordinates of a single point */
    const char *kernelSrc[] = {
        "#define CLQMC_SINGLE_PRECISION                                 \n",
        "#include <clQMC/latticerule.clh>                               \n",
        "                                                               \n",
        "__kernel void example(__global const clqmcLatticeRule *lat,    \n",
        "                      __global float *out)                     \n",
        "{                                                              \n",
        "    int gid = get_global_id(0);                                \n",
        "    int gsize = get_global_size(0);                            \n",
        "    int dim = clqmcLatticeRuleDimension(lat);                  \n",
        "                                                               \n",
        "    clqmcLatticeRuleStream stream;                             \n",
        "    clqmcLatticeRuleCreateOverStream(&stream, lat, gsize,      \n",
        "                                              gid, (void*)0);  \n",
        "                                                               \n",
        "    for (int j = 0; j < dim; j++) {                            \n",
        "        out[j * gsize + gid] =                                 \n",
        "                      clqmcLatticeRuleNextCoordinate(&stream); \n",
        "    }                                                          \n",
        "}                                                              \n",
    };

    /* Setup OpenCL environment. */
    err = clGetPlatformIDs( 1, &platform, NULL );
    err = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL );

    props[1] = (cl_context_properties)platform;
    ctx = clCreateContext( props, 1, &device, NULL, NULL, &err );
    #ifdef CL_VERSION_2_0
        queue = clCreateCommandQueueWithProperties( ctx, device, (cl_queue_properties[]){0}, &err );
    #else
        queue = clCreateCommandQueue( ctx, device, 0, &err );
    #endif


    /* Make sure CLQMC_ROOT is specified to get library path */
    includes = clqmcGetLibraryDeviceIncludes(&err);
    if(err != CL_SUCCESS) printf("\n%s\nSpecify environment variable CLQMC_ROOT as described\n", clqmcGetErrorString());

    /* Create sample kernel */
    kernelLines = sizeof(kernelSrc) / sizeof(kernelSrc[0]);
    program = clCreateProgramWithSource(ctx, kernelLines, kernelSrc, NULL, &err);
    err = clBuildProgram(program, 1, &device, includes, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("\nclBuildProgram has failed\n");
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 4096, buildLog, NULL);
        printf("%s", buildLog);
        exit(1);
    }
    kernel = clCreateKernel(program, "example", &err);

    /* Create a 3-dimensional lattice point set */
    lat = clqmcLatticeRuleCreate(numWorkItems, 3, (cl_int[]){1, 27, 15}, &latBufferSize, (clqmcStatus *)&err);

    /* Create buffers for the kernel */
    bufIn = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, latBufferSize, lat, &err);
    bufOut = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, numWorkItems * clqmcLatticeRuleDimension(lat) * sizeof(cl_float), NULL, &err);

    /* Setup the kernel */
    err = clSetKernelArg(kernel, 0, sizeof(bufIn),  &bufIn);
    err = clSetKernelArg(kernel, 1, sizeof(bufOut), &bufOut);

    /* Execute the kernel and read back results */
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &numWorkItems, NULL, 0, NULL, &event);
    err = clWaitForEvents(1, &event);
    out = (float *)malloc(numWorkItems * clqmcLatticeRuleDimension(lat) * sizeof(out[0]));
    err = clEnqueueReadBuffer(queue, bufOut, CL_TRUE, 0, numWorkItems * clqmcLatticeRuleDimension(lat) * sizeof(out[0]), out, 0, NULL, NULL);

    /* Display results */
    for (i = 0; i < clqmcLatticeRuleNumPoints(lat); i++)
    {
        for (j = 0; j < clqmcLatticeRuleDimension(lat); j++)
        {
            printf("%12.5f", out[j * clqmcLatticeRuleNumPoints(lat) + i]);
        }
        printf("\n");
    }

    /* Release allocated resources */
    clReleaseEvent(event);
    free(out);
    clReleaseMemObject(bufIn);
    clReleaseMemObject(bufOut);

    clReleaseKernel(kernel);
    clReleaseProgram(program);

    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);

    return 0;
}
