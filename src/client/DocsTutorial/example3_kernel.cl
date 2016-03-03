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

#include <clQMC/latticerule.clh>

#define StreamType     clqmcLatticeRuleStream
#define nextCoordinate clqmcLatticeRuleNextCoordinate
#include <clQMC/DocsTutorial/common.clh>

__kernel void simulateWithRQMC(
        __global const clqmcLatticeRule* pointset,
        __global const clqmc_fptype* shifts,
        uint points_per_work_item,
        uint replications,
        __global clqmc_fptype* out)
{
  uint gsize  = get_global_size(0);
  uint gid    = get_global_id(0);
  uint points = clqmcLatticeRuleNumPoints(pointset);

  clqmcLatticeRuleStream stream;

  for (uint k = 0; k < replications; k++) {

    clqmcLatticeRuleCreateOverStream(&stream, pointset, gsize, gid, &shifts[k * DIMENSION]);

    clqmc_fptype sum = 0.0;

    for (uint i = 0; i < points_per_work_item; i++) {
      sum += simulateOneRun(&stream);
      clqmcLatticeRuleForwardToNextPoint(&stream);
    }

    out[k * gsize + gid] = sum / points_per_work_item;
  }
}

/*
vim: ft=c sw=2 expandtab
*/
