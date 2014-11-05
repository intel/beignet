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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/**
 * \file assert.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "utest_assert.hpp"
#include "utest_exception.hpp"
#include <cassert>
#include <cstdlib>

void onFailedAssertion(const char *msg, const char *file, const char *fn, int line)
{
  char lineString[256];
  sprintf(lineString, "%i", line);
  assert(msg != NULL && file != NULL && fn != NULL);
  const std::string str = "Error: "
                        + std::string(msg) + "\n  at file "
                        + std::string(file)
                        + ", function " + std::string(fn)
                        + ", line " + std::string(lineString);
  throw Exception(str);
}

