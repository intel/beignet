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

#include "sys/condition.hpp"

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(__GNUC__)

namespace gbe
{
  // This is an implementation of POSIX "compatible" condition variables for
  // Win32, as described by Douglas C. Schmidt and Irfan Pyarali:
  // http://www.cs.wustl.edu/~schmidt/win32-cv-1.html. The code is directly
  // readapted from the glfw source code that may be found here:
  // http://www.glfw.org
  enum {
    MINGW32_COND_SIGNAL     = 0,
    MINGW32_COND_BROADCAST  = 1
  };

  /*! Implement the internal condition variable implementation Mingw lacks */
  struct Mingw32Cond
  {
    HANDLE events[2];                   //<! Signal and broadcast event HANDLEs
    unsigned int waiters_count;         //<! Count of the number of waiters
    CRITICAL_SECTION waiters_count_lock;//!< Serialize access to <waiters_count>
  };

  ConditionSys::ConditionSys ()
  {
    cond = (Mingw32Cond *) GBE_NEW(Mingw32Cond);
    ((Mingw32Cond *)cond)->waiters_count = 0;
    ((Mingw32Cond *)cond)->events[MINGW32_COND_SIGNAL]    = CreateEvent(NULL, FALSE, FALSE, NULL);
    ((Mingw32Cond *)cond)->events[MINGW32_COND_BROADCAST] = CreateEvent(NULL, TRUE, FALSE, NULL);
    InitializeCriticalSection(&((Mingw32Cond *)cond)->waiters_count_lock);
  }

  ConditionSys::~ConditionSys ()
  {
    CloseHandle(((Mingw32Cond *)cond)->events[MINGW32_COND_SIGNAL]);
    CloseHandle(((Mingw32Cond *)cond)->events[MINGW32_COND_BROADCAST]);
    DeleteCriticalSection(&((Mingw32Cond *)cond)->waiters_count_lock);
    GBE_DELETE((Mingw32Cond *)cond);
  }

  void ConditionSys::wait(MutexSys& mutex)
  {
    Mingw32Cond *cv = (Mingw32Cond *) cond;
    int result, last_waiter;
    DWORD timeout_ms;

    // Avoid race conditions
    EnterCriticalSection(&cv->waiters_count_lock);
    cv->waiters_count ++;
    LeaveCriticalSection(&cv->waiters_count_lock);

    // It's ok to release the mutex here since Win32 manual-reset events
    // maintain state when used with SetEvent()
    LeaveCriticalSection((CRITICAL_SECTION *) mutex.mutex);
    timeout_ms = INFINITE;

    // Wait for either event to become signaled
    result = WaitForMultipleObjects(2, cv->events, FALSE, timeout_ms);

    // Check if we are the last waiter
    EnterCriticalSection(&cv->waiters_count_lock);
    cv->waiters_count --;
    last_waiter = (result == WAIT_OBJECT_0 + MINGW32_COND_BROADCAST) &&
                  (cv->waiters_count == 0);
    LeaveCriticalSection(&cv->waiters_count_lock);

    // Some thread called broadcast
    if (last_waiter) {
      // We're the last waiter to be notified or to stop waiting, so
      // reset the manual event
      ResetEvent(cv->events[MINGW32_COND_BROADCAST]);
    }

    // Reacquire the mutex
    EnterCriticalSection((CRITICAL_SECTION *) mutex.mutex);
  }

  void ConditionSys::broadcast()
  {
    Mingw32Cond *cv = (Mingw32Cond *) cond;
    int have_waiters;

    // Avoid race conditions
    EnterCriticalSection(&cv->waiters_count_lock);
    have_waiters = cv->waiters_count > 0;
    LeaveCriticalSection(&cv->waiters_count_lock);

    if (have_waiters)
      SetEvent(cv->events[MINGW32_COND_BROADCAST]);
  }
} /* namespace gbe */
#else

namespace gbe
{
  /*! system condition using windows API */
  ConditionSys::ConditionSys () { cond = GBE_NEW(CONDITION_VARIABLE); InitializeConditionVariable((CONDITION_VARIABLE*)cond); }
  ConditionSys::~ConditionSys() { GBE_DELETE((CONDITION_VARIABLE*)cond); }
  void ConditionSys::wait(MutexSys& mutex) { SleepConditionVariableCS((CONDITION_VARIABLE*)cond, (CRITICAL_SECTION*)mutex.mutex, INFINITE); }
  void ConditionSys::broadcast() { WakeAllConditionVariable((CONDITION_VARIABLE*)cond); }
} /* namespace gbe */
#endif /* __GNUC__ */
#endif /* __WIN32__ */

#if defined(__UNIX__)
#include <pthread.h>
namespace gbe
{
  ConditionSys::ConditionSys () { cond = GBE_NEW(pthread_cond_t); pthread_cond_init((pthread_cond_t*)cond,NULL); }
  ConditionSys::~ConditionSys() { GBE_DELETE((pthread_cond_t*)cond); }
  void ConditionSys::wait(MutexSys& mutex) { pthread_cond_wait((pthread_cond_t*)cond, (pthread_mutex_t*)mutex.mutex); }
  void ConditionSys::broadcast() { pthread_cond_broadcast((pthread_cond_t*)cond); }
} /* namespace gbe */
#endif /* __UNIX__ */



