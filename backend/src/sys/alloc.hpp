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

/**
 * \file alloc.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_ALLOC_HPP__
#define __GBE_ALLOC_HPP__

#include "sys/platform.hpp"
#include "sys/assert.hpp"
#include <algorithm>
#include <limits>

namespace gbe
{
  /*! regular allocation */
  void* memAlloc(size_t size);
  void  memFree(void *ptr);

  /*! Aligned allocation */
  void* alignedMalloc(size_t size, size_t align = 64);
  void  alignedFree(void* ptr);

  /*! Monitor memory allocations */
#if GBE_DEBUG_MEMORY
  void* MemDebuggerInsertAlloc(void*, const char*, const char*, int);
  void  MemDebuggerRemoveAlloc(void *ptr);
  void  MemDebuggerDumpAlloc(void);
  void  MemDebuggerInitializeMem(void *mem, size_t sz);
  void  MemDebuggerEnableMemoryInitialization(bool enabled);
#else
  INLINE void* MemDebuggerInsertAlloc(void *ptr, const char*, const char*, int) {return ptr;}
  INLINE void  MemDebuggerRemoveAlloc(void *ptr) {}
  INLINE void  MemDebuggerDumpAlloc(void) {}
  INLINE void  MemDebuggerInitializeMem(void *mem, size_t sz) {}
  INLINE void  MemDebuggerEnableMemoryInitialization(bool enabled) {}
#endif /* GBE_DEBUG_MEMORY */

  /*! Properly handle the allocated type */
  template <typename T>
  T* _MemDebuggerInsertAlloc(T *ptr, const char *file, const char *function, int line) {
    MemDebuggerInsertAlloc(ptr, file, function, line);
    return ptr;
  }
} /* namespace gbe */

/*! Declare a class with custom allocators */
#define GBE_CLASS(TYPE) \
  GBE_STRUCT(TYPE) \
private:

/*! Declare a structure with custom allocators */
#define GBE_STRUCT(TYPE) \
public: \
  void* operator new(size_t size) { \
    return gbe::alignedMalloc(size, GBE_DEFAULT_ALIGNMENT); \
  } \
  void  operator delete(void* ptr) { return gbe::alignedFree(ptr); } \
  void* operator new[](size_t size) { \
   return gbe::alignedMalloc(size, GBE_DEFAULT_ALIGNMENT); \
  } \
  void  operator delete[](void* ptr) { return gbe::alignedFree(ptr); } \
  void* operator new(size_t size, void *p) { return p; } \
  void  operator delete(void* ptr, void *p) {/*do nothing*/} \
  void* operator new[](size_t size, void *p) { return p; } \
  void  operator delete[](void* ptr, void *p) { /*do nothing*/ }

/*! Macros to handle allocation position */
#define GBE_NEW(T,...) \
  gbe::_MemDebuggerInsertAlloc(new T(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)

#define GBE_NEW_NO_ARG(T) \
  gbe::_MemDebuggerInsertAlloc(new T, __FILE__, __FUNCTION__, __LINE__)

#define GBE_NEW_ARRAY(T,N,...) \
  gbe::_MemDebuggerInsertAlloc(new T[N](__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)

#define GBE_NEW_ARRAY_NO_ARG(T,N)\
  gbe::_MemDebuggerInsertAlloc(new T[N], __FILE__, __FUNCTION__, __LINE__)

#define GBE_NEW_P(T,X,...) \
  gbe::_MemDebuggerInsertAlloc(new (X) T(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)

#define GBE_DELETE(X) \
  do { gbe::MemDebuggerRemoveAlloc(X); delete X; } while (0)

#define GBE_DELETE_ARRAY(X) \
  do { gbe::MemDebuggerRemoveAlloc(X); delete[] X; } while (0)

#define GBE_MALLOC(SZ) \
  gbe::MemDebuggerInsertAlloc(gbe::memAlloc(SZ),__FILE__, __FUNCTION__, __LINE__)

#define GBE_FREE(X) \
  do { gbe::MemDebuggerRemoveAlloc(X); gbe::memFree(X); } while (0)

#define GBE_ALIGNED_FREE(X) \
  do { gbe::MemDebuggerRemoveAlloc(X); gbe::alignedFree(X); } while (0)

#define GBE_ALIGNED_MALLOC(SZ,ALIGN) \
  gbe::MemDebuggerInsertAlloc(gbe::alignedMalloc(SZ,ALIGN),__FILE__, __FUNCTION__, __LINE__)

namespace gbe
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
      if (ALIGNOF(T) > sizeof(uintptr_t))
        return (pointer) GBE_ALIGNED_MALLOC(n*sizeof(T), ALIGNOF(T));
      else
        return (pointer) GBE_MALLOC(n * sizeof(T));
    }
    INLINE void deallocate(pointer p, size_type) {
      if (ALIGNOF(T) > sizeof(uintptr_t))
        GBE_ALIGNED_FREE(p);
      else
        GBE_FREE(p);
    }
    INLINE size_type max_size(void) const {
      return std::numeric_limits<size_type>::max() / sizeof(T);
    }
    INLINE void construct(pointer p, const T& t = T()) { ::new(p) T(t); }
    INLINE void destroy(pointer p) { p->~T(); }
    INLINE bool operator==(Allocator const&) { return true; }
    INLINE bool operator!=(Allocator const& a) { return !operator==(a); }
  };

// Deactivate fast allocators
#ifndef GBE_DEBUG_SPECIAL_ALLOCATOR
#define GBE_DEBUG_SPECIAL_ALLOCATOR 0
#endif

  /*! A growing pool never gives memory to the system but chain free elements
   *  together such as deallocation can be quickly done
   */
  template <typename T>
  class GrowingPool
  {
  public:
    GrowingPool(uint32_t elemNum = 1) :
      curr(GBE_NEW(GrowingPoolElem, elemNum <= 1 ? 1 : elemNum)),
      free(NULL), full(NULL), freeList(NULL) {}
    ~GrowingPool(void) {
      GBE_SAFE_DELETE(curr);
      GBE_SAFE_DELETE(free);
      GBE_SAFE_DELETE(full);
    }
    void *allocate(void) {
#if GBE_DEBUG_SPECIAL_ALLOCATOR
      return GBE_ALIGNED_MALLOC(sizeof(T), ALIGNOF(T));
#else
      // Pick up an element from the free list
      if (this->freeList != NULL) {
        void *data = (void*) freeList;
        this->freeList = *(void**) freeList;
        return data;
      }

      // Pick up an element from the current block (if not full)
      if (this->curr->allocated < this->curr->maxElemNum) {
        void *data = (T*) curr->data + curr->allocated++;
        return data;
      }

      // Block is full
      this->curr->next = this->full;
      this->full = this->curr;

      // Try to pick up a free block
      if (this->free) this->getFreeBlock();

      // No free block we must allocate a new one
      else
        this->curr = GBE_NEW(GrowingPoolElem, 2 * this->curr->maxElemNum);

      void *data = (T*) curr->data + curr->allocated++;
      return data;
#endif /* GBE_DEBUG_SPECIAL_ALLOCATOR */
    }
    void deallocate(void *t) {
      if (t == NULL) return;
#if GBE_DEBUG_SPECIAL_ALLOCATOR
      GBE_ALIGNED_FREE(t);
#else
      *(void**) t = this->freeList;
      this->freeList = t;
#endif /* GBE_DEBUG_SPECIAL_ALLOCATOR */
    }
    void rewind(void) {
#if GBE_DEBUG_SPECIAL_ALLOCATOR == 0
      // All free elements return to their blocks
      this->freeList = NULL;

      // Put back current block in full list
      if (this->curr) {
        this->curr->next = this->full;
        this->full = this->curr;
        this->curr = NULL;
      }

      // Reverse the chain list and mark all blocks as empty
      while (this->full) {
        GrowingPoolElem *next = this->full->next;
        this->full->allocated = 0;
        this->full->next = this->free;
        this->free = this->full;
        this->full = next;
      }

      // Provide a valid current block
      this->getFreeBlock();
#endif /* GBE_DEBUG_SPECIAL_ALLOCATOR */
    }
  private:
    /*! Pick-up a free block */
    INLINE void getFreeBlock(void) {
      GBE_ASSERT(this->free);
      this->curr = this->free;
      this->free = this->free->next;
      this->curr->next = NULL;
    }
    /*! Chunk of elements to allocate */
    class GrowingPoolElem
    {
      friend class GrowingPool;
      GrowingPoolElem(size_t elemNum) {
        const size_t sz = std::max(sizeof(T), sizeof(void*));
        this->data = (T*) GBE_ALIGNED_MALLOC(elemNum * sz, ALIGNOF(T));
        this->next = NULL;
        this->maxElemNum = elemNum;
        this->allocated = 0;
      }
      ~GrowingPoolElem(void) {
        GBE_ALIGNED_FREE(this->data);
        if (this->next) GBE_DELETE(this->next);
      }
      T *data;
      GrowingPoolElem *next;
      size_t allocated, maxElemNum;
    };
    GrowingPoolElem *curr; //!< To get new element from
    GrowingPoolElem *free; //!< Blocks that can be reused (after rewind)
    GrowingPoolElem *full; //!< Blocks fully used
    void *freeList;        //!< Elements that have been deallocated
    GBE_CLASS(GrowingPool);
  };

/*! Helper macros to build and destroy objects with a growing pool */
#define DECL_POOL(TYPE, POOL) \
  GrowingPool<TYPE> POOL; \
  template <typename... Args> \
  TYPE *new##TYPE(Args&&... args) { \
    return new (POOL.allocate()) TYPE(args...); \
  } \
  void delete##TYPE(TYPE *ptr) { \
    ptr->~TYPE(); \
    POOL.deallocate(ptr); \
  }

  /*! A linear allocator just grows and does not reuse freed memory. It can
   *  however allocate objects of any size
   */
  class LinearAllocator
  {
  public:
    /*! Initiate the linear allocator (one segment is allocated) */
    LinearAllocator(size_t minSize = CACHE_LINE, size_t maxSize = 64*KB);
    /*! Free up everything */
    ~LinearAllocator(void);
    /*! Allocate size bytes */
    void *allocate(size_t size);
    /*! Nothing here */
    INLINE void deallocate(void *ptr) {
#if GBE_DEBUG_SPECIAL_ALLOCATOR
      if (ptr) GBE_ALIGNED_FREE(ptr);
#endif /* GBE_DEBUG_SPECIAL_ALLOCATOR */
    }
  private:
    /*! Helds an allocated segment of memory */
    struct Segment {
      /*! Allocate a new segment */
      Segment(size_t size);
      /*! Destroy the segment and the next ones */
      ~Segment(void);
      /* Size of the segment */
      size_t size;
      /*! Offset to the next free bytes (if any left) */
      size_t offset;
      /*! Pointer to valid data */
      void *data;
      /*! Pointer to the next segment */
      Segment *next;
      /*! Use internal allocator */
      GBE_STRUCT(Segment);
    };
    /*! Points to the current segment we can allocate from */
    Segment *curr;
    /*! Maximum segment size */
    size_t maxSize;
    /*! Use internal allocator */
    GBE_CLASS(LinearAllocator);
  };

} /* namespace gbe */

#endif /* __GBE_ALLOC_HPP__ */

