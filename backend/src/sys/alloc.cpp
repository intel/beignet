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

/**
 * \file alloc.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 *  Provides facilities to track allocations and pre-initialize memory at
 *  memory allocation and memory free time
 */
#include "sys/alloc.hpp"
#include "sys/atomic.hpp"
#include "sys/mutex.hpp"

#if GBE_DEBUG_MEMORY
#include <tr1/unordered_map>
#include <cstring>
#endif /* GBE_DEBUG_MEMORY */

#if defined(__ICC__)
#include <stdint.h>
#endif /* __ICC__ */
#include <map>
#include <vector>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////
/// Memory debugger
////////////////////////////////////////////////////////////////////////////////

#if GBE_DEBUG_MEMORY
namespace gbe
{
  /*! Store each allocation data */
  struct AllocData {
    INLINE AllocData(void) {}
    INLINE AllocData(int fileName_, int functionName_, int line_, intptr_t alloc_) :
      fileName(fileName_), functionName(functionName_), line(line_), alloc(alloc_) {}
    int fileName, functionName, line;
    intptr_t alloc;
  };

  /*! Store allocation information */
  struct MemDebugger {
    MemDebugger(void) : unfreedNum(0), allocNum(0) {}
    ~MemDebugger(void) { this->dumpAlloc(); }
    void* insertAlloc(void *ptr, const char *file, const char *function, int line);
    void removeAlloc(void *ptr);
    void dumpAlloc(void);
    void dumpData(const AllocData &data);
    /*! Count the still unfreed allocations */
    volatile intptr_t unfreedNum;
    /*! Total number of allocations done */
    volatile intptr_t allocNum;
    /*! Sorts the file name and function name strings */
    std::tr1::unordered_map<const char*, int> staticStringMap;
    /*! Each element contains the actual string */
    std::vector<const char*> staticStringVector;
    std::map<uintptr_t, AllocData> allocMap;
    /*! Protect the memory debugger accesses */
    MutexSys mutex;
  };

  void* MemDebugger::insertAlloc(void *ptr, const char *file, const char *function, int line)
  {
    if (ptr == NULL) return ptr;
    Lock<MutexSys> lock(mutex);
    const uintptr_t iptr = (uintptr_t) ptr;
    if (UNLIKELY(allocMap.find(iptr) != allocMap.end())) {
      this->dumpData(allocMap.find(iptr)->second);
      FATAL("Pointer already in map");
    }
    const auto fileIt = staticStringMap.find(file);
    const auto functionIt = staticStringMap.find(function);
    int fileName, functionName;
    if (fileIt == staticStringMap.end()) {
      staticStringVector.push_back(file);
      staticStringMap[file] = fileName = int(staticStringVector.size()) - 1;
    } else
      fileName = staticStringMap[file];
    if (functionIt == staticStringMap.end()) {
      staticStringVector.push_back(function);
      staticStringMap[function] = functionName = int(staticStringVector.size()) - 1;
    } else
      functionName = staticStringMap[function];
    allocMap[iptr] = AllocData(fileName, functionName, line, allocNum);
    unfreedNum++;
    allocNum++;
    return ptr;
  }

  void MemDebugger::removeAlloc(void *ptr)
  {
    if (ptr == NULL) return;
    Lock<MutexSys> lock(mutex);
    const uintptr_t iptr = (uintptr_t) ptr;
    FATAL_IF(allocMap.find(iptr) == allocMap.end(), "Pointer not referenced");
    allocMap.erase(iptr);
    unfreedNum--;
  }

  void MemDebugger::dumpData(const AllocData &data) {
    std::cerr << "ALLOC " << data.alloc << ": " <<
                 "file " << staticStringVector[data.fileName] << ", " <<
                 "function " << staticStringVector[data.functionName] << ", " <<
                 "line " << data.line << std::endl;
  }

  void MemDebugger::dumpAlloc(void) {
    std::cerr << "MemDebugger: Unfreed number: " << unfreedNum << std::endl;
    for (const auto &alloc : allocMap) this->dumpData(alloc.second);
    std::cerr << "MemDebugger: " << staticStringVector.size()
              << " allocated static strings" << std::endl;
  }

  /*! The user can deactivate the memory initialization */
  static bool memoryInitializationEnabled = true;

  /*! Declare C like interface functions here */
  static MemDebugger *memDebugger = NULL;

  /*! Monitor maximum memory requirement in the compiler */
  static MutexSys *sizeMutex = NULL;
  static bool isMutexInitializing = true;
  static size_t memDebuggerCurrSize(0u);
  static size_t memDebuggerMaxSize(0u);
  static void SizeMutexDeallocate(void) { if (sizeMutex) delete sizeMutex; }
  static void SizeMutexAllocate(void) {
    if (sizeMutex == NULL && isMutexInitializing == false) {
      isMutexInitializing = true;
      sizeMutex = new MutexSys;
      atexit(SizeMutexDeallocate);
    }
  }

  /*! Stop the memory debugger */
  static void MemDebuggerEnd(void) {
    MemDebugger *_debug = memDebugger;
    memDebugger = NULL;
    std::cout << "Maximum memory consumption: "
              << std::setprecision(2) << std::fixed
              << float(memDebuggerMaxSize) / 1024. << "KB" << std::endl;
    delete _debug;
    GBE_ASSERT(memDebuggerCurrSize == 0);
  }

  /*! Bring up the debugger at pre-main */
  static struct ForceMemDebugger {
    ForceMemDebugger(void) {
      doesnotmatter = GBE_NEW(int);
      GBE_DELETE(doesnotmatter);
    }
    int *doesnotmatter;
  } forceMemDebugger;

  /*! Start the memory debugger */
  static void MemDebuggerStart(void) {
    if (memDebugger == NULL) {
      atexit(MemDebuggerEnd);
      memDebugger = new MemDebugger;
    }
  }

  void* MemDebuggerInsertAlloc(void *ptr, const char *file, const char *function, int line) {
    if (memDebugger == NULL) MemDebuggerStart();
    return memDebugger->insertAlloc(ptr, file, function, line);
  }
  void MemDebuggerRemoveAlloc(void *ptr) {
    if (memDebugger == NULL) MemDebuggerStart();
    memDebugger->removeAlloc(ptr);
  }
  void MemDebuggerDumpAlloc(void) {
    if (memDebugger == NULL) MemDebuggerStart();
    memDebugger->dumpAlloc();
  }
  void MemDebuggerEnableMemoryInitialization(bool enabled) {
    memoryInitializationEnabled = enabled;
  }
  void MemDebuggerInitializeMem(void *mem, size_t sz) {
    if (memoryInitializationEnabled) std::memset(mem, 0xcd, sz);
  }
} /* namespace gbe */

#endif /* GBE_DEBUG_MEMORY */

namespace gbe
{
#if GBE_DEBUG_MEMORY
  void* memAlloc(size_t size) {
    void *ptr = std::malloc(size + sizeof(size_t));
    *(size_t *) ptr = size;
    MemDebuggerInitializeMem((char*) ptr + sizeof(size_t), size);
    SizeMutexAllocate();
    if (sizeMutex) sizeMutex->lock();
    memDebuggerCurrSize += size;
    memDebuggerMaxSize = std::max(memDebuggerCurrSize, memDebuggerMaxSize);
    if (sizeMutex) sizeMutex->unlock();
    return (char *) ptr + sizeof(size_t);
  }
  void memFree(void *ptr) {
    if (ptr != NULL) {
      char *toFree = (char*) ptr - sizeof(size_t);
      const size_t size = *(size_t *) toFree;
      MemDebuggerInitializeMem(ptr, size);
      SizeMutexAllocate();
      if (sizeMutex) sizeMutex->lock();
      memDebuggerCurrSize -= size;
      if (sizeMutex) sizeMutex->unlock();
      std::free(toFree);
    }
  }
#else
  void* memAlloc(size_t size) { return  std::malloc(size); }
  void memFree(void *ptr) { if (ptr != NULL) std::free(ptr); }
#endif /* GBE_DEBUG_MEMORY */

} /* namespace gbe */

#if GBE_DEBUG_MEMORY

namespace gbe
{
  void* alignedMalloc(size_t size, size_t align) {
    void* mem = malloc(size+align+sizeof(uintptr_t) + sizeof(void*));
    FATAL_IF (!mem && size, "memory allocation failed");
    char* aligned = (char*) mem + sizeof(uintptr_t) + sizeof(void*);
    aligned += align - ((uintptr_t)aligned & (align - 1));
    ((void**)aligned)[-1] = mem;
    ((uintptr_t*)aligned)[-2] = uintptr_t(size);
    MemDebuggerInitializeMem(aligned, size);
    SizeMutexAllocate();
    if (sizeMutex) sizeMutex->lock();
    memDebuggerCurrSize += size;
    memDebuggerMaxSize = std::max(memDebuggerCurrSize, memDebuggerMaxSize);
    if (sizeMutex) sizeMutex->unlock();
    return aligned;
  }

  void alignedFree(void* ptr) {
    if (ptr) {
      const size_t size = ((uintptr_t*)ptr)[-2];
      MemDebuggerInitializeMem(ptr, size);
      free(((void**)ptr)[-1]);
      SizeMutexAllocate();
      if (sizeMutex) sizeMutex->lock();
      memDebuggerCurrSize -= size;
      if (sizeMutex) sizeMutex->unlock();
    }
  }
} /* namespace gbe */

#else /* GBE_DEBUG_MEMORY */

////////////////////////////////////////////////////////////////////////////////
/// Linux Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__LINUX__) || defined(__GLIBC__)

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <malloc.h>
#include <iostream>

namespace gbe
{
  void* alignedMalloc(size_t size, size_t align) {
    void* ptr = memalign(align,size);
    FATAL_IF (!ptr && size, "memory allocation failed");
    MemDebuggerInitializeMem(ptr, size);
    return ptr;
  }

  void alignedFree(void *ptr) { if (ptr) std::free(ptr); }
} /* namespace gbe */

#else
#error "Unsupported platform"
#endif /* __LINUX__ */
#endif

////////////////////////////////////////////////////////////////////////////////
// Linear allocator
////////////////////////////////////////////////////////////////////////////////

namespace gbe
{
  LinearAllocator::Segment::Segment(size_t size) :
    size(size), offset(0u), data(alignedMalloc(size, CACHE_LINE)), next(NULL){}

  LinearAllocator::Segment::~Segment(void) {
    alignedFree(data);
    if (this->next) GBE_DELETE(this->next);
  }

  LinearAllocator::LinearAllocator(size_t minSize, size_t maxSize) :
    maxSize(std::max(maxSize, size_t(CACHE_LINE)))
  {
    this->curr = GBE_NEW(LinearAllocator::Segment, std::max(minSize, size_t(1)));
  }

  LinearAllocator::~LinearAllocator(void) {
    if (this->curr) GBE_DELETE(this->curr);
  }

  void *LinearAllocator::allocate(size_t size)
  {
#if GBE_DEBUG_SPECIAL_ALLOCATOR
    if (ptr) GBE_ALIGNED_MALLOC(size, sizeof(void*));
#else
    // Try to use the current segment. This is the most likely condition here
    this->curr->offset = ALIGN(this->curr->offset, sizeof(void*));
    if (this->curr->offset + size <= this->curr->size) {
      char *ptr = (char*) curr->data + this->curr->offset;
      this->curr->offset += size;
      return (void*) ptr;
    }

    // Well not really a use case in this code base
    if (UNLIKELY(size > maxSize)) {
      // This is really bad since we do two allocations
      Segment *unfortunate = GBE_NEW(Segment, size);
      GBE_ASSERT(this->curr);
      Segment *next = this->curr->next;
      this->curr->next = unfortunate;
      unfortunate->next = next;
      return unfortunate->data;
    }

    // OK. We need a new segment
    const size_t segmentSize = std::max(size, 2*this->curr->size);
    Segment *next = GBE_NEW(Segment, segmentSize);
    next->next = curr;
    this->curr = next;
    char *ptr = (char*) curr->data;
    this->curr->offset += size;
    return ptr;
#endif
  }

} /* namespace gbe */

