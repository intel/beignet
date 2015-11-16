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
 * \file profiling.hpp
 *
 */
#ifndef __GBE_IR_PROFILING_HPP__
#define __GBE_IR_PROFILING_HPP__

#include <string.h>
#include "sys/map.hpp"
#include "sys/vector.hpp"
#include "unit.hpp"

namespace gbe
{
  namespace ir
  {
    class Context;
    class ProfilingInfo //: public Serializable
    {
    public:
      const static uint32_t MaxTimestampProfilingPoints = 20;
      enum {
        ProfilingSimdType1,
        ProfilingSimdType8,
        ProfilingSimdType16,
      };

      typedef struct {
        uint32_t fixedFunctionID:4;
        uint32_t simdType:4;
        uint32_t kernelID:24;
        union GenInfo {
          struct Gen7Info {
            uint16_t thread_id:3;
            uint16_t reserved1:5;
            uint16_t eu_id:4;
            uint16_t half_slice_id:1;
            uint16_t slice_id:2;
            uint16_t reserved0:1;
          } gen7;
          struct Gen8Info {
            uint16_t thread_id:3;
            uint16_t reserved1:5;
            uint16_t eu_id:4;
            uint16_t subslice_id:2;
            uint16_t slice_id:2;
          } gen8;
        } genInfo;
        uint16_t dispatchMask;
        uint32_t gidXStart;
        uint32_t gidXEnd;
        uint32_t gidYStart;
        uint32_t gidYEnd;
        uint32_t gidZStart;
        uint32_t gidZEnd;
        uint32_t userTimestamp[MaxTimestampProfilingPoints];
        uint32_t timestampPrologLo;
        uint32_t timestampPrologHi;
        uint32_t timestampEpilogLo;
        uint32_t timestampEpilogHi;
      } ProfilingReportItem;

      ProfilingInfo(const ProfilingInfo& other) {
        this->bti = other.bti;
        this->profilingType = other.profilingType;
        this->deviceID = other.deviceID;
      }

      ProfilingInfo(void) {
        this->bti = 0;
        this->profilingType = 0;
        this->deviceID = 0;
      }
      struct LockOutput {
        LockOutput(void) {
          pthread_mutex_lock(&lock);
        }

        ~LockOutput(void) {
          pthread_mutex_unlock(&lock);
        }
      };

      void setBTI(uint32_t b) {
        bti = b;
      }
      uint32_t getBTI() const {
        return bti;
      }
      void setProfilingType(uint32_t t) {
        profilingType = t;
      }
      uint32_t getProfilingType() const {
        return profilingType;
      }
      void setDeviceID(uint32_t id) {
        deviceID = id;
      }
      uint32_t getDeviceID() const {
        return deviceID;
      }
      void outputProfilingInfo(void* logBuf);

    private:
      uint32_t bti;
      uint32_t profilingType;
      uint32_t deviceID;
      friend struct LockOutput;
      static pthread_mutex_t lock;
      GBE_CLASS(ProfilingInfo);
    };
  } /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_PROFILING_HPP__ */
