# clQMC Library

A library for quasi-Monte Carlo methods in OpenCL.


## Overview

Quasi-Monte Carlo methods replace the independent uniform random numbers used in
Monte Carlo simulation with carefully selected sampling points, to integrate
smooth functions over the unit cube (in arbitrary dimension) with better
accuracy.

Quasi-Monte Carlo point sets are created on the host, and streams are attached
to them to enumerate the points on the host or on the device. These streams act
as sources of numbers in (0,1) and can replace the streams from the
[clRNG](https://github.com/clMathLibraries/clRNG)
library in code that uses them.
The [clProbDist](https://github.com/umontreal-simul/clProbDist) library can also
be used on top of clQMC for generating nonuniform variates.
The design is closely inspired from that of the
[SSJ library](https://github.com/umontreal-simul/ssj).

**Only *rank-1 lattice rules* are currently implemented.**
This is basically an *API proposal*, and lattice rules are provided for
illustration of the ideas.
The [Lattice Builder software](https://github.com/mungerd/latbuilder) can be
used to find good parameters for rank-1 lattice rules.


## Documentation

The
[clQMC documentation](http://umontreal-simul.github.io/clQMC/htmldocs/index.html)
includes:

- [an introduction to quasi-Monte Carlo methods](http://umontreal-simul.github.io/clQMC/htmldocs/index.html#qmc);
- [usage examples](http://umontreal-simul.github.io/clQMC/htmldocs/index.html#examples) showing how to migrate from Monte Carlo with [clRNG](https://github.com/clMathLibraries/clRNG) to quasi-Monte Carlo using clQMC; and
- [the API reference](http://umontreal-simul.github.io/clQMC/htmldocs/files.html).


## Examples

Examples can be found in `src/client`.
The compiled client program examples can be found under the `bin` subdirectory
of the installation package (`$CLQMC_ROOT/bin` under Linux).


## Simple example

The simple example below shows how to use clQMC to enumerate the coordinates of
rank-1 lattice points by directly using device side headers (`.clh`) in your
OpenCL kernel.
Note that the example expects an OpenCL GPU device to be available.

```c
#include <stdlib.h>
#include <string.h>

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
```

## Acknowledgments

clQMC was developed by David Munger and Pierre L'Ecuyer at Université de Montréal,
in collaboration with Advanced Micro Devices, Inc.
