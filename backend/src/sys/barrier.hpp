/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

//////////////////////////////////////////////////////////////////////////////////////////
// Part of this file is taken from the Apache licensed Intel Embree project here:       //
// http://software.intel.com/en-us/articles/embree-photo-realistic-ray-tracing-kernels/ //
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef __GBE_BARRIER_HPP__
#define __GBE_BARRIER_HPP__

#include "sys/condition.hpp"

namespace gbe
{
  /*! system barrier using operating system */
  class BarrierSys
  {
  public:

    void init(size_t count) {
      this->count = 0;
      this->full_size = count;
    }

    int wait() {
      count_mutex.lock();
      count++;
      if (count == full_size) {
        count = 0;
        cond.broadcast();
        count_mutex.unlock();
        return 1;
      }
      cond.wait(count_mutex);
      count_mutex.unlock();
      return 0;
    }

  protected:
    size_t count, full_size;
    MutexSys count_mutex;
    ConditionSys cond;
    GBE_CLASS(BarrierSys);
  };

  /* default barrier type */
  class Barrier : public BarrierSys {};
}

#endif
