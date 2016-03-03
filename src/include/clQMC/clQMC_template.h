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
#ifndef CLQMC_TEMPLATE_H
#define CLQMC_TEMPLATE_H

#error This file is a template for point sets; it should not be included as is.  Use a distribution-specific header such as poisson.h instead.

#include <clQMC/clQMC.h>

/*! @file clQMC_template.h
 *  @brief Template of the API for specific quasi-Monte Carlo methods(not to be included as is!)
 *
 *  The function and type names in this API all start with @c clqmc.
 *  In each specific implementation, prefixes to function names are expanded to
 *  a specific prefix and the generic point set and types @c clqmcPointset and
 *  @c clqmcPointsetStream are replaced with specific point set and stream
 *  type, e.g., @ref clqmcLatticeRule and @ref clqmcLatticeRuleStream.
 *
 *
 *  ### Host and Device APIs
 *
 *  The functions described here are all available on the host, in all implementations,
 *  unless specified otherwise.  Only some of the functions and types are also
 *  available on the device in addition to the host;  they are tagged with
 *  [**device**].
 *  Other functions are only available on the device; they are tagged with
 *  [**device-only**].
 *  Some functions return an error code in @c err.
 *
 */

/*! @brief Point set object [**device**]
 *
 *  This structure contains data that describes a quasi-Monte Carlo point set.
 *  Its definition depends on the type of point set.
 */
typedef struct clqmcPointset_ clqmcPointset;

/*! @brief Point set stream object [**device**]
 *
 *  This structure contains data that describes a stream associated to a
 *  quasi-Monte Carlo point set.
 *  Its definition depends on the type of point set.
 */
typedef struct clqmcPointsetStream_ clqmcPointsetStream;


#ifdef __cplusplus
extern "C" {
#endif


/*! @brief Return the number of point in the point set [**device**]
 *
 *  @param[in] pointset	    A point set object.
 *
 *  @return Number of points in the point set.
 */
cl_uint clqmcNumPoints(const clqmcPointset* pointset);


/*! @brief Return the dimension of the point set [**device**]
 *
 *  @param[in] pointset	    A point set object.
 *
 *  @return Dimension of the point set.
 */
cl_uint clqmcDimension(const clqmcPointset* pointset);


/*! @brief Create a new point set object
 *
 *  Allocate resources for and create a new point set object.
 *
 *  The arguments of this function depend on the specific type of point set.
 *
 *  @param[out] objectSize  Size in bytes of the returned object.
 *  @param[out] err         Error status.
 *
 *  @return Newly created point set object.
 *
 *  @see clqmcLatticeRuleCreate(), clqmcLatticeRuleCreateKorobov()
 */
clqmcPointset* clqmcCreate(..., size_t* objectSize, clqmcStatus* err);


/*! @brief Destroy a point set object
 *
 *  Release the resources associated to a point set object.
 *
 *  @param[in]	pointset    Point set object to be destroyed.
 */
clqmcStatus clqmcDestroy(clqmcPointset* pointset);


/*! @brief Write information about a point set object
 *
 *  Write a description of a point set object and its properties to the
 *  console.
 */
clqmcStatus clqmcWriteInfo(const clqmcPointset* pointset, FILE* file);


/*! @brief Attach a new stream to a point set object
 *
 *  This function allocates resources for a new stream object and calls
 *  clqmcCreateOverStream() to actually create the stream object.
 *
 *  @see clqmcCreateOverStream()
 */
clqmcPointsetStream* clqmcCreateStream(const clqmcPointset* pointset, cl_uint partCount, cl_uint partIndex, const double* shift, clqmcStatus* err);


/*! @brief Attach a new stream to a point set object in already allocated memory [**device**]
 *
 *  This function does not allocate memory; it uses already allocated memory to
 *  create a stream object.
 *
 *  The point set `pointset` is divided into `partCount` indexed from `0` to `partCount - 1`.
 *  The internal structure of the partitioning generally depends on the type of point set.
 *
 *  A randomization is optionally applied.
 *  The type of the randomization (written here as `RandType` for convenience)
 *  depends on the type of point set.
 *  For example, for a rank-1 lattice rule, the randomization is a periodic random shift.
 *  The argument `shift` must be a vector of the same dimension as the lattice.
 *
 *  @param[out]	stream		Memory location that will hold the new stream
 *			    	object.
 *  @param[in]	pointset    	Point set object to attach the new stream to.
 *  @param[in]	partCount   	Number of subsets of equal cardinality into which
 *			    	pointset must be partitioned.
 *  @param[in]	partIndex   	Index of the subset represented by the new stream object.
 *  @param[in]  randomization   Randomization of an appropriate type, or `NULL` for no randomization.
 *
 *  @return Error status.
 */
clqmcStatus clqmcCreateOverStream(clqmcPointsetStream* stream, const clqmcPointset* pointset, cl_uint partCount, cl_uint partIndex, const RandType* randomization);


/*! @brief Destroy a stream object
 *
 *  Release the resources associated to a stream object.
 */
clqmcStatus clqmcDestroyStream(clqmcPointsetStream* stream);


/*! @brief Return the value of the next coordinate [**device**]
 *
 *  Advance the stream to the next coordinate of the current point and return
 *  its value.
 *  When all coordinates of a point are exhausted, clqmcForwardToNextPoint()
 *  should be call to advance the stream to the next point.
 *
 *  @param[in,out]  stream	Point set stream object.
 *
 *  @return Value of the next coordinate of the current point in the stream, or
 *  `-1.0` if no further coordinate is available.
 */
double clqmcNextCoordinate(clqmcPointsetStream* stream);


/*! @brief Retrieve all coordinates of the next point [**device**]
 *
 *  Advance the stream to the next point and return all its coordinates.
 *
 *  @param[in,out]  stream	Point set stream object.
 *  @param[out]	    coords	Coordinates of the point.
 *
 *  @return Index of the point returned in `coords`.
 *
 *  @todo Not implemented yet.
 */
cl_uint clqmcNextPoint(clqmcPointsetStream* stream, double* coords);


/*! @brief Advance a stream to the next point [**device**]
 *
 *  Advance the stream to the first coordinate of the next point.
 *
 *  @param[in,out]  stream	Point set stream object.
 *
 *  @return Index of the next point.
 */
cl_uint clqmcForwardToNextPoint(clqmcPointsetStream* stream);


/*! @brief Return the current point index of a stream [**device**]
 *
 *  @param[in]	stream	Point set stream object.
 *
 *  @return Index of the point currently pointed to by the stream.
 */
cl_uint clqmcCurrentPointIndex(const clqmcPointsetStream* stream);


/*! @brief Return the current coordinate index of a stream [**device**]
 *
 *  @param[in]	stream	Point set stream object.
 *
 *  @return Index of the coordinate of the point currently pointed to by the
 *  stream.
 */
cl_uint clqmcCurrentCoordIndex(const clqmcPointsetStream* stream);

#ifdef __cplusplus
}
#endif

#endif
/*
vim: ft=c.doxygen
*/
