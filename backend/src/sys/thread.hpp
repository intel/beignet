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

#ifndef __PF_THREAD_HPP__
#define __PF_THREAD_HPP__

#include "sys/platform.hpp"

namespace pf
{
  /*! Type for thread */
  typedef struct opaque_thread_t* thread_t;
  /*! Signature of thread start function */
  typedef void (*thread_func)(void*);
  /*! Creates a hardware thread running on specific logical thread */
  thread_t createThread(thread_func f, void* arg, size_t stack_size = 0, int affinity = -1);
  /*! Set affinity of the calling thread */
  void setAffinity(int affinity);
  /*! The thread calling this function gets yielded for a number of seconds */
  void yield(int time = 0);
  /*! Waits until the given thread has terminated */
  void join(thread_t tid);
  /*! Destroy handle of a thread */
  void destroyThread(thread_t tid);
  /*! Type for handle to thread local storage */
  typedef struct opaque_tls_t* tls_t;
  /*! Creates thread local storage */
  tls_t createTls();
  /*! Set the thread local storage pointer */
  void setTls(tls_t tls, void* const ptr);
  /*! Return the thread local storage pointer */
  void* getTls(tls_t tls);
  /*! Destroys thread local storage identifier */
  void destroyTls(tls_t tls);
}

#endif /* __PF_THREAD_HPP__ */

