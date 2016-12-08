/*
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
/**
 * \file profiling.cpp
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "ir/profiling.hpp"
#include "src/cl_device_data.h"

namespace gbe
{
namespace ir
{
  pthread_mutex_t ProfilingInfo::lock = PTHREAD_MUTEX_INITIALIZER;

  void ProfilingInfo::outputProfilingInfo(void * logBuf)
  {
    LockOutput lock;
    uint32_t logNum = *reinterpret_cast<uint32_t*>(logBuf);
    printf("Total log number is %u\n", logNum);
    ProfilingReportItem* log = reinterpret_cast<ProfilingReportItem*>((char*)logBuf + 4);
    for (int i = 0; i < (int)logNum; i++) {
      GBE_ASSERT(log->simdType == ProfilingSimdType8 || log->simdType == ProfilingSimdType16);
      uint32_t simd = log->simdType == ProfilingSimdType16 ? 16 : 8;
      printf(" ------------------------ Log %-6d -----------------------\n", i);
      printf(" | fix functions id:%4d     simd: %4d   kernel id: %4d  |\n", log->fixedFunctionID,
          simd, log->kernelID);
      if (IS_IVYBRIDGE(deviceID)) {
        printf(" | thread id:       %4d     EU id:%4d   half slice id:%2d |\n", log->genInfo.gen7.thread_id,
            log->genInfo.gen7.eu_id, log->genInfo.gen7.half_slice_id);
      } else if (IS_HASWELL(deviceID)) {
        printf(" | thread id: %4d  EU id:%4d half slice id:%2d slice id%2d |\n", log->genInfo.gen7.thread_id,
            log->genInfo.gen7.eu_id, log->genInfo.gen7.half_slice_id, log->genInfo.gen7.slice_id);
      } else if (IS_BROADWELL(deviceID)) {
        printf(" | thread id: %4d  EU id:%4d  sub slice id:%2d slice id%2d |\n", log->genInfo.gen8.thread_id,
            log->genInfo.gen8.eu_id, log->genInfo.gen8.subslice_id, log->genInfo.gen8.slice_id);
      }

      uint64_t proLog = log->timestampPrologHi;
      proLog = ((proLog << 32) & 0xffffffff00000000) + log->timestampPrologLo;
      uint64_t epiLog = log->timestampEpilogHi;
      epiLog = ((epiLog << 32) & 0xffffffff00000000) + log->timestampEpilogLo;
      printf(" | dispatch Mask:%4x prolog:%10" PRIu64 "  epilog:%10" PRIu64 " |\n", log->dispatchMask, proLog, epiLog);

      printf(" | globalX:%4d~%4d  globalY:%4d~%4d  globalZ:%4d~%4d |\n", log->gidXStart, log->gidXEnd,
          log->gidYStart, log->gidYEnd, log->gidZStart, log->gidZEnd);
      for (uint32_t i = 0; i < MaxTimestampProfilingPoints - 2; i += 3) {
        printf(" |  ts%-2d:%10u  | ts%-2d:%10u  | ts%-2d:%10u  |\n", i, log->userTimestamp[i],
            i + 1, log->userTimestamp[i + 1], i + 2, log->userTimestamp[i + 2]);
      }
      printf(" |  ts18:%10u  | ts19:%10u  |                  |\n", log->userTimestamp[18], log->userTimestamp[19]);
      log++;
    }
  }
}
}
