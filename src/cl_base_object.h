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

#ifndef __CL_BASE_OBJECT_H__
#define __CL_BASE_OBJECT_H__

#include "cl_utils.h"
#include "cl_khr_icd.h"
#include "CL/cl.h"
#include <pthread.h>
#include <assert.h>

/************************************************************************
  Every CL objects should have:
    ICD dispatcher: Hold the ICD function table pointer.

    Reference: To maintain its' life time. CL retain/release API will
    change its value. We will destroy the object when the count reach 0.

    Magic: Just a number to represent each CL object. We will use it
    to check whether it is the object we want.

    Mutex & Cond: Used to protect the CL objects MT safe. lock/unlock
    critical region should be short enough and should not have any block
    function call. take_ownership/release_ownership  can own the object
    for a long time. take_ownership will not hold the lock and so will
    not cause deadlock problems. we can wait on the cond to get the
    ownership.
*************************************************************************/

typedef struct _cl_base_object {
  DEFINE_ICD(dispatch);  /* Dispatch function table for icd */
  cl_ulong magic;        /* Magic number for each CL object */
  atomic_t ref;          /* Reference for each CL object */
  list_node node;        /* CL object node belong to some container */
  pthread_mutex_t mutex; /* THe mutex to protect this object MT safe */
  pthread_cond_t cond;   /* Condition to wait for getting the object */
  pthread_t owner;       /* The thread which own this object */
} _cl_base_object;

typedef struct _cl_base_object *cl_base_object;

#define CL_OBJECT_INVALID_MAGIC 0xFEFEFEFEFEFEFEFELL
#define CL_OBJECT_IS_VALID(obj) (((cl_base_object)obj)->magic != CL_OBJECT_INVALID_MAGIC)

#define CL_OBJECT_INC_REF(obj) (atomic_inc(&((cl_base_object)obj)->ref))
#define CL_OBJECT_DEC_REF(obj) (atomic_dec(&((cl_base_object)obj)->ref))
#define CL_OBJECT_GET_REF(obj) (atomic_read(&((cl_base_object)obj)->ref))

#define CL_OBJECT_LOCK(obj) (pthread_mutex_lock(&((cl_base_object)obj)->mutex))
#define CL_OBJECT_UNLOCK(obj) (pthread_mutex_unlock(&((cl_base_object)obj)->mutex))

extern void cl_object_init_base(cl_base_object obj, cl_ulong magic);
extern void cl_object_destroy_base(cl_base_object obj);
extern cl_int cl_object_take_ownership(cl_base_object obj, cl_int wait, cl_bool withlock);
extern void cl_object_release_ownership(cl_base_object obj, cl_bool withlock);
extern void cl_object_wait_on_cond(cl_base_object obj);
extern void cl_object_notify_cond(cl_base_object obj);

#define CL_OBJECT_INIT_BASE(obj, magic) (cl_object_init_base((cl_base_object)obj, magic))
#define CL_OBJECT_DESTROY_BASE(obj) (cl_object_destroy_base((cl_base_object)obj))
#define CL_OBJECT_TAKE_OWNERSHIP(obj, wait) (cl_object_take_ownership((cl_base_object)obj, wait, CL_FALSE))
#define CL_OBJECT_RELEASE_OWNERSHIP(obj) (cl_object_release_ownership((cl_base_object)obj, CL_FALSE))
#define CL_OBJECT_TAKE_OWNERSHIP_WITHLOCK(obj, wait) (cl_object_take_ownership((cl_base_object)obj, wait, CL_TRUE))
#define CL_OBJECT_RELEASE_OWNERSHIP_WITHLOCK(obj) (cl_object_release_ownership((cl_base_object)obj, CL_TRUE))
#define CL_OBJECT_WAIT_ON_COND(obj) (cl_object_wait_on_cond((cl_base_object)obj))
#define CL_OBJECT_NOTIFY_COND(obj) (cl_object_notify_cond((cl_base_object)obj))

#endif /* __CL_BASE_OBJECT_H__ */
