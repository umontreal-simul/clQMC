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

// The actual distribution object is the following structure followed by an
// array of type clqmc_int[].
// The array is neither named nor pointed to by any member of the structure.
// Its location in memory is computed using a macro.

// macros for hidden member access
#define _CLQMC_LATTICE_GENVEC(lat,mem)                   ((mem clqmc_int*)(&(lat)[1]))
#define _CLQMC_LATTICE_GENVECNORMED(lat,mem,fptype)      ((mem fptype*)(&_CLQMC_LATTICE_GENVEC(lat,mem)[lat->dimension]))

#ifndef _CLQMC_LATTICE_MEM
#define _CLQMC_LATTICE_MEM
#endif

#ifndef _CLQMC_SHIFT_MEM
#define _CLQMC_SHIFT_MEM
#endif

/********************************************************************************
 * Functions and types declarations                                             *
 ********************************************************************************/

struct clqmcLatticeRule_ {
    clqmc_uint   numPoints;
    clqmc_uint   dimension;
    // TODO: in the docs, mention device & host must use same FPTYPE
    /* hidden members: */
    /* clqmc_int     genVec[dimension]; */
    /* _CLQMC_FPTYPE genVecNormed[dimension]; */
};

// IMPORTANT: cannot be transferred to device
struct clqmcLatticeRuleStream_ {
  _CLQMC_LATTICE_MEM const clqmcLatticeRule* lattice;
  clqmc_uint pointIndex;
  clqmc_uint coordinateIndex;
  _CLQMC_SHIFT_MEM const void* shift;
};

/********************************************************************************
 * Implementation                                                               *
 ********************************************************************************/

// We use an underscore on the r.h.s. to avoid potential recursion with certain
// preprocessors.
#define IMPLEMENT_STREAM_FOR_TYPE(fptype) \
  \
  clqmcStatus clqmcLatticeRuleCreateOverStream_##fptype(clqmcLatticeRuleStream* stream, _CLQMC_LATTICE_MEM const clqmcLatticeRule* lattice, clqmc_uint partCount, clqmc_uint partIndex, _CLQMC_SHIFT_MEM const fptype* shift) \
  { \
    if (!stream) \
      return clqmcSetErrorString(CLQMC_INVALID_VALUE, "%s(): stream cannot be NULL", __func__); \
    if (partIndex >= partCount) \
      return clqmcSetErrorString(CLQMC_INVALID_VALUE, "%s(): partIndex >= partCount", __func__); \
    if (lattice->numPoints % partCount != 0) \
      return clqmcSetErrorString(CLQMC_INVALID_VALUE, "%s(): number of points must be a multiple of partCount", __func__); \
    stream->lattice = lattice; \
    stream->pointIndex = (lattice->numPoints / partCount) * partIndex; \
    stream->coordinateIndex = 0; \
    stream->shift = shift; \
    return CLQMC_SUCCESS; \
  } \
  \
  fptype clqmcLatticeRuleNextCoordinate_##fptype(clqmcLatticeRuleStream* stream) { \
    if (stream->coordinateIndex >= stream->lattice->dimension) \
      return -1.0; \
    fptype ret = fmod( \
        _CLQMC_LATTICE_GENVECNORMED(stream->lattice,_CLQMC_LATTICE_MEM const,fptype)[stream->coordinateIndex] * stream->pointIndex \
        + (stream->shift ? ((_CLQMC_SHIFT_MEM const fptype*)stream->shift)[stream->coordinateIndex] : (fptype) 0.0), \
        (fptype) 1.0); \
    stream->coordinateIndex++; \
    return ret; \
  }

#ifdef __OPENCL_C_VERSION__
  // On the device, implement only what is required to avoid cluttering memory.
  #ifdef CLQMC_SINGLE_PRECISION
    IMPLEMENT_STREAM_FOR_TYPE(float)
  #else
    IMPLEMENT_STREAM_FOR_TYPE(double)
  #endif
#else
  // On the host, implement everything.
  IMPLEMENT_STREAM_FOR_TYPE(clqmc_float)
  IMPLEMENT_STREAM_FOR_TYPE(clqmc_double)
#endif

// Clean up macros, especially to avoid polluting device code.
#undef IMPLEMENT_STREAM_FOR_TYPE

clqmc_uint clqmcLatticeRuleForwardToNextPoint(clqmcLatticeRuleStream* stream) {
  stream->coordinateIndex = 0;
  stream->pointIndex++;
  return stream->pointIndex;
}

// FIXME: maybe as a macro?
clqmc_uint clqmcLatticeRuleNumPoints(_CLQMC_LATTICE_MEM const clqmcLatticeRule* lattice)
{
  return lattice->numPoints;
}

// FIXME: maybe as a macro?
clqmc_uint clqmcLatticeRuleDimension(_CLQMC_LATTICE_MEM const clqmcLatticeRule* lattice)
{
  return lattice->dimension;
}


// FIXME: maybe as a macro?
clqmc_uint clqmcLatticeRuleCurrentPointIndex(const clqmcLatticeRuleStream* stream)
{
  return stream->pointIndex;
}

// FIXME: maybe as a macro?
clqmc_uint clqmcLatticeRuleCurrentCoordIndex(const clqmcLatticeRuleStream* stream)
{
  return stream->coordinateIndex;
}


