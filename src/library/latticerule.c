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

#include "clQMC/latticerule.h"
#include "private.h"

#include <math.h>

// code that is common to the host and to the device
#include "../include/clQMC/private/latticerule.c.h"

static clqmcLatticeRule* clqmcLatticeRuleCreate_(cl_uint numPoints, cl_uint dimension, const cl_int* genVec, size_t fpsize, size_t *objectSize, clqmcStatus* err)
{
  clqmcStatus err_ = CLQMC_SUCCESS;

  // basic size
  size_t size = sizeof(clqmcLatticeRule);

  // size of hidden data (generating vector + normed generating vector)
  size += dimension * (sizeof(cl_int) + fpsize);

  // allocation
  clqmcLatticeRule* lattice = (clqmcLatticeRule*) malloc(size);

  if (!lattice) {
    // allocation failed
    err_ = clqmcSetErrorString(CLQMC_OUT_OF_RESOURCES, "%s(): could not allocate memory for lattice", __func__);
  }
  else {
    // initialization
    lattice->numPoints = numPoints;
    lattice->dimension = dimension;
    for (cl_uint j = 0; j < dimension; j++)
      _CLQMC_LATTICE_GENVEC(lattice,)[j] = genVec[j];

    // set object size if needed
    if (objectSize)
      *objectSize = size;
  }

  // set error status if needed
  if (err)
    *err = err_;

  return (clqmcLatticeRule*) lattice;
}


#define IMPLEMENT_CREATE_FOR_TYPE(fptype) \
  clqmcLatticeRule* clqmcLatticeRuleCreate_##fptype(cl_uint numPoints, cl_uint dimension, const cl_int* genVec, size_t *objectSize, clqmcStatus* err) { \
    clqmcStatus err_; \
    clqmcLatticeRule* lattice = clqmcLatticeRuleCreate_(numPoints, dimension, genVec, sizeof(fptype), objectSize, &err_); \
    if (err_ == CLQMC_SUCCESS) { \
      fptype norm = ((fptype) 1.0) / numPoints; \
      for (cl_uint j = 0; j < dimension; j++) { \
        cl_uint gen = (_CLQMC_LATTICE_GENVEC(lattice,const)[j] % lattice->numPoints); \
        if (gen < 0) gen += lattice->numPoints; \
        _CLQMC_LATTICE_GENVECNORMED(lattice,,fptype)[j] = gen * norm; \
      } \
    } \
    if (err) \
      *err = err_; \
    return lattice; \
  }

IMPLEMENT_CREATE_FOR_TYPE(clqmc_float)
IMPLEMENT_CREATE_FOR_TYPE(clqmc_double)
#undef IMPLEMENT_CREATE_FOR_TYPE


#define IMPLEMENT_STREAM_FOR_TYPE(fptype) \
  \
  clqmcLatticeRuleStream* clqmcLatticeRuleCreateStream_##fptype(const clqmcLatticeRule* lattice, clqmc_uint partCount, clqmc_uint partIndex, const fptype* shift, clqmcStatus* err) \
  { \
    clqmcStatus err_ = CLQMC_SUCCESS; \
    clqmcLatticeRuleStream* stream = (clqmcLatticeRuleStream*) malloc(sizeof(clqmcLatticeRuleStream)); \
    if (stream == NULL) \
      err_ = clqmcSetErrorString(CLQMC_OUT_OF_RESOURCES, "%s(): could not allocate memory for stream", __func__); \
    else \
      err_ = clqmcLatticeRuleCreateOverStream_##fptype(stream, lattice, partCount, partIndex, shift); \
    if (err != NULL) \
      *err = err_; \
    return stream; \
  }

IMPLEMENT_STREAM_FOR_TYPE(clqmc_float)
IMPLEMENT_STREAM_FOR_TYPE(clqmc_double)

clqmcStatus clqmcLatticeRuleDestroyStream(clqmcLatticeRuleStream* stream)
{
  if (stream != NULL)
    free(stream);
  return CLQMC_SUCCESS;
}


clqmcStatus clqmcLatticeRuleDestroy(clqmcLatticeRule* lattice)
{
  if (!lattice)
    return clqmcSetErrorString(CLQMC_INVALID_VALUE, "%s(): lattice cannot be NULL", __func__);
  free(lattice);
  return CLQMC_SUCCESS;
}

clqmcStatus clqmcLatticeRuleWriteInfo(const clqmcLatticeRule* lattice, FILE* file)
{
  if (!lattice)
    return clqmcSetErrorString(CLQMC_INVALID_VALUE, "%s(): lattice cannot be NULL", __func__);
  fprintf(file, "%d-dimensional, %d-point lattice rule with generating vector [ ",
      lattice->dimension, lattice->numPoints);
  for (cl_uint j = 0; j < lattice->dimension; j++)
    fprintf(file, "%s%d", j > 0 ? ", " : "", _CLQMC_LATTICE_GENVEC(lattice,const)[j]);
    //fprintf(file, "%s%f", j > 0 ? ", " : "", _CLQMC_LATTICE_GENVECNORMED(lattice,const,cl_double)[j]);
  fprintf(file, " ]\n");
  return CLQMC_SUCCESS;
}
