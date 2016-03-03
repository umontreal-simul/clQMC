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

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "../common.h"
#include "./common.h"

#include <clRNG/mrg31k3p.h>

int main(int argc, char** argv)
{
  return tut_main(argc, argv, TUT_DEFAULT);
}

int task(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
  const TaskData* data = (const TaskData*) data_;
  cl_int err;

  if (data->points % data->points_per_work_item)
    check_error(CLQMC_INVALID_VALUE, "points must be a multiple of points_per_work_item");


  // Streams buffer

  size_t streams_size;
  clrngMrg31k3pStream* streams = clrngMrg31k3pCreateStreams(NULL, data->points / data->points_per_work_item, &streams_size, &err);
  check_error(err, NULL);

  cl_mem streams_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
      streams_size, streams, &err);
  check_error(err, "cannot create streams buffer");


  // Output buffer

  size_t points_block_count = data->points / data->points_per_work_item;
  cl_mem output_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, 
      points_block_count * sizeof(clqmc_fptype), NULL, &err);
  check_error(err, "cannot create output buffer");


  // OpenCL kernel

  cl_program program = build_program_from_file(context, device,
      "client/DocsTutorial/example1_kernel.cl",
      clrngGetLibraryDeviceIncludes(&err));
  check_error(err, NULL);
  cl_kernel kernel = clCreateKernel(program, "simulateWithMC", &err);
  check_error(err, "cannot create kernel");

  int iarg = 0;
  err  = clSetKernelArg(kernel, iarg++, sizeof(streams_buf), &streams_buf);
  err |= clSetKernelArg(kernel, iarg++, sizeof(data->points_per_work_item), &data->points_per_work_item);
  err |= clSetKernelArg(kernel, iarg++, sizeof(output_buf),  &output_buf);
  check_error(err, "cannot set kernel arguments");


  // Execution

  cl_event ev;
  size_t global_size = points_block_count;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, &ev);
  check_error(err, "cannot enqueue kernel");

  err = clWaitForEvents(1, &ev);
  check_error(err, "error waiting for events");

  clqmc_fptype* output = (clqmc_fptype*) malloc(points_block_count * sizeof(clqmc_fptype));
  err = clEnqueueReadBuffer(queue, output_buf, CL_TRUE, 0,
      points_block_count * sizeof(clqmc_fptype), output, 0, NULL, NULL);
  check_error(err, "cannot read output buffer");

  printf("\nMonte Carlo integration:\n\n");
  rqmcReport(1, data->points, points_block_count, output);


  // Clean up

  clReleaseEvent(ev);
  clReleaseMemObject(output_buf);
  clReleaseMemObject(streams_buf);
  clReleaseKernel(kernel);
  clReleaseProgram(program);

  free(output);
  err = clrngMrg31k3pDestroyStreams(streams);
  check_error(err, NULL);

  return EXIT_SUCCESS;
}
