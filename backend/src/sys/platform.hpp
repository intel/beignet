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

#ifndef __GBE_PLATFORM_HPP__
#define __GBE_PLATFORM_HPP__
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
/// CPU architecture
////////////////////////////////////////////////////////////////////////////////

/* detect 32 or 64 platform */
#if defined(__x86_64__) || defined(__ia64__) || defined(_M_X64)
#define __X86_64__
#else
#define __X86__
#endif

/* We require SSE ... */
#ifndef __SSE__
#define __SSE__
#endif

/* ... and SSE2 */
#ifndef __SSE2__
#define __SSE2__
#endif

#if defined(_INCLUDED_IMM)
// #define __AVX__
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1600) && !defined(__INTEL_COMPILER) || defined(_DEBUG) && defined(_WIN32)
#define __NO_AVX__
#endif

#if defined(_MSC_VER) && !defined(__SSE4_2__)
// #define __SSE4_2__  //! activates SSE4.2 support
#endif

////////////////////////////////////////////////////////////////////////////////
/// Operating system
////////////////////////////////////////////////////////////////////////////////

/* detect Linux platform */
#if defined(linux) || defined(__linux__) || defined(__LINUX__)
#  if !defined(__LINUX__)
#     define __LINUX__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

/* detect FreeBSD platform */
#if defined(__FreeBSD__) || defined(__FREEBSD__)
#  if !defined(__FREEBSD__)
#     define __FREEBSD__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

/* detect Windows 95/98/NT/2000/XP/Vista/7 platform */
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)) && !defined(__CYGWIN__)
#  if !defined(__WIN32__)
#     define __WIN32__
#  endif
#endif

/* detect Cygwin platform */
#if defined(__CYGWIN__)
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

/* detect MAC OS X platform */
#if defined(__APPLE__) || defined(MACOSX) || defined(__MACOSX__)
#  if !defined(__MACOSX__)
#     define __MACOSX__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

/* try to detect other Unix systems */
#if defined(__unix__) || defined (unix) || defined(__unix) || defined(_unix)
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

////////////////////////////////////////////////////////////////////////////////
/// Compiler
////////////////////////////////////////////////////////////////////////////////

/*! GCC compiler */
#ifdef __GNUC__
// #define __GNUC__
#endif

/*! Intel compiler */
#ifdef __INTEL_COMPILER
#define __ICC__
#endif

/*! Visual C compiler */
#ifdef _MSC_VER
#define __MSVC__
#endif

////////////////////////////////////////////////////////////////////////////////
/// Makros
////////////////////////////////////////////////////////////////////////////////

#ifdef __WIN32__
#define __dllexport extern "C" __declspec(dllexport)
#define __dllimport extern "C" __declspec(dllimport)
#else
#define __dllexport extern "C"
#define __dllimport extern "C"
#endif

#ifdef __MSVC__
#undef NOINLINE
#define NOINLINE             __declspec(noinline)
#define INLINE               __forceinline
#define RESTRICT             __restrict
#define THREAD               __declspec(thread)
#define ALIGNED(...)         __declspec(align(__VA_ARGS__))
//#define __FUNCTION__           __FUNCTION__
#define debugbreak()         __debugbreak()
#else
#undef NOINLINE
#undef INLINE
#define NOINLINE        __attribute__((noinline))
#define INLINE          inline __attribute__((always_inline))
#define RESTRICT        __restrict
#define THREAD          __thread
#define ALIGNED(...)    __attribute__((aligned(__VA_ARGS__)))
#define __FUNCTION__    __PRETTY_FUNCTION__
#define debugbreak()    asm ("int $3")
#endif

/*! Modern x86 processors */
#define CACHE_LINE 64
#define CACHE_LINE_ALIGNED ALIGNED(CACHE_LINE)

#ifdef __GNUC__
  #define MAYBE_UNUSED __attribute__((used))
#else
  #define MAYBE_UNUSED
#endif

#if defined(_MSC_VER)
#define __builtin_expect(expr,b) expr
#endif

/*! Debug syntactic sugar */
#ifdef NDEBUG
#define IF_DEBUG(EXPR)
#else
#define IF_DEBUG(EXPR) EXPR
#endif /* NDEBUG */

/*! Debug printing macros */
#define STRING(x) #x
#define PING std::cout << __FILE__ << " (" << __LINE__ << "): " << __FUNCTION__ << std::endl
#define PRINT(x) std::cout << STRING(x) << " = " << (x) << std::endl

/*! Branch hint */
#define LIKELY(x)       __builtin_expect(!!(x),1)
#define UNLIKELY(x)     __builtin_expect((x),0)

/*! Stringify macros */
#define JOIN(X, Y) _DO_JOIN(X, Y)
#define _DO_JOIN(X, Y) _DO_JOIN2(X, Y)
#define _DO_JOIN2(X, Y) X##Y

/*! Run-time assertion */
#ifndef NDEBUG
#define GBE_ASSERT(EXPR) do {            \
  if (UNLIKELY(!(EXPR))) assert(EXPR);  \
} while (0)
#else
#define GBE_ASSERT(EXPR) do { } while (0)
#endif

/*! Compile-time assertion */
#define STATIC_ASSERT(value)                                     \
  struct JOIN(__,JOIN(__,__LINE__)) { int x[(value) ? 1 : -1]; }

/*! Fatal error macros */
#define NOT_IMPLEMENTED FATAL ("Not implemented")
#define FATAL_IF(COND, MSG)                          \
do {                                                 \
  if(UNLIKELY(COND)) FATAL(MSG);                     \
} while (0)

/* Safe deletion macros */
#define GBE_SAFE_DELETE_ARRAY(x) do { if (x != NULL) GBE_DELETE_ARRAY(x); } while (0)
#define GBE_SAFE_DELETE(x) do { if (x != NULL) GBE_DELETE(x); } while (0)

/* Number of elements in an array */
#define ARRAY_ELEM_NUM(x) (sizeof(x) / sizeof(x[0]))

/* Align X on A */
#define ALIGN(X,A) (((X) % (A)) ? ((X) + (A) - ((X) % (A))) : (X))

/*! Produce a string from the macro locatiom */
#define HERE (STRING(__LINE__) "@" __FILE__)

/*! Portable AlignOf */
template <typename T>
struct AlignOf
{
  struct Helper { char x; T t; };
  static const size_t value = offsetof(Helper, t);
};

////////////////////////////////////////////////////////////////////////////////
/// Visibility parameters (DLL export and so on)
////////////////////////////////////////////////////////////////////////////////
#if defined __WIN32__
  #if defined __GNUC__
    #define GBE_EXPORT_SYMBOL __attribute__ ((dllexport))
    #define GBE_IMPORT_SYMBOL __attribute__ ((dllimport))
  #else
    #define GBE_IMPORT_SYMBOL __declspec(dllimport)
    #define GBE_EXPORT_SYMBOL __declspec(dllexport)
  #endif /* __GNUC__ */
#else
  #define GBE_EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #define GBE_IMPORT_SYMBOL
#endif /* __WIN32__ */

////////////////////////////////////////////////////////////////////////////////
/// Basic Types
////////////////////////////////////////////////////////////////////////////////

#if defined(__MSVC__)
typedef          __int64  int64;
typedef unsigned __int64 uint64;
typedef          __int32  int32;
typedef unsigned __int32 uint32;
typedef          __int16  int16;
typedef unsigned __int16 uint16;
typedef          __int8    int8;
typedef unsigned __int8   uint8;
#else
typedef          long long  int64;
typedef unsigned long long uint64;
typedef                int  int32;
typedef unsigned       int uint32;
typedef              short  int16;
typedef unsigned     short uint16;
typedef               char   int8;
typedef unsigned      char  uint8;
#endif

#if defined(__X86_64__)
typedef int64 index_t;
#else
typedef int32 index_t;
#endif

/*! To protect some classes from being copied */
class NonCopyable
{
protected:
  INLINE NonCopyable(void) {}
  INLINE ~NonCopyable(void) {}
private: 
  INLINE NonCopyable(const NonCopyable&) {}
  INLINE NonCopyable& operator= (const NonCopyable&) {return *this;}
};

////////////////////////////////////////////////////////////////////////////////
/// Disable some compiler warnings
////////////////////////////////////////////////////////////////////////////////

#ifdef __ICC__
#pragma warning(disable:265)  // floating-point operation result is out of range
#pragma warning(disable:383)  // value copied to temporary, reference to temporary used
#pragma warning(disable:869)  // parameter was never referenced
#pragma warning(disable:981)  // operands are evaluated in unspecified order
#pragma warning(disable:1418) // external function definition with no prior declaration
#pragma warning(disable:1419) // external declaration in primary source file
#pragma warning(disable:1572) // floating-point equality and inequality comparisons are unreliable
#pragma warning(disable:1125) // virtual function override intended?
#endif /* __ICC__ */

////////////////////////////////////////////////////////////////////////////////
/// Default Includes and Functions
////////////////////////////////////////////////////////////////////////////////

#include "sys/constants.hpp"
#include "sys/alloc.hpp"

namespace gbe
{
  /*! selects */
  INLINE bool  select(bool s, bool  t , bool f) { return s ? t : f; }
  INLINE int   select(bool s, int   t,   int f) { return s ? t : f; }
  INLINE float select(bool s, float t, float f) { return s ? t : f; }

  /*! Fatal error function */
  void FATAL(const std::string&);

  /*! Return the next power of 2 */
  INLINE uint32 nextHighestPowerOf2(uint32 x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
  }

  INLINE uint32 logi2(uint32 x) {
    uint32 r = 0;
    while(x >>= 1) r++;
    return r;
  }

  template<uint32 N>
  INLINE uint32 isPowerOf(uint32 i) {
    while (i > 1) {
      if (i%N) return false;
      i = i/N;
    }
    return true;
  }
  template<> INLINE uint32 isPowerOf<2>(uint32 i) { return ((i-1)&i) == 0; }

  /*! random functions */
  template<typename T> T   random() { return T(0); }
  template<> INLINE int    random() { return int(rand()); }
  template<> INLINE uint32 random() { return uint32(rand()); }
  template<> INLINE float  random() { return random<uint32>()/float(RAND_MAX); }
  template<> INLINE double random() { return random<uint32>()/double(RAND_MAX); }

  /** returns performance counter in seconds */
  double getSeconds();

} /* namespace gbe */

#endif /* __GBE_PLATFORM_HPP__ */

