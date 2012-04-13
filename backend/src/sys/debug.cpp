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
 * \file debug.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "debug.hpp"
#include <cstdio>
#include <sstream>
#include <string>

namespace gbe
{
#define DECL_DEBUG_VAR(TYPE, NAME) TYPE NAME;
  #include "debug.hxx"
#undef DECL_DEBUG_VAR
} /* namespace gbe */

namespace
{
  template <typename VarType>
  static VarType getValue(const char *str) {
    VarType value;
    std::stringstream ss;
    ss << std::string(str);
    ss >> value;
    return value;
  }

  struct DebugVarInitializer
  {
    DebugVarInitializer(void) {
#define DECL_DEBUG_VAR(TYPE, NAME) gbe::NAME = TYPE(0);
#include "debug.hxx"
#undef DECL_DEBUG_VAR

#define DECL_DEBUG_VAR(TYPE, NAME) do { \
  const char *str = getenv(#NAME); \
  if (str != NULL) gbe::NAME = getValue<TYPE>(str); \
} while (0);
#include "debug.hxx"
#undef DECL_DEBUG_VAR
    }
  };

  static DebugVarInitializer debugVarInitializer;
} /* namespace */

