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

/**
 * \file cvar.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "sys/cvar.hpp"
#include <cstdio>

namespace gbe
{

  CVarInit::CVarInit(const char *name, int32_t *addr, int32_t imin, int32_t i, int32_t imax) :
    varType(CVarInit::INTEGER)
  {
    this->i.min = imin;
    this->i.max = imax;
    const char *env = getenv(name);
    if (env != NULL) {
      sscanf(env, "%i", &i);
      i = std::min(imax, std::max(imin, i));
    }
    *addr = i;
  }

  CVarInit::CVarInit(const char *name, float *addr, float fmin, float f, float fmax) :
    varType(CVarInit::FLOAT)
  {
    this->f.min = fmin;
    this->f.max = fmax;
    const char *env = getenv(name);
    if (env != NULL) {
      sscanf(env, "%f", &f);
      f = std::min(fmax, std::max(fmin, f));
    }
    *addr = f;
  }

  CVarInit::CVarInit(const char *name, std::string *str, const std::string &v) :
    varType(CVarInit::STRING)
  {
    const char *env = getenv(name);
    *str = env != NULL ? env : v;
  }

} /* namespace gbe */

