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

/*! @file latticerule.h
 *  @brief Host interface for lattice rules
 */

#pragma once
#ifndef CLQMC_LATTICERULE_H
#define CLQMC_LATTICERULE_H

#include <clQMC/clQMC.h>
#include <stdio.h>

struct clqmcLatticeRule_;

/*! @brief Lattice rule object
 *  
 *  This object stores the properties of a lattice rule (number of points,
 *  dimension and generating vector).
 */
typedef struct clqmcLatticeRule_ clqmcLatticeRule;


struct clqmcLatticeRuleStream_;

/*! @brief Lattice rule stream object
 *  
 *  This stream object can enumerate all or a portion of the points of a
 *  lattice rule.
 *  When a random shift is applied to a lattice rule, it is attached to the
 *  stream object.
 */
typedef struct clqmcLatticeRuleStream_ clqmcLatticeRuleStream;


#ifdef __cplusplus
extern "C" {
#endif

/*! @copybrief clqmcNumPoints()
*  @see clqmcNumPoints()
*/
cl_uint clqmcLatticeRuleNumPoints(const clqmcLatticeRule* lattice);

/*! @copybrief clqmcDimension()
*  @see clqmcDimension()
*/
cl_uint clqmcLatticeRuleDimension(const clqmcLatticeRule* lattice);

#define clqmcLatticeRuleCreate _CLQMC_TAG_FPTYPE(clqmcLatticeRuleCreate)

/*! @brief Create a new rank-1 lattice rule.
 *
 *  @see qmc_qmc
 *
 *  @param[in]  numPoints   Number of points.
 *  @param[in]  dimension   Dimension.
 *  @param[in]  genVec      Generating vector.
 *  @param[out] objectSize  Size in bytes of the returned object.
 *  @param[out] err         Error status.
 *
 *  @return New rank-1 lattice rule object.
 */
clqmcLatticeRule* clqmcLatticeRuleCreate             (cl_uint numPoints, cl_uint dimension, const cl_int* genVec, size_t* objectSize, clqmcStatus* err);
clqmcLatticeRule* clqmcLatticeRuleCreate_clqmc_float (cl_uint numPoints, cl_uint dimension, const cl_int* genVec, size_t* objectSize, clqmcStatus* err);
clqmcLatticeRule* clqmcLatticeRuleCreate_clqmc_double(cl_uint numPoints, cl_uint dimension, const cl_int* genVec, size_t* objectSize, clqmcStatus* err);

/*! @brief Create a new Korobov lattice rule.
 *
 *  For a Korobov lattice rule, the generating vector @f$\boldsymbol a = (1, a,
 *  a^2 \bmod n, \dots, a^s \bmod n)@f$ is parameterized by a single number
 *  @f$a@f$.
 *
 *  @param[in]  numPoints   Number of points.
 *  @param[in]  dimension   Dimension.
 *  @param[in]  gen         Parameter @f$a@f$ for the generating vector.
 *  @param[out] objectSize  Size in bytes of the returned object.
 *  @param[out] err         Error status.
 *
 *  @return New Korobov lattice rule object.
 *
 *  @see clqmcLatticeRuleCreate()
 *
 */
clqmcLatticeRule* clqmcLatticeRuleCreateKorobov(cl_uint numPoints, cl_uint dimension, cl_int gen, size_t* objectSize, clqmcStatus* err);

/*! @copybrief clqmcDestroy()
*  @see clqmcDestroy()
*/
clqmcStatus       clqmcLatticeRuleDestroy(clqmcLatticeRule* lattice);

/*! @copybrief clqmcWriteInfo()
*  @see clqmcWriteInfo()
*/
clqmcStatus       clqmcLatticeRuleWriteInfo(const clqmcLatticeRule* lattice, FILE* file);

#define clqmcLatticeRuleCreateStream       _CLQMC_TAG_FPTYPE(clqmcLatticeRuleCreateStream)
#define clqmcLatticeRuleCreateOverStream   _CLQMC_TAG_FPTYPE(clqmcLatticeRuleCreateOverStream)
#define clqmcLatticeRuleNextCoordinate     _CLQMC_TAG_FPTYPE(clqmcLatticeRuleNextCoordinate)
#define clqmcLatticeRuleNextPoint          _CLQMC_TAG_FPTYPE(clqmcLatticeRuleNextPoint)

/*! @copybrief clqmcCreateStream()
*  @see clqmcCreateStream()
*/
CLQMCAPI clqmcLatticeRuleStream* clqmcLatticeRuleCreateStream             (const clqmcLatticeRule* lattice, cl_uint partCount, cl_uint partIndex, const _CLQMC_FPTYPE* shift, clqmcStatus* err);
CLQMCAPI clqmcLatticeRuleStream* clqmcLatticeRuleCreateStream_clqmc_float (const clqmcLatticeRule* lattice, cl_uint partCount, cl_uint partIndex, const cl_float*      shift, clqmcStatus* err);
CLQMCAPI clqmcLatticeRuleStream* clqmcLatticeRuleCreateStream_clqmc_double(const clqmcLatticeRule* lattice, cl_uint partCount, cl_uint partIndex, const cl_double*     shift, clqmcStatus* err);

/*! @copybrief clqmcCreateOverStream()
 *  @see clqmcCreateOverStream()
 *
 *  For a rank-1 lattice rule, the randomization is a periodic random shift.
 *  The argument `shift` must be a vector of the same dimension as the lattice,
 *  or `NULL`.
 */
CLQMCAPI clqmcStatus clqmcLatticeRuleCreateOverStream             (clqmcLatticeRuleStream* stream, const clqmcLatticeRule* lattice, cl_uint partCount, cl_uint partIndex, const _CLQMC_FPTYPE* shift);
CLQMCAPI clqmcStatus clqmcLatticeRuleCreateOverStream_clqmc_float (clqmcLatticeRuleStream* stream, const clqmcLatticeRule* lattice, cl_uint partCount, cl_uint partIndex, const cl_float*      shift);
CLQMCAPI clqmcStatus clqmcLatticeRuleCreateOverStream_clqmc_double(clqmcLatticeRuleStream* stream, const clqmcLatticeRule* lattice, cl_uint partCount, cl_uint partIndex, const cl_double*     shift);

/*! @copybrief clqmcDestroyStream()
*  @see clqmcDestroyStream()
*/
CLQMCAPI clqmcStatus clqmcLatticeRuleDestroyStream(clqmcLatticeRuleStream* stream);

/*! @copybrief clqmcNextCoordinate()
*  @see clqmcNextCoordinate()
*/
CLQMCAPI _CLQMC_FPTYPE clqmcLatticeRuleNextCoordinate         (clqmcLatticeRuleStream* stream);
CLQMCAPI cl_float  clqmcLatticeRuleNextCoordinate_clqmc_float (clqmcLatticeRuleStream* stream);
CLQMCAPI cl_double clqmcLatticeRuleNextCoordinate_clqmc_double(clqmcLatticeRuleStream* stream);

/*! @copybrief clqmcNextPoint()
*  @see clqmcNextPoint()
*/
CLQMCAPI cl_uint clqmcLatticeRuleNextPoint             (clqmcLatticeRuleStream* stream, clqmc_fptype* coords);
CLQMCAPI cl_uint clqmcLatticeRuleNextPoint_clqmc_float (clqmcLatticeRuleStream* stream, clqmc_fptype* coords);
CLQMCAPI cl_uint clqmcLatticeRuleNextPoint_clqmc_double(clqmcLatticeRuleStream* stream, clqmc_fptype* coords);

/*! @copybrief clqmcForwardToNextPoint()
*  @see clqmcForwardToNextPoint()
*/
CLQMCAPI cl_uint clqmcLatticeRuleForwardToNextPoint(clqmcLatticeRuleStream* stream);

/*! @copybrief clqmcCurrentPointIndex()
*  @see clqmcCurrentPointIndex()
*/
CLQMCAPI cl_uint clqmcLatticeRuleCurrentPointIndex(const clqmcLatticeRuleStream* stream);

/*! @copybrief clqmcCurrentCoordIndex()
*  @see clqmcCurrentCoordIndex()
*/
CLQMCAPI cl_uint clqmcLatticeRuleCurrentCoordIndex(const clqmcLatticeRuleStream* stream);

#ifdef __cplusplus
}
#endif

#endif
