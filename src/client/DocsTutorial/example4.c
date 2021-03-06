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

#include <clQMC/latticerule.h>
#include <clRNG/mrg31k3p.h>

int main(int argc, char** argv)
{
  return tut_main(argc, argv, TUT_REPLICATIONS | TUT_REPLICATIONS_PER_WI);
}

int task(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
  const TaskData* data = (const TaskData*) data_;
  cl_int err;

  if (data->points % data->points_per_work_item)
    check_error(CLQMC_INVALID_VALUE, "points must be a multiple of points_per_work_item");

  if (data->replications % data->replications_per_work_item)
    check_error(CLQMC_INVALID_VALUE, "replications must be a multiple of replications_per_work_item");


  // Lattice buffer

  size_t pointset_size;
  // gen_vec is given in common.c
  clqmcLatticeRule* pointset = clqmcLatticeRuleCreate(data->points, DIMENSION, gen_vec, &pointset_size, &err);
  check_error(err, NULL);

  cl_mem pointset_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
      pointset_size, pointset, &err);
  check_error(err, "cannot create point set buffer");


  // Shifts buffer
  
  clqmc_fptype* shifts = (clqmc_fptype*) malloc(data->replications * DIMENSION * sizeof(clqmc_fptype));

  // populate random shifts using a random stream
  clrngMrg31k3pStream* stream = clrngMrg31k3pCreateStreams(NULL, 1, NULL, &err);
  check_error(err, NULL);
  for (cl_uint i = 0; i < data->replications; i++)
      for (cl_uint j = 0; j < DIMENSION; j++)
          shifts[i * DIMENSION + j] = clrngMrg31k3pRandomU01(stream);
  err = clrngMrg31k3pDestroyStreams(stream);
  check_error(err, NULL);

  cl_mem shifts_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
      data->replications * DIMENSION * sizeof(clqmc_fptype), shifts, &err);
  check_error(err, "cannot create shifts buffer");


  // Output buffer

  size_t points_block_count = data->points / data->points_per_work_item;
  cl_mem output_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, 
      data->replications * points_block_count * sizeof(clqmc_fptype), NULL, &err);
  check_error(err, "cannot create output buffer");


  // OpenCL kernel

  cl_program program = build_program_from_file(context, device,
      "client/DocsTutorial/example4_kernel.cl",
      NULL);
  check_error(err, NULL);
  cl_kernel kernel = clCreateKernel(program, "simulateWithRQMC", &err);
  check_error(err, "cannot create kernel");

  int iarg = 0;
  err  = clSetKernelArg(kernel, iarg++, sizeof(pointset_buf), &pointset_buf);
  err |= clSetKernelArg(kernel, iarg++, sizeof(shifts_buf), &shifts_buf);
  err |= clSetKernelArg(kernel, iarg++, sizeof(data->points_per_work_item), &data->points_per_work_item);
  err |= clSetKernelArg(kernel, iarg++, sizeof(data->replications), &data->replications);
  err |= clSetKernelArg(kernel, iarg++, sizeof(output_buf), &output_buf);
  check_error(err, "cannot set kernel arguments");


  // Execution

  cl_event ev;
  size_t global_size = (data->replications / data->replications_per_work_item) * points_block_count;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, &ev);
  check_error(err, "cannot enqueue kernel");

  err = clWaitForEvents(1, &ev);
  check_error(err, "error waiting for events");

  clqmc_fptype* output = (clqmc_fptype*) malloc(data->replications * points_block_count * sizeof(clqmc_fptype));
  err = clEnqueueReadBuffer(queue, output_buf, CL_TRUE, 0,
      data->replications * points_block_count * sizeof(clqmc_fptype), output, 0, NULL, NULL);
  check_error(err, "cannot read output buffer");

  printf("\nAdvanced randomized quasi-Monte Carlo integration:\n\n");

  err = clqmcLatticeRuleWriteInfo(pointset, stdout);
  check_error(err, NULL);
  printf("\n");

  rqmcReport(data->replications, data->points, points_block_count, output);


  // Clean up

  clReleaseEvent(ev);
  clReleaseMemObject(output_buf);
  clReleaseMemObject(pointset_buf);
  clReleaseKernel(kernel);
  clReleaseProgram(program);

  free(output);
  err = clqmcLatticeRuleDestroy(pointset);
  check_error(err, NULL);

  return EXIT_SUCCESS;
}
