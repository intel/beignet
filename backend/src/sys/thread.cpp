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

#include <iostream>

#include "sys/thread.hpp"
#include "sys/sysinfo.hpp"

////////////////////////////////////////////////////////////////////////////////
/// Windows Platform
////////////////////////////////////////////////////////////////////////////////

#ifdef __WIN32__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace pf
{
  /*! creates a hardware thread running on specific core */
  thread_t createThread(thread_func f, void* arg, size_t stack_size, int affinity)
  {
    HANDLE handle = CreateThread(NULL,stack_size,(LPTHREAD_START_ROUTINE)f,arg,0,NULL);
    if (handle == NULL)
      FATAL("createThread error");
    if (affinity < 0) return thread_t(handle);

    // set thread affinity
    int thread = affinity % 64;
    SetThreadAffinityMask(handle, DWORD_PTR(1L << thread));

    return thread_t(handle);
  }

  void setAffinity(int affinity) {
    if (affinity >= 0) SetThreadAffinityMask(GetCurrentThread(), DWORD_PTR(1L << affinity));
  }

  void yield(int time) { Sleep(time); }

  void join(thread_t tid) {
    WaitForSingleObject(HANDLE(tid), INFINITE);
    CloseHandle(HANDLE(tid));
  }

  void destroyThread(thread_t tid) {
    TerminateThread(HANDLE(tid),0);
    CloseHandle(HANDLE(tid));
  }

  tls_t createTls() { return tls_t(TlsAlloc()); }

  void setTls(tls_t tls, void* const ptr) {
    TlsSetValue(DWORD(size_t(tls)), ptr);
  }

  void* getTls(tls_t tls) {
    return TlsGetValue(DWORD(size_t(tls)));
  }

  void destroyTls(tls_t tls) {
    TlsFree(DWORD(size_t(tls)));
  }
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// Linux Platform
////////////////////////////////////////////////////////////////////////////////

#ifdef __LINUX__
namespace pf
{
  /*! set affinity of the calling thread */
  void setAffinity(int affinity)
  {
    int wrap = getNumberOfLogicalThreads()/2;
    affinity = (affinity/2) + wrap*(affinity%2);
    if (affinity >= 0 && affinity < 64*64) {
      union { uint64 u; cpu_set_t set; } mask[64];
      for (size_t i=0; i<64; i++) mask[i].u = 0;
      mask[affinity/64].u= uint64(1) << (affinity % 64);
      if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask[0].set) < 0)
        std::cerr << "Thread: cannot set affinity" << std::endl;
    }
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// MacOSX Platform
////////////////////////////////////////////////////////////////////////////////

#ifdef __MACOSX__

#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <mach/mach_init.h>

namespace pf
{
  /*! set affinity of the calling thread */
  void setAffinity(int affinity)
  {
    if (affinity >= 0) {
      thread_affinity_policy ap;
      ap.affinity_tag = affinity;
      if (thread_policy_set(mach_thread_self(),THREAD_AFFINITY_POLICY,(integer_t*)&ap,THREAD_AFFINITY_POLICY_COUNT) != KERN_SUCCESS)
        std::cerr << "Thread: cannot set affinity" << std::endl;
    }
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// Unix Platform
////////////////////////////////////////////////////////////////////////////////

#ifdef __UNIX__

#include <pthread.h>
#include <sched.h>

namespace pf
{
  struct ThreadStartupData {
    int affinity;
    thread_func f;
    void* arg;
  };

  static void* threadStartup(ThreadStartupData* parg)
  {
    ThreadStartupData arg = *parg; PF_DELETE(parg); parg = NULL;
    setAffinity(arg.affinity);
    arg.f(arg.arg);
    return NULL;
  }

  thread_t createThread(thread_func f, void* arg, size_t stack_size, int affinity)
  {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stack_size > 0) pthread_attr_setstacksize (&attr, stack_size);

    pthread_t* tid = PF_NEW(pthread_t);
    ThreadStartupData* startup = PF_NEW(ThreadStartupData);
    startup->f = f;
    startup->arg = arg;
    startup->affinity = affinity;

    if (pthread_create(tid,&attr,(void*(*)(void*))threadStartup,startup) != 0)
      FATAL("Thread creation error");

    return thread_t(tid);
  }

  void yield(int time) {
    if (time == 0) sched_yield();
    else usleep(time * 1000);
  }

  void join(thread_t tid) {
    if (pthread_join(*(pthread_t*)tid, NULL) != 0)
      FATAL("pthread_join error");
    PF_DELETE((pthread_t*)tid);
  }

  void destroyThread(thread_t tid) {
    pthread_cancel(*(pthread_t*)tid);
    PF_DELETE((pthread_t*)tid);
  }

  tls_t createTls() {
    pthread_key_t* key = PF_NEW(pthread_key_t);
    if (pthread_key_create(key,NULL) != 0)
      FATAL("pthread_key_create error");
    return tls_t(key);
  }

  void* getTls(tls_t tls) {
    return pthread_getspecific(*(pthread_key_t*)tls);
  }

  void setTls(tls_t tls, void* const ptr) {
    if (pthread_setspecific(*(pthread_key_t*)tls, ptr) != 0)
      FATAL("pthread_setspecific error");
  }

  void destroyTls(tls_t tls) {
    if (pthread_key_delete(*(pthread_key_t*)tls) != 0)
      FATAL("pthread_key_delete error");
    PF_DELETE((pthread_key_t*)tls);
  }
}
#endif
