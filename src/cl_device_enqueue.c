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
 * Author: Rong Yang<rong.r.yang@intel.com>
 */
#include "cl_device_enqueue.h"
#include "cl_mem.h"
#include "cl_utils.h"
#include "cl_context.h"
#include "cl_program.h"
#include "cl_alloc.h"
#include "cl_kernel.h"
#include "cl_command_queue.h"
#include "cl_event.h"

LOCAL cl_int
cl_device_enqueue_fix_offset(cl_kernel ker) {
  uint32_t i;
  void *ptr;
  cl_mem mem;
  enum gbe_arg_type arg_type; /* kind of argument */
  for (i = 0; i < ker->arg_n; ++i) {
    arg_type = interp_kernel_get_arg_type(ker->opaque, i);
    //HOW about image
    if (!(arg_type == GBE_ARG_GLOBAL_PTR || arg_type == GBE_ARG_CONSTANT_PTR) || !ker->args[i].mem)
      continue;

    if(!ker->args[i].is_svm) {
      mem = ker->args[i].mem;
      ptr = cl_mem_map(mem, 0);
      cl_buffer_set_softpin_offset(mem->bo, (size_t)ptr);
      cl_buffer_set_bo_use_full_range(mem->bo, 1);
      cl_buffer_disable_reuse(mem->bo);
      mem->host_ptr = ptr;
      cl_mem_unmap(mem);
      ker->device_enqueue_infos[ker->device_enqueue_info_n++] = ptr;
    } else {
      ker->device_enqueue_infos[ker->device_enqueue_info_n++] = ker->args[i].mem->host_ptr;
    }
  }
  return 0;
}

LOCAL cl_int
cl_device_enqueue_bind_buffer(cl_gpgpu gpgpu, cl_kernel ker, uint32_t *max_bti, cl_gpgpu_kernel *kernel)
{
  int32_t value = GBE_CURBE_ENQUEUE_BUF_POINTER;
  int32_t offset = interp_kernel_get_curbe_offset(ker->opaque, value, 0);
  size_t buf_size = 32 * 1024 * 1024;  //fix 32M
  cl_mem mem;

  if(offset > 0) {
    if(ker->useDeviceEnqueue == false) {
      if(ker->device_enqueue_ptr == NULL)
        ker->device_enqueue_ptr = cl_mem_svm_allocate(ker->program->ctx, 0, buf_size, 0);
      if(ker->device_enqueue_infos == NULL)
        ker->device_enqueue_infos = cl_calloc(ker->arg_n, sizeof(void *));
      ker->device_enqueue_info_n = 0;
      ker->useDeviceEnqueue = CL_TRUE;
      cl_device_enqueue_fix_offset(ker);
      cl_kernel_add_ref(ker);
    }

    mem = cl_context_get_svm_from_ptr(ker->program->ctx, ker->device_enqueue_ptr);
    assert(mem);
    cl_gpgpu_bind_buf(gpgpu, mem->bo, offset, 0, buf_size, *max_bti);

    cl_gpgpu_set_kernel(gpgpu, ker);
  }
  return 0;
}

typedef struct ndrange_info_t {
  int type;
  int global_work_size[3];
  int local_work_size[3];
  int global_work_offset[3];
} ndrange_info_t;

typedef struct Block_literal {
  void *isa; // initialized to &_NSConcreteStackBlock or &_NSConcreteGlobalBlock
  int flags;
  int reserved;
  int index;
  struct Block_descriptor_1 {
    unsigned long int slm_size;         // NULL
    unsigned long int size;         // sizeof(struct Block_literal_1)
    // optional helper functions
    void *copy_helper;     // IFF (1<<25)
    void *dispose_helper;             // IFF (1<<25)
    // required ABI.2010.3.16
    const char *signature;                         // IFF (1<<30)
  } *descriptor;
  // imported variables
} Block_literal;

LOCAL cl_int
cl_device_enqueue_parse_result(cl_command_queue queue, cl_gpgpu gpgpu)
{
  cl_mem mem;
  int size, type, dim, i;
  const char * kernel_name;
  cl_kernel child_ker;
  cl_event evt = NULL;

  cl_kernel ker = cl_gpgpu_get_kernel(gpgpu);
  if(ker == NULL || ker->useDeviceEnqueue == CL_FALSE)
    return 0;

  void *buf = cl_gpgpu_ref_batch_buf(gpgpu);
  //wait the gpgpu's batch buf finish, the gpgpu in queue may be not
  //same as the param gpgpu, for example when flush event.
  cl_gpgpu_sync(buf);
  cl_gpgpu_unref_batch_buf(buf);

  mem = cl_context_get_svm_from_ptr(ker->program->ctx, ker->device_enqueue_ptr);
  if(mem == NULL) return -1;
  char *ptr = (char *)cl_mem_map(mem, 0);

  size =  *(int *)ptr;
  ptr += 4;
  while(size > 0) {
    size_t fixed_global_off[] = {0,0,0};
    size_t fixed_global_sz[] = {1,1,1};
    size_t fixed_local_sz[] = {1,1,1};
    ndrange_info_t* ndrange_info = (ndrange_info_t *)ptr;
    size -= sizeof(ndrange_info_t);
    ptr += sizeof(ndrange_info_t);

    Block_literal *block = (Block_literal *)ptr;
    size -=  block->descriptor->size;
    ptr += block->descriptor->size;

    type = ndrange_info->type;
    dim = (type & 0xf0) >> 4;
    type = type & 0xf;
    assert(dim <= 2);
    for(i = 0; i <= dim; i++) {
      fixed_global_sz[i] = ndrange_info->global_work_size[i];
      if(type > 1)
        fixed_local_sz[i] = ndrange_info->local_work_size[i];
      if(type > 2)
        fixed_global_off[i] = ndrange_info->global_work_offset[i];
    }

    int *slm_sizes = (int *)ptr;
    int slm_size = block->descriptor->slm_size;
    size -= slm_size;
    ptr += slm_size;

    kernel_name = interp_program_get_device_enqueue_kernel_name(ker->program->opaque, block->index);
    child_ker = cl_program_create_kernel(ker->program, kernel_name, NULL);
    assert(child_ker);
    cl_kernel_set_arg_svm_pointer(child_ker, 0, block);
    int index = 1;
    for(i=0; i<slm_size/sizeof(int); i++, index++) {
      cl_kernel_set_arg(child_ker, index, slm_sizes[i], NULL);
    }
    cl_kernel_set_exec_info(child_ker, ker->device_enqueue_info_n * sizeof(void *),
                            ker->device_enqueue_infos);

    if (evt != NULL) {
      clReleaseEvent(evt);
      evt = NULL;
    }
    clEnqueueNDRangeKernel(queue, child_ker, dim + 1, fixed_global_off,
                           fixed_global_sz, fixed_local_sz, 0, NULL, &evt);
    cl_command_queue_flush_gpgpu(gpgpu);
    cl_kernel_delete(child_ker);
  }

  if (evt != NULL) {
    //Can't call clWaitForEvents here, it may cause dead lock.
    //If evt->exec_data.gpgpu is NULL, evt has finished.
    if (evt->exec_data.gpgpu) {
      buf = cl_gpgpu_ref_batch_buf(evt->exec_data.gpgpu);
      //wait the gpgpu's batch buf finish, the gpgpu in queue may be not
      //same as the param gpgpu, for example when flush event.
      cl_gpgpu_sync(buf);
      cl_gpgpu_unref_batch_buf(buf);
    }
    clReleaseEvent(evt);
    evt = NULL;
  }
  cl_mem_unmap_auto(mem);
  cl_kernel_delete(ker);
  return 0;
}
