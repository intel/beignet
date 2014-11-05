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

#ifndef __GBE_INTRINSICS_HPP__
#define __GBE_INTRINSICS_HPP__

#include "sys/platform.hpp"
#include <xmmintrin.h>
#include <emmintrin.h>

#if defined(__MSVC__)

#include <intrin.h>

#define GBE_COMPILER_WRITE_BARRIER       _WriteBarrier()
#define GBE_COMPILER_READ_WRITE_BARRIER  _ReadWriteBarrier()

#if _MSC_VER >= 1400
#pragma intrinsic(_ReadBarrier)
#define GBE_COMPILER_READ_BARRIER        _ReadBarrier()
#else
#define GBE_COMPILER_READ_BARRIER        _ReadWriteBarrier()
#endif /* _MSC_VER >= 1400 */

INLINE int __bsf(int v) {
  unsigned long r = 0; _BitScanForward(&r,v); return r;
}

INLINE int __bsr(int v) {
  unsigned long r = 0; _BitScanReverse(&r,v); return r;
}

INLINE int __btc(int v, int i) {
  long r = v; _bittestandcomplement(&r,i); return r;
}

INLINE int __bts(int v, int i) {
  long r = v; _bittestandset(&r,i); return r;
}

INLINE int __btr(int v, int i) {
  long r = v; _bittestandreset(&r,i); return r;
}

INLINE void memoryFence(void) { _mm_mfence(); }

#if defined(__X86_64__) && !defined(__INTEL_COMPILER)

INLINE size_t __bsf(size_t v) {
  unsigned long r = 0; _BitScanForward64(&r,v); return r;
}

INLINE size_t __bsr(size_t v) {
  unsigned long r = 0; _BitScanReverse64(&r,v); return r;
}

INLINE size_t __btc(size_t v, size_t i) {
  __int64_t r = v; _bittestandcomplement64(&r,i); return r;
}

INLINE size_t __bts(size_t v, size_t i) {
  __int64_t r = v; _bittestandset64(&r,i); return r;
}

INLINE size_t __btr(size_t v, size_t i) {
  __int64_t r = v; _bittestandreset64(&r,i); return r;
}

#endif /* defined(__X86_64__) && !defined(__INTEL_COMPILER) */

typedef int32_t atomic32_t;

INLINE int32_t atomic_add(volatile int32_t* m, const int32_t v) {
  return _InterlockedExchangeAdd((volatile long*)m,v);
}

INLINE int32_t atomic_cmpxchg(volatile int32_t* m, const int32_t v, const int32_t c) {
  return _InterlockedCompareExchange((volatile long*)m,v,c);
}

#if defined(__X86_64__)

typedef int64_t atomic_t;

INLINE int64_t atomic_add(volatile int64_t* m, const int64_t v) {
  return _InterlockedExchangeAdd64(m,v);
}

INLINE int64_t atomic_cmpxchg(volatile int64_t* m, const int64_t v, const int64_t c) {
  return _InterlockedCompareExchange64(m,v,c);
}

#else

typedef int32_t atomic_t;

#endif /* defined(__X86_64__) */

#else

INLINE unsigned int __popcnt(unsigned int in) {
  int r = 0; asm ("popcnt %1,%0" : "=r"(r) : "r"(in)); return r;
}

INLINE int __bsf(int v) {
  int r = 0; asm ("bsf %1,%0" : "=r"(r) : "r"(v)); return r;
}

INLINE int __bsr(int v) {
  int r = 0; asm ("bsr %1,%0" : "=r"(r) : "r"(v)); return r;
}

INLINE int __btc(int v, int i) {
  int r = 0; asm ("btc %1,%0" : "=r"(r) : "r"(i), "0"(v) : "flags"); return r;
}

INLINE int __bts(int v, int i) {
  int r = 0; asm ("bts %1,%0" : "=r"(r) : "r"(i), "0"(v) : "flags"); return r;
}

INLINE int __btr(int v, int i) {
  int r = 0; asm ("btr %1,%0" : "=r"(r) : "r"(i), "0"(v) : "flags"); return r;
}

INLINE size_t __bsf(size_t v) {
  size_t r = 0; asm ("bsf %1,%0" : "=r"(r) : "r"(v)); return r;
}

INLINE size_t __bsr(size_t v) {
  size_t r = 0; asm ("bsr %1,%0" : "=r"(r) : "r"(v)); return r;
}

INLINE size_t __btc(size_t v, size_t i) {
  size_t r = 0; asm ("btc %1,%0" : "=r"(r) : "r"(i), "0"(v) : "flags"); return r;
}

INLINE size_t __bts(size_t v, size_t i) {
  size_t r = 0; asm ("bts %1,%0" : "=r"(r) : "r"(i), "0"(v) : "flags"); return r;
}

INLINE size_t __btr(size_t v, size_t i) {
  size_t r = 0; asm ("btr %1,%0" : "=r"(r) : "r"(i), "0"(v) : "flags"); return r;
}

INLINE void memoryFence(void) { _mm_mfence(); }

typedef int32_t atomic32_t;

INLINE int32_t atomic_add(int32_t volatile* value, int32_t input)
{  asm volatile("lock xadd %0,%1" : "+r" (input), "+m" (*value) : "r" (input), "m" (*value)); return input; }

INLINE int32_t atomic_cmpxchg(int32_t volatile* value, const int32_t input, int32_t comparand)
{  asm volatile("lock cmpxchg %2,%0" : "=m" (*value), "=a" (comparand) : "r" (input), "m" (*value), "a" (comparand) : "flags"); return comparand; }

#if defined(__X86_64__)

  typedef int64_t atomic_t;

  INLINE int64_t atomic_add(int64_t volatile* value, int64_t input)
  {  asm volatile("lock xaddq %0,%1" : "+r" (input), "+m" (*value) : "r" (input), "m" (*value));  return input;  }

  INLINE int64_t atomic_cmpxchg(int64_t volatile* value, const int64_t input, int64_t comparand)
  {  asm volatile("lock cmpxchgq %2,%0" : "+m" (*value), "+a" (comparand) : "r" (input), "m" (*value), "r" (comparand) : "flags"); return comparand;  }

#else

  typedef int32_t atomic_t;

#endif /* defined(__X86_64__) */

#define GBE_COMPILER_READ_WRITE_BARRIER    asm volatile("" ::: "memory");
#define GBE_COMPILER_WRITE_BARRIER         GBE_COMPILER_READ_WRITE_BARRIER
#define GBE_COMPILER_READ_BARRIER          GBE_COMPILER_READ_WRITE_BARRIER

#endif /* __MSVC__ */

template <typename T>
INLINE T __load_acquire(volatile T *ptr)
{
  GBE_COMPILER_READ_WRITE_BARRIER;
  T x = *ptr; // for x86, load == load_acquire
  GBE_COMPILER_READ_WRITE_BARRIER;
  return x;
}

template <typename T>
INLINE void __store_release(volatile T *ptr, T x)
{
  GBE_COMPILER_READ_WRITE_BARRIER;
  *ptr = x; // for x86, store == store_release
  GBE_COMPILER_READ_WRITE_BARRIER;
}
#endif /* __GBE_INTRINSICS_HPP__ */

