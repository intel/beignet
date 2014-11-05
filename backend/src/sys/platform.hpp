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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_PLATFORM_HPP__
#define __GBE_PLATFORM_HPP__

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <istream>
#include <string>
#include <cassert>
#include <new>

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
#define DEBUGBREAK()         __debugbreak()
#else
#undef NOINLINE
#undef INLINE
#define NOINLINE        __attribute__((noinline))
#define INLINE          inline __attribute__((always_inline))
#define RESTRICT        __restrict
#define THREAD          __thread
#define ALIGNED(...)    __attribute__((aligned(__VA_ARGS__)))
#define __FUNCTION__    __PRETTY_FUNCTION__
#define DEBUGBREAK()    asm ("int $3")
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
#if GBE_DEBUG
#define IF_DEBUG(EXPR) EXPR
#else
#define IF_DEBUG(EXPR)
#endif /* GBE_DEBUG */

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
#if GBE_DEBUG
#define GBE_ASSERT(EXPR) do { \
  if (UNLIKELY(!(EXPR))) \
    gbe::onFailedAssertion(#EXPR, __FILE__, __FUNCTION__, __LINE__); \
} while (0)
#define GBE_ASSERTM(EXPR, MSG) do { \
  if (UNLIKELY(!(EXPR))) \
    gbe::onFailedAssertion(MSG, __FILE__, __FUNCTION__, __LINE__); \
} while (0)
#else
#define GBE_ASSERT(EXPR) do { } while (0)
#define GBE_ASSERTM(EXPR, MSG) do { } while (0)
#endif /* GBE_DEBUG */

#define NOT_IMPLEMENTED GBE_ASSERTM (false, "Not implemented")
#define NOT_SUPPORTED GBE_ASSERTM (false, "Not supported")

/*! Fatal error macros */
#define FATAL_IF(COND, MSG) \
do { \
  if(UNLIKELY(COND)) FATAL(MSG); \
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

/*! Typesafe encapusalation of a type (mostly for integers) */
#define TYPE_SAFE(SAFE, UNSAFE) \
class SAFE \
{ \
public: \
  INLINE SAFE(void) {} \
  explicit INLINE SAFE(uint16_t unsafe) : unsafe(unsafe) {} \
  INLINE operator UNSAFE (void) const { return unsafe; } \
  UNSAFE value(void) const { return unsafe; } \
private: \
  UNSAFE unsafe; \
};

/*! Default alignment for the platform */
#define GBE_DEFAULT_ALIGNMENT 16

/*! Useful constants */
#define KB 1024
#define MB (KB*KB)

/*! Portable AlignOf */
template <typename T>
struct AlignOf {
  struct Helper { char x; T t; };
  enum { value = offsetof(Helper, t) };
};

//gcc 4.8+ support C++11 alignof keyword
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 8)
#define ALIGNOF(T) (alignof(T))
#else
#define ALIGNOF(T) (AlignOf<T>::value)
#endif

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
typedef          __int64_t  int64_t;
typedef unsigned __int64_t uint64_t;
typedef          __int32_t  int32_t;
typedef unsigned __int32_t uint32_t;
typedef          __int16_t  int16_t;
typedef unsigned __int16_t uint16_t;
typedef          __int8_t    int8_t;
typedef unsigned __int8_t   uint8_t;
#else
#include <cstdint>
#endif

#if defined(__X86_64__)
typedef int64_t index_t;
#else
typedef int32_t index_t;
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

#define TO_MAGIC(A, B, C, D)  (A<<24 | B<<16 | C<<8 | D)

class Serializable
{
public:
  INLINE Serializable(void) = default;
  INLINE Serializable(const Serializable&) = default;
  INLINE Serializable& operator= (const Serializable&) = default;

  virtual size_t serializeToBin(std::ostream& outs) = 0;
  virtual size_t deserializeFromBin(std::istream& ins) = 0;

  /* These two will follow LLVM's ABI. */
  virtual size_t serializeToLLVM(void) { return 0;/* not implemented now. */}
  virtual size_t deserializeFromLLVM(void) { return 0;/* not implemented now. */}

  virtual void printStatus(int indent = 0, std::ostream& outs = std::cout) { }

  virtual ~Serializable(void) { }

protected:
  static std::string indent_to_str(int indent) {
    std::string ind(indent, ' ');
    return ind;
  }
};

/* Help Macro for serialization. */
#define SERIALIZE_OUT(elt, out, sz)			\
     do {						\
	  auto tmp_val = elt;				\
	  out.write((char *)(&tmp_val), sizeof(elt));	\
	  sz += sizeof(elt);				\
     } while(0)

#define DESERIALIZE_IN(elt, in, sz)			\
     do {						\
	  in.read((char *)(&(elt)), sizeof(elt));	\
	  sz += sizeof(elt);				\
     } while(0)

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
  INLINE uint32_t nextHighestPowerOf2(uint32_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
  }

  INLINE uint32_t logi2(uint32_t x) {
    uint32_t r = 0;
    while(x >>= 1) r++;
    return r;
  }

  template<uint32_t N>
  INLINE uint32_t isPowerOf(uint32_t i) {
    while (i > 1) {
      if (i%N) return false;
      i = i/N;
    }
    return true;
  }
  template<> INLINE uint32_t isPowerOf<2>(uint32_t i) { return ((i-1)&i) == 0; }

  /*! random functions */
  template<typename T> T     random() { return T(0); }
  template<> INLINE int32_t  random() { return int(rand()); }
  template<> INLINE uint32_t random() { return uint32_t(rand()); }
  template<> INLINE float    random() { return random<uint32_t>()/float(RAND_MAX); }
  template<> INLINE double   random() { return random<uint32_t>()/double(RAND_MAX); }

  /** returns performance counter in seconds */
  double getSeconds();

} /* namespace gbe */

#endif /* __GBE_PLATFORM_HPP__ */

