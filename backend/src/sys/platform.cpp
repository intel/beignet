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
 */

#include "sys/platform.hpp"
#include "sys/intrinsics.hpp"
#include <string>

////////////////////////////////////////////////////////////////////////////////
/// Windows Platform
////////////////////////////////////////////////////////////////////////////////

#ifdef __WIN32__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace gbe
{
  double getSeconds() {
    LARGE_INTEGER freq, val;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&val);
    return (double)val.QuadPart / (double)freq.QuadPart;
  }

  void FATAL(const std::string &msg) {
    std::cerr << msg << std::endl;
    MessageBox(NULL, msg.c_str(), "Fatal Error", MB_OK | MB_ICONEXCLAMATION);
    GBE_ASSERT(0);
#ifdef __GNUC__
    exit(-1);
#else
    _exit(-1);
#endif /* __GNUC__ */
  }

} /* namespace gbe */
#endif /* __WIN32__ */

////////////////////////////////////////////////////////////////////////////////
/// Unix Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__UNIX__)

#include <sys/time.h>
#include <unistd.h>

namespace gbe
{
  double getSeconds() {
    struct timeval tp; gettimeofday(&tp,NULL);
    return double(tp.tv_sec) + double(tp.tv_usec)/1E6;
  }

  void FATAL(const std::string &msg) {
    std::cerr << msg << std::endl;
    GBE_ASSERT(0);
    _exit(-1);
  }
} /* namespace gbe */

#endif /* __UNIX__ */

