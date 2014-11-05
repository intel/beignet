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
 */

#ifndef __GBE_MUTEX_HPP__
#define __GBE_MUTEX_HPP__

#include "platform.hpp"
#include "atomic.hpp"
#include <xmmintrin.h>

namespace gbe
{
  class MutexSys {
    friend class ConditionSys;
  public:
    MutexSys(void);
    ~MutexSys(void);
    void lock(void);
    void unlock(void);
  protected:
    void* mutex;
    MutexSys(const MutexSys&); // don't implement
    MutexSys& operator= (const MutexSys&); // don't implement
    GBE_CLASS(MutexSys);
  };

  /*! active mutex */
  class MutexActive {
  public:
    INLINE MutexActive(void) : _lock(LOCK_IS_FREE) {}
    INLINE void lock(void) {
      GBE_COMPILER_READ_BARRIER;
      while (cmpxchg(_lock, LOCK_IS_TAKEN, LOCK_IS_FREE) != LOCK_IS_FREE)
        _mm_pause();
      GBE_COMPILER_READ_BARRIER;
    }
    INLINE void unlock(void) { _lock.storeRelease(LOCK_IS_FREE); }
  protected:
    enum { LOCK_IS_FREE = 0, LOCK_IS_TAKEN = 1 };
    Atomic _lock;
    MutexActive(const MutexActive&); // don't implement
    MutexActive& operator=(const MutexActive&); // don't implement
    GBE_CLASS(MutexActive);
  };

  /*! safe mutex lock and unlock helper */
  template<typename Mutex> class Lock {
  public:
    Lock (Mutex& mutex) : mutex(mutex) { mutex.lock(); }
    ~Lock() { mutex.unlock(); }
  protected:
    Mutex& mutex;
    Lock(const Lock&); // don't implement
    Lock& operator= (const Lock&); // don't implement
    GBE_CLASS(Lock);
  };
}

#endif /* __GBE_MUTEX_HPP__ */
