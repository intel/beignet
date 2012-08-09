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

#ifndef __PF_CONDITION_HPP__
#define __PF_CONDITION_HPP__

#include "sys/mutex.hpp"

namespace pf
{
  class ConditionSys
  {
  public:
    ConditionSys(void);
    ~ConditionSys(void);
    void wait(class MutexSys& mutex);
    void broadcast(void);
  protected:
    void* cond;
  };
}

#endif

