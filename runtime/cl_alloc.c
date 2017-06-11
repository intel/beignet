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
#include "cl_alloc.h"
#include "cl_utils.h"
#include "cl_device_id.h"
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>

#ifdef CL_ALLOC_DEBUG

static pthread_mutex_t cl_alloc_log_lock;
#define MAX_ALLOC_LOG_NUM 1024 * 1024
static unsigned int cl_alloc_log_num;

typedef struct _cl_alloc_log_item {
  void *ptr;
  size_t size;
  char *file;
  int line;
} _cl_alloc_log_item;
typedef struct _cl_alloc_log_item *cl_alloc_log_item;

#define ALLOC_LOG_BUCKET_SZ 128
static cl_alloc_log_item *cl_alloc_log_map[ALLOC_LOG_BUCKET_SZ];
static int cl_alloc_log_map_size[ALLOC_LOG_BUCKET_SZ];

LOCAL void cl_alloc_debug_init(void)
{
  static int inited = 0;
  int i;
  if (inited)
    return;

  pthread_mutex_init(&cl_alloc_log_lock, NULL);

  for (i = 0; i < ALLOC_LOG_BUCKET_SZ; i++) {
    cl_alloc_log_map_size[i] = 128;
    cl_alloc_log_map[i] = malloc(cl_alloc_log_map_size[i] * sizeof(cl_alloc_log_item));
    memset(cl_alloc_log_map[i], 0, cl_alloc_log_map_size[i] * sizeof(cl_alloc_log_item));
  }
  cl_alloc_log_num = 0;

  atexit(cl_alloc_report_unfreed);
  atexit(cl_device_gen_cleanup);
  inited = 1;
}

static void insert_alloc_log_item(void *ptr, size_t sz, char *file, int line)
{
  cl_long slot;
  int i;

  if (cl_alloc_log_num > MAX_ALLOC_LOG_NUM) {
    // To many alloc without free. We consider already leaks a lot.
    cl_alloc_report_unfreed();
    assert(0);
  }

  slot = (cl_long)ptr;
  slot = (slot >> 5) & 0x07f;
  assert(slot < ALLOC_LOG_BUCKET_SZ);

  cl_alloc_log_item it = malloc(sizeof(_cl_alloc_log_item));
  assert(it);
  it->ptr = ptr;
  it->size = sz;
  it->file = file;
  it->line = line;

  pthread_mutex_lock(&cl_alloc_log_lock);
  for (i = 0; i < cl_alloc_log_map_size[slot]; i++) {
    if (cl_alloc_log_map[slot][i] == NULL) {
      break;
    }
  }

  if (i == cl_alloc_log_map_size[slot]) {
    cl_alloc_log_map[slot] =
      realloc(cl_alloc_log_map[slot], 2 * cl_alloc_log_map_size[slot] * sizeof(cl_alloc_log_item));
    memset(cl_alloc_log_map[slot] + cl_alloc_log_map_size[slot], 0,
           cl_alloc_log_map_size[slot] * sizeof(cl_alloc_log_item));
    cl_alloc_log_map_size[slot] = cl_alloc_log_map_size[slot] * 2;
  }

  cl_alloc_log_map[slot][i] = it;
  cl_alloc_log_num++;
  pthread_mutex_unlock(&cl_alloc_log_lock);
}

static void delete_alloc_log_item(void *ptr, char *file, int line)
{
  cl_long slot;
  int i;

  slot = (cl_long)ptr;
  slot = (slot >> 5) & 0x07f;
  assert(slot < ALLOC_LOG_BUCKET_SZ);

  pthread_mutex_lock(&cl_alloc_log_lock);
  for (i = 0; i < cl_alloc_log_map_size[slot]; i++) {
    if (cl_alloc_log_map[slot][i] && cl_alloc_log_map[slot][i]->ptr == ptr) {
      break;
    }
  }

  if (i == cl_alloc_log_map_size[slot]) {
    printf("Free at file: %s, line: %d, We can not find the malloc log for this ptr:%p, fatal\n",
           file, line, ptr);
    assert(0);
  }

  free(cl_alloc_log_map[slot][i]);
  cl_alloc_log_map[slot][i] = NULL;

  cl_alloc_log_num--;
  pthread_mutex_unlock(&cl_alloc_log_lock);
}

LOCAL void cl_register_alloc_ptr(void *ptr, size_t sz, char *file, int line)
{
  assert(ptr);
  insert_alloc_log_item(ptr, sz, file, line);
}

LOCAL void *cl_malloc(size_t sz, char *file, int line)
{
  void *p = malloc(sz);
  assert(p);
  insert_alloc_log_item(p, sz, file, line);
  return p;
}

LOCAL void *cl_memalign(size_t align, size_t sz, char *file, int line)
{
  void *p = NULL;
  p = memalign(align, sz);
  assert(p);
  insert_alloc_log_item(p, ((sz + align - 1) / align) * align, file, line);
  return p;
}

LOCAL void *cl_calloc(size_t n, size_t elem_size, char *file, int line)
{
  void *p = NULL;
  p = calloc(n, elem_size);
  assert(p);
  insert_alloc_log_item(p, n * elem_size, file, line);
  return p;
}

LOCAL void *cl_realloc(void *ptr, size_t sz, char *file, int line)
{
  void *p = NULL;

  if (ptr != NULL) {
    delete_alloc_log_item(ptr, file, line);
  }

  p = realloc(ptr, sz);
  assert(p);
  insert_alloc_log_item(p, sz, file, line);
  return p;
}

LOCAL void cl_free(void *ptr, char *file, int line)
{
  if (ptr == NULL)
    return;

  delete_alloc_log_item(ptr, file, line);
  free(ptr);
}

void cl_alloc_report_unfreed(void)
{
  int i, slot, num;
  pthread_mutex_lock(&cl_alloc_log_lock);
  if (cl_alloc_log_num == 0) {
    pthread_mutex_unlock(&cl_alloc_log_lock);
    return;
  }

  printf("-------------------------------------------------------------------\n");
  num = 0;
  for (slot = 0; slot < ALLOC_LOG_BUCKET_SZ; slot++) {
    for (i = 0; i < cl_alloc_log_map_size[slot]; i++) {
      if (cl_alloc_log_map[slot][i]) {
        printf("Leak point at file:%s, line: %d, ptr is %p, alloc size is %ld\n",
               cl_alloc_log_map[slot][i]->file, cl_alloc_log_map[slot][i]->line,
               cl_alloc_log_map[slot][i]->ptr, cl_alloc_log_map[slot][i]->size);
        num++;
      }
    }
  }
  printf("-------------------------------------------------------------------\n");
  assert(num == cl_alloc_log_num);
  pthread_mutex_unlock(&cl_alloc_log_lock);
}

#endif
