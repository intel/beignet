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
#include "CL/cl.h"

/* INLINE is forceinline */
#define INLINE __attribute__((always_inline)) inline
/* For all internal functions */
#define LOCAL __attribute__((visibility("internal")))
/* Align a structure or a variable */
#define ALIGNED(X) __attribute__((aligned(X)))

/* Branch hint */
#define LIKELY(x) __builtin_expect((x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)

/* Stringify macros */
#define JOIN(X, Y) _DO_JOIN(X, Y)
#define _DO_JOIN(X, Y) _DO_JOIN2(X, Y)
#define _DO_JOIN2(X, Y) X##Y

/* Debug log output */
typedef enum DEBUGP_LEVEL {
  DL_NO_OUTPUT = 0,
  DL_ERROR,
  DL_WARNING,
  DL_INFO,
} DEBUGP_LEVEL;

#ifdef NDEBUG
#define CL_LOG_INFO(...)
#define CL_LOG_WARNING(...)
#define CL_LOG_ERROR(...)
#else
extern char *cl_log_get_str(DEBUGP_LEVEL level);
#define CL_LOG_INFO(...)                                                             \
  do {                                                                               \
    char *str = cl_log_get_str(DL_INFO);                                             \
    if (str) {                                                                       \
      fprintf(stderr, "Beignet %s, File :%s, Line: %d : ", str, __FILE__, __LINE__); \
      fprintf(stderr, __VA_ARGS__);                                                  \
      fprintf(stderr, "\n");                                                         \
    }                                                                                \
  } while (0)

#define CL_LOG_WARNING(...)                                                          \
  do {                                                                               \
    char *str = cl_log_get_str(DL_WARNING);                                          \
    if (str) {                                                                       \
      fprintf(stderr, "Beignet %s, File :%s, Line: %d : ", str, __FILE__, __LINE__); \
      fprintf(stderr, __VA_ARGS__);                                                  \
      fprintf(stderr, "\n");                                                         \
    }                                                                                \
  } while (0)

#define CL_LOG_ERROR(...)                                                            \
  do {                                                                               \
    char *str = cl_log_get_str(DL_ERROR);                                            \
    if (str) {                                                                       \
      fprintf(stderr, "Beignet %s, File :%s, Line: %d : ", str, __FILE__, __LINE__); \
      fprintf(stderr, __VA_ARGS__);                                                  \
      fprintf(stderr, "\n");                                                         \
    }                                                                                \
  } while (0)
#endif
/* Always output */
#define CL_MESSAGE(fmt, ...) printf(fmt, ##__VA_ARGS__);

/* Check compile time errors */
#define STATIC_ASSERT(value)            \
  struct JOIN(__, JOIN(__, __LINE__)) { \
    int x[(value) ? 1 : -1];            \
  }

#define MAX(x0, x1) ((x0) > (x1) ? (x0) : (x1))
#define MIN(x0, x1) ((x0) < (x1) ? (x0) : (x1))
#define ALIGN(A, B) (((A) % (B)) ? (A) + (B) - ((A) % (B)) : (A))

#define FATAL(...)                \
  do {                            \
    fprintf(stderr, "error: ");   \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n");        \
    assert(0);                    \
    exit(-1);                     \
  } while (0)

#define NOT_IMPLEMENTED FATAL("Not implemented")

/* Number of DWORDS */
#define SIZEOF32(X) (sizeof(X) / sizeof(uint32_t))

/* Memory quantity */
#define KB 1024
#define MB (KB * KB)

/* To help bitfield definitions */
#define BITFIELD_BIT(X) 1
#define BITFIELD_RANGE(X, Y) ((Y) - (X) + 1)

/* 32 bits atomic variable */
typedef volatile int atomic_t;

static INLINE int atomic_add(atomic_t *v, const int c)
{
  register int i = c;
  __asm__ __volatile__("lock ; xaddl %0, %1;"
                       : "+r"(i), "+m"(*v)
                       : "m"(*v), "r"(i));
  return i;
}
static INLINE int atomic_read(atomic_t *v)
{
  return *v;
}

static INLINE int atomic_inc(atomic_t *v) { return atomic_add(v, 1); }
static INLINE int atomic_dec(atomic_t *v) { return atomic_add(v, -1); }

/* Define one list node. */
typedef struct list_node {
  struct list_node *n;
  struct list_node *p;
} list_node;
typedef struct list_head {
  list_node head_node;
} list_head;

static inline void list_node_init(list_node *node)
{
  node->n = node;
  node->p = node;
}
static inline int list_node_out_of_list(const struct list_node *node)
{
  return node->n == node;
}
static inline void list_init(list_head *head)
{
  head->head_node.n = &head->head_node;
  head->head_node.p = &head->head_node;
}
extern void list_node_insert_before(list_node *node, list_node *the_new);
extern void list_node_insert_after(list_node *node, list_node *the_new);
static inline void list_node_del(struct list_node *node)
{
  node->n->p = node->p;
  node->p->n = node->n;
  /* And all point to self for safe. */
  node->p = node;
  node->n = node;
}
static inline void list_add(list_head *head, list_node *the_new)
{
  list_node_insert_after(&head->head_node, the_new);
}
static inline void list_add_tail(list_head *head, list_node *the_new)
{
  list_node_insert_before(&head->head_node, the_new);
}
static inline int list_empty(const struct list_head *head)
{
  return head->head_node.n == &head->head_node;
}
/* Move the content from one head to another. */
extern void list_move(struct list_head *the_old, struct list_head *the_new);
/* Merge the content of the two lists to one head. */
extern void list_merge(struct list_head *head, struct list_head *to_merge);

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE, MEMBER) __compiler_offsetof(TYPE, MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#endif
#define list_entry(ptr, type, member) ({                      \
      const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
      (type *)( (char *)__mptr - offsetof(type,member) ); })

#define list_for_each(pos, head) \
  for (pos = (head)->head_node.n; pos != &((head)->head_node); pos = pos->n)

#define list_for_each_safe(pos, ne, head)                                   \
  for (pos = (head)->head_node.n, ne = pos->n; pos != &((head)->head_node); \
       pos = ne, ne = pos->n)

extern cl_int cl_get_info_helper(const void *src, size_t src_size, void *dst,
                                 size_t dst_size, size_t *ret_size);
#endif /* __CL_UTILS_H__ */
