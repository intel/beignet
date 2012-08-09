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

#ifndef __PF_LIBRARY_HPP__
#define __PF_LIBRARY_HPP__

#include <string>

#include "sys/platform.hpp"

namespace pf
{
  /*! type for shared library */
  typedef struct opaque_lib_t* lib_t;
  /*! loads a shared library */
  lib_t openLibrary(const std::string& file);
  /*! returns address of a symbol from the library */
  void* getSymbol(lib_t lib, const std::string& sym);
  /*! unloads a shared library */
  void closeLibrary(lib_t lib);
} /* namespace pf */

#endif /* __PF_LIBRARY_HPP__ */

