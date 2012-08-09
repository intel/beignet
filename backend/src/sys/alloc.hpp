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

#ifndef __PF_ALLOC_HPP__
#define __PF_ALLOC_HPP__

#include "sys/platform.hpp"
#include <cstdlib>
#include <new>

namespace pf
{
  /*! regular allocation */
  void* malloc(size_t size);
  void* realloc(void *ptr, size_t size);
  void  free(void *ptr);

  /*! Aligned allocation */
  void* alignedMalloc(size_t size, size_t align = 64);
  void  alignedFree(void* ptr);

  /*! Monitor memory allocations */
#if PF_DEBUG_MEMORY
  void* MemDebuggerInsertAlloc(void*, const char*, const char*, int);
  void  MemDebuggerRemoveAlloc(void *ptr);
  void  MemDebuggerDumpAlloc(void);
  void  MemDebuggerInitializeMem(void *mem, size_t sz);
  void  MemDebuggerEnableMemoryInitialization(bool enabled);
  void  MemDebuggerStart(void);
  void  MemDebuggerEnd(void);
#else
  INLINE void* MemDebuggerInsertAlloc(void *ptr, const char*, const char*, int) {return ptr;}
  INLINE void  MemDebuggerRemoveAlloc(void *ptr) {}
  INLINE void  MemDebuggerDumpAlloc(void) {}
  INLINE void  MemDebuggerInitializeMem(void *mem, size_t sz) {}
  INLINE void  MemDebuggerEnableMemoryInitialization(bool enabled) {}
  INLINE void  MemDebuggerStart(void) {}
  INLINE void  MemDebuggerEnd(void) {}
#endif /* PF_DEBUG_MEMORY */

  /*! Properly handle the allocated type */
  template <typename T>
  T* _MemDebuggerInsertAlloc(T *ptr, const char *file, const char *function, int line) {
    MemDebuggerInsertAlloc(ptr, file, function, line);
    return ptr;
  }
} /* namespace pf */

/*! Declare a structure with custom allocators */
#define PF_STRUCT(TYPE)                                      \
  void* operator new(size_t size)   {                        \
    if (AlignOf<TYPE>::value > sizeof(uintptr_t))            \
      return pf::alignedMalloc(size, AlignOf<TYPE>::value);  \
    else                                                     \
      return pf::malloc(size);                               \
  }                                                          \
  void* operator new[](size_t size)   {                      \
    if (AlignOf<TYPE>::value > sizeof(uintptr_t))            \
      return pf::alignedMalloc(size, AlignOf<TYPE>::value);  \
    else                                                     \
      return pf::malloc(size);                               \
  }                                                          \
  void  operator delete(void* ptr) {                         \
    if (AlignOf<TYPE>::value > sizeof(uintptr_t))            \
      return pf::alignedFree(ptr);                           \
    else                                                     \
      return pf::free(ptr);                                  \
  }                                                          \
  void  operator delete[](void* ptr) {                       \
    if (AlignOf<TYPE>::value > sizeof(uintptr_t))            \
      return pf::alignedFree(ptr);                           \
    else                                                     \
      return pf::free(ptr);                                  \
  }                                                          \

/*! Declare a class with custom allocators */
#define PF_CLASS(TYPE) \
public:                \
  PF_STRUCT(TYPE)      \
private:

/*! Declare an aligned structure */
#define PF_ALIGNED_STRUCT(ALIGN)                                              \
  void* operator new(size_t size)   { return pf::alignedMalloc(size, ALIGN); }\
  void* operator new[](size_t size) { return pf::alignedMalloc(size, ALIGN); }\
  void  operator delete(void* ptr)   { pf::alignedFree(ptr); }                \
  void  operator delete[](void* ptr) { pf::alignedFree(ptr); }

/*! Declare an aligned class */
#define PF_ALIGNED_CLASS(ALIGN)    \
public:                            \
  PF_ALIGNED_STRUCT(ALIGN)         \
private:

/*! Macros to handle allocation position */
#define PF_NEW(T,...)               \
  pf::_MemDebuggerInsertAlloc(new T(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)

#define PF_NEW_ARRAY(T,N,...)       \
  pf::_MemDebuggerInsertAlloc(new T[N](__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)

#define PF_NEW_P(T,X,...)           \
  pf::_MemDebuggerInsertAlloc(new (X) T(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)

#define PF_DELETE(X)                \
  do { pf::MemDebuggerRemoveAlloc(X); delete X; } while (0)

#define PF_DELETE_ARRAY(X)          \
  do { pf::MemDebuggerRemoveAlloc(X); delete[] X; } while (0)

#define PF_MALLOC(SZ)               \
  pf::MemDebuggerInsertAlloc(pf::malloc(SZ),__FILE__, __FUNCTION__, __LINE__)

#define PF_REALLOC(PTR, SZ)         \
  pf::MemDebuggerInsertAlloc(pf::realloc(PTR, SZ),__FILE__, __FUNCTION__, __LINE__)

#define PF_FREE(X)                  \
  do { pf::MemDebuggerRemoveAlloc(X); pf::free(X); } while (0)

#define PF_ALIGNED_FREE(X)          \
  do { pf::MemDebuggerRemoveAlloc(X); pf::alignedFree(X); } while (0)

#define PF_ALIGNED_MALLOC(SZ,ALIGN) \
  pf::MemDebuggerInsertAlloc(pf::alignedMalloc(SZ,ALIGN),__FILE__, __FUNCTION__, __LINE__)

namespace pf
{
  /*! STL compliant allocator to intercept all memory allocations */
  template<typename T>
  class Allocator {
  public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename std::allocator<void>::const_pointer void_allocator_ptr;
    template<typename U>
    struct rebind { typedef Allocator<U> other; };

    INLINE Allocator(void) {}
    INLINE ~Allocator(void) {}
    INLINE Allocator(Allocator const&) {}
    template<typename U>
    INLINE Allocator(Allocator<U> const&) {}
    INLINE pointer address(reference r) { return &r; }
    INLINE const_pointer address(const_reference r) { return &r; }
    INLINE pointer allocate(size_type n, void_allocator_ptr = 0) {
      if (AlignOf<T>::value > sizeof(uintptr_t))
        return (pointer) PF_ALIGNED_MALLOC(n*sizeof(T), AlignOf<T>::value);
      else
        return (pointer) PF_MALLOC(n * sizeof(T));
    }
    INLINE void deallocate(pointer p, size_type) {
      if (AlignOf<T>::value > sizeof(uintptr_t))
        PF_ALIGNED_FREE(p);
      else
        PF_FREE(p);
    }
    INLINE size_type max_size(void) const {
      return std::numeric_limits<size_type>::max() / sizeof(T);
    }
    INLINE void construct(pointer p, const T& t = T()) { ::new(p) T(t); }
    INLINE void destroy(pointer p) { p->~T(); }
    INLINE bool operator==(Allocator const&) { return true; }
    INLINE bool operator!=(Allocator const& a) { return !operator==(a); }
  };

  /*! A growing pool never deallocates */
  template <typename T>
  class GrowingPool
  {
  public:
    GrowingPool(void) : current(PF_NEW(GrowingPoolElem, 1)) {}
    ~GrowingPool(void) { PF_ASSERT(current); PF_DELETE(current); }
    T *allocate(void) {
      if (UNLIKELY(current->allocated == current->maxElemNum)) {
        GrowingPoolElem *elem = PF_NEW(GrowingPoolElem, 2 * current->maxElemNum);
        elem->next = current;
        current = elem;
      }
      T *data = current->data + current->allocated++;
      return data;
    }
  private:
    /*! Chunk of elements to allocate */
    class GrowingPoolElem
    {
      friend class GrowingPool;
      GrowingPoolElem(size_t elemNum) {
        this->data = PF_NEW_ARRAY(T, elemNum);
        this->next = NULL;
        this->maxElemNum = elemNum;
        this->allocated = 0;
      }
      ~GrowingPoolElem(void) {
        PF_ASSERT(this->data);
        PF_DELETE_ARRAY(this->data);
        if (this->next) PF_DELETE(this->next);
      }
      T *data;
      GrowingPoolElem *next;
      size_t allocated, maxElemNum;
    };
    GrowingPoolElem *current;
    PF_CLASS(GrowingPool);
  };
} /* namespace pf */

#endif /* __PF_ALLOC_HPP__ */

