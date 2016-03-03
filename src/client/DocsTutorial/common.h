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

#pragma once
#ifndef CLQMC_DOCS_TUTORIAL_COMMON_H
#define CLQMC_DOCS_TUTORIAL_COMMON_H

#include <clQMC/clQMC.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
    #define SIZE_T_FORMAT   "%Iu"
#elif defined(__GNUC__)
    #define SIZE_T_FORMAT   "%zu"
#else
    #define SIZE_T_FORMAT   "%lu"
#endif

/*! @brief Dimension of the example model
 */
#define DIMENSION 30

/*! @brief Generating vector for embedded lattice rules.
 *
 *  This generating vector is good for numbers of points
 *  that are powers of 2, ranging from 2^5 to 2^20.
 *
 *  It was found with the
 *  [Lattice Builder software](https://github.com/mungerd/latbuilder).
 */
extern cl_int gen_vec[DIMENSION];

typedef struct TaskData_ {
  cl_uint points;
  cl_uint points_per_work_item;
  cl_uint replications;
  cl_uint replications_per_work_item;
} TaskData;

typedef enum TutorialOptions_ {
  TUT_DEFAULT                   = 0x00,
  TUT_REPLICATIONS              = 0x01,
  TUT_REPLICATIONS_PER_WI       = 0x02
} TutorialOptions;

int tut_main(int argc, char** argv, TutorialOptions opts);

/*! @brief Compute the average and variance of a sample.
 */
void computeStats(size_t n, clqmc_fptype* values, clqmc_fptype* avg, clqmc_fptype* var);

/*! @brief Compute multiple RQMC estimators at once.
 *
 *  Compute the `replications` realizations of the RQMC estimator contained in
 *  `values`, by block of `blocks` values.
 */
void rqmcReduce(cl_uint replications, cl_uint blocks, clqmc_fptype* values, clqmc_fptype* estimates);

/*! @brief Print a report on RQMC experiments.
 */
void rqmcReport(cl_uint replications, cl_uint points, cl_uint blocks, clqmc_fptype* values);


#endif // CLQMC_DOCS_TUTORIAL_COMMON_H
