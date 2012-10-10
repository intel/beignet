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

#include "sys/mutex.hpp"

#if defined(__WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace gbe
{
  /*! system mutex using windows API */
  MutexSys::MutexSys( void ) { mutex = new CRITICAL_SECTION; InitializeCriticalSection((CRITICAL_SECTION*)mutex); }
  MutexSys::~MutexSys( void ) { DeleteCriticalSection((CRITICAL_SECTION*)mutex); delete ((CRITICAL_SECTION*)mutex); }
  void MutexSys::lock( void ) { EnterCriticalSection((CRITICAL_SECTION*)mutex); }
  void MutexSys::unlock( void ) { LeaveCriticalSection((CRITICAL_SECTION*)mutex); }
}
#endif

#if defined(__UNIX__)
#include <pthread.h>

namespace gbe
{
  /*! system mutex using pthreads */
  MutexSys::MutexSys( void ) { mutex = new pthread_mutex_t; pthread_mutex_init((pthread_mutex_t*)mutex, NULL); }
  MutexSys::~MutexSys( void ) { pthread_mutex_destroy((pthread_mutex_t*)mutex); delete ((pthread_mutex_t*)mutex); }
  void MutexSys::lock( void ) { pthread_mutex_lock((pthread_mutex_t*)mutex); }
  void MutexSys::unlock( void ) { pthread_mutex_unlock((pthread_mutex_t*)mutex); }
}
#endif

