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

#include "sys/library.hpp"
#include "sys/sysinfo.hpp"
#include "sys/filename.hpp"

////////////////////////////////////////////////////////////////////////////////
/// Windows Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace pf
{
  /* opens a shared library */
  lib_t openLibrary(const std::string& file)
  {
    std::string fullName = file+".dll";
    HMODULE handle = LoadLibraryA(fullName.c_str());
    if (handle) return lib_t(handle);
    handle = LoadLibrary((getExecutableFileName() + fullName).c_str());
    return lib_t(handle);
  }

  /* returns address of a symbol from the library */
  void* getSymbol(lib_t lib, const std::string& sym) {
    return (void*) GetProcAddress(HMODULE(lib),sym.c_str());
  }

  /* closes the shared library */
  void closeLibrary(lib_t lib) {
    FreeLibrary(HMODULE(lib));
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// Unix Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__UNIX__)

#include <dlfcn.h>

namespace pf
{
  /* opens a shared library */
  lib_t openLibrary(const std::string& file)
  {
#if defined(__MACOSX__)
    std::string fullName = "lib"+file+".dylib";
#else
    std::string fullName = "lib"+file+".so";
#endif
    void* lib = dlopen(fullName.c_str(),RTLD_NOW);
    if (lib) return lib_t(lib);
    lib = dlopen((getExecutableFileName() + fullName).c_str(),RTLD_NOW);
    return lib_t(lib);
  }

  /* returns address of a symbol from the library */
  void* getSymbol(lib_t lib, const std::string& sym) {
    return dlsym(lib,sym.c_str());
  }

  /* closes the shared library */
  void closeLibrary(lib_t lib) {
    dlclose(lib);
  }
}
#endif
