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

#ifndef __CL_UTILS_H__
#define __CL_UTILS_H__

/* INLINE is forceinline */
#define INLINE __attribute__((always_inline)) inline

/* Branch hint */
#define LIKELY(x)       __builtin_expect((x),1)
#define UNLIKELY(x)     __builtin_expect((x),0)

/* Stringify macros */
#define JOIN(X, Y) _DO_JOIN(X, Y)
#define _DO_JOIN(X, Y) _DO_JOIN2(X, Y)
#define _DO_JOIN2(X, Y) X##Y

/* Check compile time errors */
#define STATIC_ASSERT(value)                                        \
struct JOIN(__,JOIN(__,__LINE__)) {                                 \
  int x[(value) ? 1 : -1];                                          \
}

/* Throw errors */
#ifdef NDEBUG
  #define ERR(ERROR, ...)                                             \
  do {                                                                \
    err = ERROR;                                                      \
    goto error;                                                       \
  } while (0)
#else
  #define ERR(ERROR, ...)                                             \
  do {                                                                \
    fprintf(stderr, "error in %s line %i\n", __FILE__, __LINE__);     \
    fprintf(stderr, __VA_ARGS__);                                     \
    fprintf(stderr, "\n");                                            \
    err = ERROR;                                                      \
    goto error;                                                       \
  } while (0)
#endif

#define DO_ALLOC_ERR                                                \
do {                                                                \
  ERR(CL_OUT_OF_HOST_MEMORY, "Out of memory");                      \
} while (0)

#define ERR_IF(COND, ERROR, ...)                                    \
do {                                                                \
  if (UNLIKELY(COND)) ERR (ERROR, __VA_ARGS__);                     \
} while (0)

#define INVALID_VALUE_IF(COND)                                      \
do {                                                                \
  ERR_IF(COND, CL_INVALID_VALUE, "Invalid value");                  \
} while (0)

#define INVALID_DEVICE_IF(COND)                                     \
do {                                                                \
  ERR_IF(COND, CL_INVALID_DEVICE, "Invalid device");                \
} while (0)

#define MAX(x0, x1) ((x0) > (x1) ? (x0) : (x1))
#define MIN(x0, x1) ((x0) < (x1) ? (x0) : (x1))
#define ALIGN(A, B) (((A) % (B)) ? (A) + (B) - ((A) % (B)) : (A))

#define DO_ALLOC_ERROR                                      \
do {                                                        \
  err = CL_OUT_OF_HOST_MEMORY;                              \
  goto error;                                               \
} while (0)

#define FATAL(...)                                          \
do {                                                        \
  fprintf(stderr, "error: ");                               \
  fprintf(stderr, __VA_ARGS__);                             \
  fprintf(stderr, "\n");                                    \
  assert(0);                                                \
  exit(-1);                                                 \
} while (0)

#define FATAL_IF(COND, ...)                                 \
do {                                                        \
  if (UNLIKELY(COND)) FATAL(__VA_ARGS__);                   \
} while (0)

#define NOT_IMPLEMENTED FATAL ("Not implemented")

#define CHECK_CONTEXT(CTX)                                  \
do {                                                        \
  if (UNLIKELY(CTX == NULL)) {                              \
    err = CL_INVALID_CONTEXT;                               \
    goto error;                                             \
  }                                                         \
  if (UNLIKELY(CTX->magic != CL_MAGIC_CONTEXT_HEADER)) {    \
    err = CL_INVALID_CONTEXT;                               \
    goto error;                                             \
  }                                                         \
} while (0)

#define CHECK_QUEUE(QUEUE)                                  \
do {                                                        \
  if (UNLIKELY(QUEUE == NULL)) {                            \
    err = CL_INVALID_COMMAND_QUEUE;                         \
    goto error;                                             \
  }                                                         \
  if (UNLIKELY(QUEUE->magic != CL_MAGIC_QUEUE_HEADER)) {    \
    err = CL_INVALID_COMMAND_QUEUE;                         \
    goto error;                                             \
  }                                                         \
} while (0)

#define CHECK_MEM(MEM)                                      \
do {                                                        \
  if (UNLIKELY(MEM == NULL)) {                              \
    err = CL_INVALID_MEM_OBJECT;                            \
    goto error;                                             \
  }                                                         \
  if (UNLIKELY(MEM->magic != CL_MAGIC_MEM_HEADER)) {        \
    err = CL_INVALID_MEM_OBJECT;                            \
    goto error;                                             \
  }                                                         \
} while (0)

#define CHECK_IMAGE(MEM, IMAGE)                             \
CHECK_MEM(MEM);                                             \
do {                                                        \
  if (UNLIKELY(!IS_IMAGE(MEM))) {                           \
    err = CL_INVALID_MEM_OBJECT;                            \
    goto error;                                             \
  }                                                         \
} while (0);                                                \
struct _cl_mem_image *IMAGE;                                \
IMAGE = cl_mem_image(MEM);                                  \

#define FIXUP_IMAGE_REGION(IMAGE, PREGION, REGION)          \
const size_t *REGION;                                       \
size_t REGION ##_REC[3];                                    \
do {                                                        \
  if (IMAGE->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {   \
    REGION ##_REC[0] = PREGION[0];                          \
    REGION ##_REC[1] = 1;                                   \
    REGION ##_REC[2] = PREGION[1];                          \
    REGION = REGION ##_REC;                                 \
  } else {                                                  \
    REGION = PREGION;                                       \
  }                                                         \
} while(0)

#define FIXUP_IMAGE_ORIGIN(IMAGE, PREGION, REGION)          \
const size_t *REGION;                                       \
size_t REGION ##_REC[3];                                    \
do {                                                        \
  if (IMAGE->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {   \
    REGION ##_REC[0] = PREGION[0];                          \
    REGION ##_REC[1] = 0;                                   \
    REGION ##_REC[2] = PREGION[1];                          \
    REGION = REGION ##_REC;                                 \
  } else {                                                  \
    REGION = PREGION;                                       \
  }                                                         \
} while(0)


#define CHECK_EVENT(EVENT)                                    \
  do {                                                        \
    if (UNLIKELY(EVENT == NULL)) {                            \
      err = CL_INVALID_EVENT;                                 \
      goto error;                                             \
    }                                                         \
    if (UNLIKELY(EVENT->magic != CL_MAGIC_EVENT_HEADER)) {    \
      err = CL_INVALID_EVENT;                                 \
      goto error;                                             \
    }                                                         \
  } while (0)

#define CHECK_SAMPLER(SAMPLER)                              \
do {                                                        \
  if (UNLIKELY(SAMPLER == NULL)) {                          \
    err = CL_INVALID_SAMPLER;                               \
    goto error;                                             \
  }                                                         \
  if (UNLIKELY(SAMPLER->magic != CL_MAGIC_SAMPLER_HEADER)) {\
    err = CL_INVALID_SAMPLER;                               \
    goto error;                                             \
  }                                                         \
} while (0)

#define CHECK_KERNEL(KERNEL)                                \
do {                                                        \
  if (UNLIKELY(KERNEL == NULL)) {                           \
    err = CL_INVALID_KERNEL;                                \
    goto error;                                             \
  }                                                         \
  if (UNLIKELY(KERNEL->magic != CL_MAGIC_KERNEL_HEADER)) {  \
    err = CL_INVALID_KERNEL;                                \
    goto error;                                             \
  }                                                         \
} while (0)

#define CHECK_PROGRAM(PROGRAM)                              \
do {                                                        \
  if (UNLIKELY(PROGRAM == NULL)) {                          \
    err = CL_INVALID_PROGRAM;                               \
    goto error;                                             \
  }                                                         \
  if (UNLIKELY(PROGRAM->magic != CL_MAGIC_PROGRAM_HEADER)) {\
    err = CL_INVALID_PROGRAM;                               \
    goto error;                                             \
  }                                                         \
} while (0)

#define ELEMENTS(x) (sizeof(x)/sizeof(*(x)))
#define CALLOC_STRUCT(T) (struct T*) cl_calloc(1, sizeof(struct T))
#define CALLOC(T) (T*) cl_calloc(1, sizeof(T))
#define CALLOC_ARRAY(T, N) (T*) cl_calloc(N, sizeof(T))
#define MEMZERO(x) do { memset((x),0,sizeof(*(x))); } while (0)

/* Run some code and catch errors */
#define TRY(fn,...)                                     \
do {                                                    \
  if (UNLIKELY((err = fn(__VA_ARGS__)) != CL_SUCCESS))  \
    goto error;                                         \
} while (0)

#define TRY_NO_ERR(fn,...)                              \
do {                                                    \
  if (UNLIKELY(fn(__VA_ARGS__) != CL_SUCCESS))          \
    goto error;                                         \
} while (0)

#define TRY_ALLOC(dst, EXPR)                            \
do {                                                    \
  if (UNLIKELY((dst = EXPR) == NULL))                   \
    DO_ALLOC_ERROR;                                     \
} while (0)

#define TRY_ALLOC_NO_ERR(dst, EXPR)                     \
do {                                                    \
  if (UNLIKELY((dst = EXPR) == NULL))                   \
    goto error;                                         \
} while (0)

#define TRY_ALLOC_NO_RET(EXPR)                          \
do {                                                    \
  if (UNLIKELY((EXPR) == NULL))                         \
    DO_ALLOC_ERROR;                                     \
} while (0)

/* Break Point Definitions */
#if !defined(NDEBUG)

#define BREAK                                           \
do {                                                    \
  __asm__("int3");                                      \
} while(0)

#define BREAK_IF(value)                                 \
do {                                                    \
  if (UNLIKELY(!(value))) BREAKPOINT();                 \
} while(0)

#else
#define BREAKPOINT() do { } while(0)
#define ASSERT(value) do { } while(0)
#endif

/* For all internal functions */
#define LOCAL __attribute__ ((visibility ("internal")))

/* Align a structure or a variable */
#define ALIGNED(X) __attribute__ ((aligned (X)))

/* Number of DWORDS */
#define SIZEOF32(X) (sizeof(X) / sizeof(uint32_t))

/* Memory quantity */
#define KB 1024
#define MB (KB*KB)

/* To help bitfield definitions */
#define BITFIELD_BIT(X) 1
#define BITFIELD_RANGE(X,Y) ((Y) - (X) + 1)

/* 32 bits atomic variable */
typedef volatile int atomic_t;

static INLINE int atomic_add(atomic_t *v, const int c) {
  register int i = c;
  __asm__ __volatile__("lock ; xaddl %0, %1;"
      : "+r"(i), "+m"(*v)
      : "m"(*v), "r"(i));
  return i;
}

static INLINE int atomic_inc(atomic_t *v) { return atomic_add(v, 1); }
static INLINE int atomic_dec(atomic_t *v) { return atomic_add(v, -1); }

#endif /* __CL_UTILS_H__ */

