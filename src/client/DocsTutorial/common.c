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

#include "./common.h"
#include "../common.h"

#include <stdio.h>
#include <string.h>

// see common.h for a description of this generating vector
cl_int gen_vec[DIMENSION] = {
  1, 201367, 117137, 36487, 165651, 490691, 77109, 210171, 410853, 356813, 371285, 54177, 312383, 487121, 29017, 392635, 45723, 454749, 64693, 130185, 288231, 141321, 197541, 499599, 131691, 385041, 42593, 238365, 279943, 134157
};


int task(cl_context context, cl_device_id device, cl_command_queue queue, void* data_);

int tut_main(int argc, char** argv, TutorialOptions opts)
{
  const char* prog = *argv++; argc--;
  cl_device_type device_type = CL_DEVICE_TYPE_CPU;

  while (argc && (*argv)[0] == '-') {
    if (strcmp(*argv, "--gpu") == 0) {
      device_type = CL_DEVICE_TYPE_GPU;
      argv++; argc--;
    }
  }

  int nargs = 2
    + (opts & TUT_REPLICATIONS        ? 1 : 0)
    + (opts & TUT_REPLICATIONS_PER_WI ? 1 : 0);

  if (argc != nargs) {
    fprintf(stderr, "usage: %s [--gpu] <log2-points> <log2-points-per-work-item>", prog);
    if (opts & TUT_REPLICATIONS)
        fprintf(stderr, " <replications>");
    if (opts & TUT_REPLICATIONS_PER_WI)
        fprintf(stderr, " <replications-per-work-item>");
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
  }

  TaskData data = { 0, 0, 0, 0 };
  int iarg = 0;
  data.points = 1 << atoi(argv[iarg++]);
  data.points_per_work_item = 1 << atoi(argv[iarg++]);
  if (opts & TUT_REPLICATIONS)
    data.replications = atoi(argv[iarg++]);
  if (opts & TUT_REPLICATIONS_PER_WI)
    data.replications_per_work_item = atoi(argv[iarg++]);

  return call_with_opencl(0, device_type, 0, &task, &data, CL_TRUE);
}

void computeStats(size_t n, clqmc_fptype* values, clqmc_fptype* avg, clqmc_fptype* var)
{
  double sum = 0.0;
  double sum_squares = 0.0;
  for (size_t i = 0; i < n; i++) {
    sum += values[i];
    sum_squares += values[i] * values[i];
  }
  *avg = sum / n;
  *var = (sum_squares - *avg * sum) / (n - 1);
}

void rqmcReduce(cl_uint replications, cl_uint blocks, clqmc_fptype* values, clqmc_fptype* estimates)
{
  for (cl_uint i = 0; i < replications; i++) {
    clqmc_fptype sum = 0.0;
    for (cl_uint j = 0; j < blocks; j++)
      sum += values[i * blocks + j];
    estimates[i] = sum / blocks;
  }
}

void rqmcReport(cl_uint replications, cl_uint points, cl_uint blocks, clqmc_fptype* values)
{
  clqmc_fptype* estimates = (clqmc_fptype*) malloc(replications * sizeof(clqmc_fptype));
  rqmcReduce(replications, blocks, values, estimates);
  clqmc_fptype avg, var;
  computeStats(replications, estimates, &avg, &var);
  free(estimates);
  printf("%16s%16s%16s%16s\n", "replications", "points", "mean", replications > 1 ? "variance" : "");
  printf("%16d%16d%16.6g", replications, points, avg);
  if (replications > 1)
    printf("%16.6g\n", var);
  else
    printf("\n");
}

