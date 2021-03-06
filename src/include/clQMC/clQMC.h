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

/*! @mainpage Introduction
 *
 *  We introduce clQMC, an OpenCL library for quasi-Monte Carlo methods.
 *  It provides facilities for using quasi-Monte Carlo methods to integrate
 *  functions over the unit hypercube in arbitrary dimension.
 *  Quasi-Monte Carlo methods can radically improve the accuracy of the
 *  estimator as compared to simple Monte Carlo.
 *  The main idea is to replace the independent uniform random numbers used in
 *  a simulation by quasi-Monte Carlo points.
 *
 *  With clQMC, quasi-Monte Carlo point sets are created on the host, and
 *  streams are attached to them to enumerate the points on the host or on the
 *  device.
 *  These streams act as sources of numbers in @f$(0,1)@f$ and can replace
 *  the streams from the
 *  [clRNG library](https://github.com/clMathLibraries/clRNG)
 *  in code that uses them (we show how to do this in @ref examples).
 *  The clProbDist library can also be used on top of clQMC for generating
 *  nonuniform variates.
 *  The design is closely inspired from that of the
 *  [SSJ library](http://simul.iro.umontreal.ca/ssj/) @cite iLEC08j .
 *
 *  @warning This is only an API proposal.  The actual library is not
 *  fully implemented.  Only rank-1 lattice rules are implemented for
 *  illustration of the ideas.
 *
 *
 *  ### What to read next?
 *
 *  - @ref qmc
 *  - @ref examples
 *  - @link clQMC_template.h
 *    API supported by each point set
 *    @endlink
 *    presented in a generic fashion
 *  - @link clQMC.h
 *    API not related to specific point sets
 *    @endlink
 *  - @ref configuration
 *  - @ref mem_types (advanced usage)
 *
 *
 *
 *  @section qmc About Quasi-Monte Carlo Methods
 *
 *
 *  @subsection qmc_mc Monte Carlo
 *
 *  **Monte Carlo** integration estimates the expectation @f$\mu =
 *  \mathbb E[X]@f$ of a random variate @f$X@f$ by the average
 *  @f[
 *	\hat\mu_{\mathrm{mc}} = \frac1n \sum_{i=0}^{n-1} X_i
 *  @f]
 *  of @f$n@f$ independent realizations @f$X_0,\dots,X_{n-1}@f$ of
 *  @f$X@f$.
 *  Each @f$X_i@f$ is usually generated by simulation,
 *  using say @f$s@f$ independent
 *  (pseudo)random numbers @f$U_{i,1},\dots,U_{i,s}@f$ uniformly distributed over
 *  @f$(0,1)@f$ for its source of randomness, so one can write
 *  @f$X_i = f(\boldsymbol U_i)@f$, where  @f$\boldsymbol U_i = (U_{i,1},\dots,U_{i,s})@f$ for
 *  @f$i=0,\dots,n-1@f$, for some function @f$f : [0,1)^s \to \mathbb R@f$.
 *  For standard Monte Carlo, the points
 *  @f$\boldsymbol U_i@f$ are independent and uniformly distributed in the @f$s@f$-dimensional
 *  hypercube @f$(0,1)^s@f$.
 *
 *  The variance of the Monte Carlo estimator @f$\hat\mu_{\mathrm{mc}}@f$ decreases
 *  as @f$n^{-1}@f$.
 *  This slow convergence rate is due to the uneven
 *  coverage of the integration domain @f$(0,1)^s@f$ by the independent random points
 *  @f$\boldsymbol U_0, \dots, \boldsymbol U_{n-1}@f$.
 *
 *
 *  @subsection qmc_qmc Quasi-Monte Carlo
 *
 *  **Quasi-Monte Carlo** methods
 *  (see @cite rNIE92a, @cite vSLO94a, @cite vLEC00b, @cite vLEC09f,
 *  @cite sLEM09a, @cite rDIC10a, @cite rNUY14a)
    can provide faster convergence.
 *  They replace the independent random points
 *  @f$\boldsymbol U_0, \dots, \boldsymbol U_{n-1}@f$ with deterministic,
 *  structured points
 *  @f$\boldsymbol u_0, \dots, \boldsymbol u_{n-1}@f$ that cover the
 *  integration domain more evenly.
 *  Popular quasi-Monte Carlo methods include lattice rules and digital nets
    (which include Sobol sequences and polynomial lattice rules, for example).
 *  The quasi-Monte Carlo estimator is deterministic:
 *  @f[
 *	\hat\mu_{\mathrm{qmc}} = \frac1n \sum_{i=0}^{n-1} f(\boldsymbol u_i).
 *  @f]
 *
 *
 *  @subsubsection qmc_qmc_latrules Rank-1 Lattice Rules
 *
 *  For **rank-1 lattice rules**, the points are defined as
 *  @f[
 *	\boldsymbol u_i = \frac{i \boldsymbol a \bmod n}{n}
 *  @f]
 *  for @f$i=0,\dots,n-1@f$, where the quality of the point set depends on the
 *  choice of the generating vector @f$\boldsymbol a \in \{1,\dots,n-1\}^s@f$.
 *  The modulo operation in the above formula is applied coordinate by
 *  coordinate.
 *  External software may be needed in order to find a proper generating
 *  vector @f$\boldsymbol a@f$ for any given integration problem (for a given
 *  dimension, number of points, etc.).
 *  We recommend [Lattice Builder](https://github.com/mungerd/latbuilder) @cite vLEC15a .
 *  An important advantage of lattice rules is that enumerating their points
 *  requires a trivial computational effort, which can be very helpful for use
 *  on a GPU.
 *
 *
 *  @subsection qmc_rqmc Randomized Quasi-Monte Carlo
 *
 *  **Randomized Quasi-Monte Carlo** produces an unbiased stochastic estimator
 *  @f[
 *	\hat\mu_{\mathrm{rqmc}} = \frac1n \sum_{i=0}^{n-1} f(\boldsymbol U_i)
 *  @f]
 *  by randomizing the quasi-Monte Carlo points @f$\boldsymbol u_i \mapsto
 *  \boldsymbol U_i@f$ in a way that preserves their mutual structure, and such
 *  that each randomized point @f$\boldsymbol U_i@f$ is uniformly distributed
 *  in @f$(0,1)^s\f$, when taken separately.
 *
 *  Usually, one generates several replications of
 *  @f$\hat\mu_{\mathrm{rqmc}}@f$ to estimate its mean and variance,
 *  and thus obtain both a better estimation of @f$\mu@f$ together with an
 *  indication of the accuracy of this estimation.
 *  It is generally more productive to increase the number of points than the
 *  number of randomizations to obtain a better estimation.
 *
 *
 *  @subsubsection qmc_rqmc_latrules Randomly-Shifted Rank-1 Lattice Rules
 *
 *  For rank-1 lattice rules, the randomization is generally a periodic random
 *  shift, i.e., a random vector @f$\boldsymbol U@f$ uniformly
 *  distributed in @f$(0,1)^s@f$ is added (modulo 1) to each point:
 *  @f[
 *	\boldsymbol U_i = (\boldsymbol u_i + \boldsymbol U) \bmod 1,
 *  @f]
 *  for @f$i=0,\dots,n-1@f$.
 *  The result is called a **randomly-shifted rank-1 lattice rule**.
 *  See @cite vLEC00b and @cite vLEC12a .
 *
 *
 *  @section examples Usage Examples: From Monte Carlo to Randomized Quasi-Monte Carlo
 *
 *  We show here how existing Monte Carlo code that uses
 *  [clRNG](https://github.com/clMathLibraries/clRNG) streams can be adapted to
 *  quasi-Monte Carlo methods by using clQMC streams instead.
 *  We further show how to combine clQMC and clRNG streams to apply randomized
 *  quasi-Monte Carlo.
 *
 *
 *  @subsection examples_model Example Model
 *
 *  Suppose we want to integrate
 *  @f[
 *	f(u_1,\dots,u_s) = \prod_{j=1}^s \frac{u_j^2}{3},
 *  @f]
 *  in dimension @f$s=30@f$.
 *
 *  We want our implementation of @f$f(u_1,\dots,u_s)@f$ to take as input a
 *  stream from either clRNG (for Monte Carlo) or clQMC (for quasi-Monte
 *  Carlo), and to obtain the successive coordinates \f$u_1,\dots,u_s\f$ as
 *  successive outputs from the stream.
 *  However, the type of the stream and the function that generates outputs
 *  from the stream have different names in clRNG and clQMC, so we refer to
 *  them through the preprocessor symbols `StreamType` and `nextCoordinate()`,
 *  respectively.
 *  This allows us to define them later, in accordance with the simulation
 *  method we want to use.
 *
 *  The following function evaluates @f$f(u_1,\dots,u_s)@f$ by generating the
 *  coordinates of the point @f$(u_1,\dots,u_s)@f$ through successive calls to
 *  `nextCoordinate()` on the input stream `stream`, as specified above:
 *  @code
 *  #define DIMENSION 30
 *
 *  double simulateOneRun(StreamType* stream)
 *  {
 *    double ret = 1.0;
 *    for (uint j = 0; j < DIMENSION; j++) {
 *      double uj = nextCoordinate(stream);
 *      ret *= 3 * uj * uj;
 *    }
 *    return ret;
 *  }
 *  @endcode
 *  The complete implementation is given in @ref DocsTutorial/common.clh.
 *  Henceforth, we refer to the dimension @f$s=30@f$ through the constant
 *  `DIMENSION`.
 *
 *
 *  @subsection examples_mc Example 1: Monte Carlo
 *
 *  To perform Monte Carlo integration, we use the MRG31k3p generator from
 *  [clRNG](https://github.com/clMathLibraries/clRNG).
 *  Every work item evaluates the integrand @f$f@f$ at `nw`
 *  distinct points, each provided by a distinct substream
 *  (see the documentation of
 *  [clRNG](https://github.com/clMathLibraries/clRNG)).
 *
 *  We define `StreamType` and `nextCoordinate`, from the definition of
 *  `simulateOneRun()` given in @ref examples_model, to use MRG31k3p streams:
 *  @code
 *  #define StreamType     clrngMrg31k3pStream
 *  #define nextCoordinate clrngMrg31k3pRandomU01
 *  @endcode
 *  The OpenCL kernel stores at a unique location in the output array `out` the
 *  average of `nw` values of @f$f@f$, obtained by invoking
 *  `simulateOneRun()` with `nw` different substreams:
 *  @code
 *  double sum = 0.0;
 *
 *  for (uint i = 0; i < nw; i++) {
 *    sum += simulateOneRun(&stream);
 *    clrngMrg31k3pForwardToNextSubstreams(1, &stream);
 *  }
 *
 *  out[get_global_id(0)] = sum / nw;
 *  @endcode
 *  The above code assumes that the variable `stream` already contains the stream
 *  object assigned to the current work item.
 *  The host is responsible for averaging the values stored in `out` by the
 *  work items and obtain @f$\hat\mu_{\mathrm{mc}}@f$.
 *
 *  Details about the clRNG device API (used in the above kernel) and about
 *  writing host code to send stream objects to the device are explained in
 *  the clRNG documentation.
 *
 *  The complete code for this example is given in @ref DocsTutorial/example1.c
 *  and @ref DocsTutorial/example1_kernel.cl.
 *
 *
 *  @subsection examples_qmc Example 2: Quasi-Monte Carlo
 *
 *  Here, we replace the Monte Carlo points from @ref examples_mc with
 *  quasi-Monte Carlo points by using rank-1 lattice rule streams from clQMC
 *  instead of MRG31k3p streams from clRNG.
 *  Every work item evaluates the integrand @f$f@f$ at only
 *  `nw` of the lattice points.
 *
 *  We redefine `StreamType` and `nextCoordinate` to use lattice rule streams:
 *  @code
 *  #define StreamType     clqmcLatticeRuleStream
 *  #define nextCoordinate clqmcLatticeRuleNextCoordinate
 *  @endcode
 *  In the device code, we include the clQMC header for lattice rules:
 *  @code
 *  #include <clQMC/latticerule.clh>
 *  @endcode
 *  The kernel receives the lattice point set object as one of its
 *  arguments, declared as:
 *  @code
 *  __global const clqmcLatticeRule* pointset
 *  @endcode
 *  This lattice point set object is stored in global memory and is shared
 *  across all work items.
 *  Each work item, however, attaches its own *stream object* to the point set
 *  by invoking clqmcLatticeRuleCreateOverStream():
 *  @code
 *  clqmcLatticeRuleStream stream; // in private memory
 *  clqmcLatticeRuleCreateOverStream(&stream, pointset,
 *    get_global_size(0), get_global_id(0),
 *    (void*)0);
 *  @endcode
 *  The third argument to clqmcLatticeRuleCreateOverStream() defines a
 *  partition of the lattice point set into `get_global_size(0)` even subsets,
 *  and the fourth argument selects the subset of index `get_global_id(0)` as
 *  the source of this work item's stream.
 *  The internal structure of the partitioning may depend on the type of point
 *  set used.
 *  We slightly modify the main loop to forward the stream to the next point
 *  instead of to the next substream:
 *  @code
 *  for (uint i = 0; i < nw; i++) {
 *    sum += simulateOneRun(&stream);
 *    clqmcLatticeRuleForwardToNextPoint(&stream);
 *  }
 *  @endcode
 *
 *  On the host, we include the clQMC header file for lattice:
 *  @code
 *  #include <clQMC/latticerule.h>
 *  @endcode
 *  A lattice point set that contains `n` points is created by
 *  invoking clqmcLatticeRuleCreate() with a proper generating vector `gen_vec`
 *  (selected beforehand):
 *  @code
 *  size_t pointset_size;
 *  cl_int gen_vec[] = { ... };
 *  clqmcLatticeRule* pointset = clqmcLatticeRuleCreate(n, DIMENSION, gen_vec, &pointset_size, &err);
 *  @endcode
 *  The resulting object `pointset`, of size `pointset_size` in bytes, can be
 *  copied to the device using standard OpenCL techniques (not shown here).
 *
 *  The complete code for this example is given in @ref DocsTutorial/example2.c
 *  and @ref DocsTutorial/example2_kernel.cl.
 *
 *
 *  @subsection examples_rqmc Example 3: Randomized Quasi-Monte Carlo
 *
 *  To use randomized quasi-Monte Carlo, we can add a periodic random shift
 *  @f$\boldsymbol U@f$, as explained in @ref qmc_rqmc, to a lattice rule.
 *  As in @ref examples_qmc, each work item is assigned a
 *  specific subset of the whole point set.
 *  But here, a work item is also responsible for generating all of the
 *  `replications` realizations of @f$f(\boldsymbol U_i)@f$ for each point
 *  @f$\boldsymbol U_i@f$ it was assigned.
 *  Each realization requires a distinct random @f$\boldsymbol U@f$, and it is
 *  the same for all points, so it is the same for all work items.
 *  We thus store the random shifts in advance on the host and copy them into
 *  the device's global memory in an array named `shifts`, composed of
 *  `replications` tuples of `DIMENSION` values (one for each point and each
 *  coordinate).
 *
 *  In the kernel code, for each replication of index `k`, we create a new
 *  lattice rule stream using the `k`-th random shift vector:
 *  @code
 *  clqmcLatticeRuleStream stream;
 *
 *  for (uint k = 0; k < replications; k++) {
 *
 *    clqmcLatticeRuleCreateOverStream(&stream, pointset,
 *      get_global_size(0), get_global_id(0),
 *      &shifts[k * DIMENSION]);
 *
 *    // ... (compute the value of `sum' here; this is unchanged)
 *
 *    out[k * get_global_size(0) + get_global_id(0)] = sum / nw;
 *  }
 *  @endcode
 *  The average associated with each random shift is stored at a distinct
 *  location in the output array `out`, divided into `replications` blocks of
 *  size `get_global_size(0)` (one for each replication).
 *
 *  In very high dimension, it might be preferable not to store all the random
 *  shifts in advance but to pass random streams to the kernel and let the work
 *  items generate the single shift they need at the moment they need it.
 *
 *  The complete code for this example is given in @ref DocsTutorial/example3.c
 *  and @ref DocsTutorial/example3_kernel.cl.
 *
 *
 *  @subsection examples_rqmc_adv Example 4: Advanced Randomized Quasi-Monte Carlo
 *
 *  We refine @ref examples_rqmc to allow for the work on different
 *  replications for the same points to be shared across multiple work items.
 *  We partition the replications into `r / rw` blocks of size `rw`, as the
 *  whole point set is partitioned into `N = n / nw` subsets of equal
 *  cardinality `nw`.
 *  Each work item processes a distinct combination of replication block and
 *  point subset.
 *  More precisely, we assign to the `gid`-th work item the `(gid / N)`-th
 *  replication block for the `(gid % N)`-th point subset.
 *
 *  The kernel code from @ref examples_rqmc is changed to compute the index
 *  (stored in the variable `k`) of each replication that a work item
 *  is responsible for, and to partition the point set into
 *  `N` (instead of `get_global_size(0)`) subsets by invoking
 *  clqmcLatticeRuleCreateOverStream():
 *  @code
 *  for (uint kw = 0; kw < rw; kw++) {
 *
 *    uint k = (gid / N) * rw + kw;
 *    uint j = gid % N;
 *
 *    clqmcLatticeRuleCreateOverStream(&stream, pointset,
 *      N, j,
 *      &shifts[k * DIMENSION]);
 *
 *    // ... (compute the value of `sum' here; this is unchanged)
 *
 *    out[k * N + j] = sum / nw;
 *  }
 *  @endcode
 *
 *  The complete code for this example is given in @ref DocsTutorial/example4.c
 *  and @ref DocsTutorial/example4_kernel.cl.
 *
 *  This example is very general because it allows the extreme cases where each
 *  work item to takes care of all randomizations at single point and where
 *  each work item takes care of a single randomization at all points, and of
 *  all intermediary cases.
 *  We recall that, usually, it is preferable to use as many points as possible
 *  to obtain an accurate estimator @f$\hat\mu_{\mathrm{rqmc}}@f$, with a small
 *  number of randomizations (maybe 5 to 30), just enough to be able to
 *  estimate the variance of @f$\hat\mu_{\mathrm{rqmc}}@f$.
 *
 *
 *  @section configuration Configuration
 *
 *  @subsection environment Environment variables
 *
 *  For all features of the library to work properly, the `CLQMC_ROOT`
 *  environment variable must be set to point to the installation path of the
 *  clQMC package, that is, the directory under which lies the
 *  `include/clQMC` subdirectory.
 *  Means of setting an environment variable depend on the operating system
 *  used.
 *
 *
 *  @section mem_types Device memory types
 *
 *  The API of each point set assumes that the point set objects are
 *  stored in a specific type of memory (the stream objects are always stored
 *  in private memory).
 *  It defaults to global memory, but can be customized by the user by changing
 *  the value of the preprocessor symbol `CLQMC_<POINTSET>_OBJ_MEM`, where
 *  `<POINTSET>` is the uppercase name of the point set, before including the
 *  device header file of the point set, to one of the following values:
 *
 *  - CLQMC_MEM_TYPE_LOCAL to use local memory;
 *  - CLQMC_MEM_TYPE_CONSTANT to use constant memory;
 *  - CLQMC_MEM_TYPE_GLOBAL to use global memory (the **default**).
 *
 *  For example, to store lattice rule objects in **constant memory**,
 *  the device code should simply begin with:
 *  @code
 *  #define CLQMC_LATTICERULE_OBJ_MEM CLQMC_MEM_TYPE_CONSTANT
 *  #include <clQMC/latticerule.clh>
 *  @endcode
 *
 *  @todo This is not implemented yet, but should be.
 */


/*! @example DocsTutorial/common.clh
 *  @brief Device code for @ref examples_model
 */

/*! @example DocsTutorial/example1.c
 *  @brief Host code for @ref examples_mc
 *  @see DocsTutorial/example1_kernel.cl
 */
/*! @example DocsTutorial/example1_kernel.cl
 *  @brief Device code for @ref examples_mc
 *  @see DocsTutorial/example1.c
 */

/*! @example DocsTutorial/example2.c
 *  @brief Host code for @ref examples_qmc
 *  @see DocsTutorial/example2_kernel.cl
 */
/*! @example DocsTutorial/example2_kernel.cl
 *  @brief Device code for @ref examples_qmc
 *  @see DocsTutorial/example2.c
 */

/*! @example DocsTutorial/example3.c
 *  @brief Host code for @ref examples_rqmc
 *  @see DocsTutorial/example3_kernel.cl
 */
/*! @example DocsTutorial/example3_kernel.cl
 *  @brief Device code for @ref examples_rqmc
 *  @see DocsTutorial/example3.c
 */

/*! @example DocsTutorial/example4.c
 *  @brief Host code for @ref examples_rqmc_adv
 *  @see DocsTutorial/example4_kernel.cl
 */
/*! @example DocsTutorial/example4_kernel.cl
 *  @brief Device code for @ref examples_rqmc_adv
 *  @see DocsTutorial/example4.c
 */


/*! @file clQMC.h
 *  @brief Library definitions.
 */

#pragma once
#ifndef CLQMC_H
#define CLQMC_H

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

typedef cl_double   clqmc_double;
typedef cl_float    clqmc_float;
typedef cl_int      clqmc_int;
typedef cl_uint     clqmc_uint;
typedef cl_long     clqmc_long;
typedef cl_ulong    clqmc_ulong;

#ifdef CLQMC_SINGLE_PRECISION
  typedef clqmc_float	clqmc_fptype;
  #define _CLQMC_FPTYPE clqmc_float
#else
  typedef clqmc_double	clqmc_fptype;
  #define _CLQMC_FPTYPE clqmc_double
#endif
#define _CLQMC_TAG_FPTYPE(name)           _CLQMC_TAG_FPTYPE_(name,_CLQMC_FPTYPE)
#define _CLQMC_TAG_FPTYPE_(name,fptype)   _CLQMC_TAG_FPTYPE__(name,fptype)
#define _CLQMC_TAG_FPTYPE__(name,fptype)  name##_##fptype


#if defined( _WIN32 )
	#define CLQMCAPI __declspec( dllexport )
#else
	#define CLQMCAPI
#endif


/*! @brief Error codes
 *
 *  Most library functions return an error status indicating the success or
 *  error state of the operation carried by the function.
 *  In case of success, the error status is set to `CLQMC_SUCCESS`.
 *  Otherwise, an error message can be retrieved by invoking
 *  clqmcGetErrorString().
 *
 *  @note In naming this type clqmcStatus, we follow the convention from clFFT
 *  and clBLAS, where the homologous types are name clfftStatus and
 *  clblasStatus, respectively.
 */
typedef enum clqmcStatus_ {
    CLQMC_SUCCESS              = CL_SUCCESS,
    CLQMC_OUT_OF_RESOURCES     = CL_OUT_OF_RESOURCES,
    CLQMC_INVALID_VALUE        = CL_INVALID_VALUE,
    /* ... */
    CLQMC_INVALID_ENVIRONMENT,
    CLQMC_NOT_IMPLEMENTED
} clqmcStatus;


#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Retrieve the last error message.
 *
 *  The buffer containing the error message is internally allocated and must
 *  not be freed by the client.
 *
 *  @return     Error message or `NULL`.
 */
const char* clqmcGetErrorString();

/*! @brief Generate an include option string for use with the OpenCL C compiler
 *
 *  Generate and return "-I${CLQMC_ROOT}/include", where \c ${CLQMC_ROOT} is
 *  the value of the \c CLQMC_ROOT environment variable.
 *  This string is meant to be passed as an option to the OpenCL C compiler for
 *  programs that make use of the clQMC device-side headers.
 *  If the \c CLQMC_ROOT environment variable is not defined, it defaults
 *  `/usr` if the file `/usr/include/clQMC/clQMC.h` exists, else to the current
 *  directory of execution of the program.
 *
 *  A static buffer is return and need not be released; it could change upon
 *  successive calls to the function.
 *
 *  An error is returned in \c err if the preallocated buffer is too small to
 *  contain the include string.
 *
 *  @param[out]     err         Error status variable, or `NULL`.
 *
 *  @return An OpenCL C compiler option to indicate where to find the
 *  device-side clQMC headers.
 */
const char* clqmcGetLibraryDeviceIncludes(cl_int* err);

/*! @brief Retrieve the library installation path
 *
 *  @return Value of the CLQMC_ROOT environment variable, if defined; else,
 *  `/usr` if the file `/usr/include/clQMC/clQMC.h` exists; or, the current
 *  directory (.) of execution of the program otherwise.
 */
const char* clqmcGetLibraryRoot();

#ifdef __cplusplus
}
#endif

#endif /* CLQMC_H *
 * vim: syntax=c.doxygen spell spelllang=en fdm=syntax fdls=0
 */
