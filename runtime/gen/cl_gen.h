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
#ifndef __CL_GEN_H__
#define __CL_GEN_H__

#include "intel_driver.h"
#include "gen_device_pci_id.h"
#include "cl_program.h"
#include "cl_kernel.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_platform_id.h"
#include "cl_device_id.h"
#include "cl_mem.h"
#include "cl_image.h"
#include "cl_device_id.h"
#include "cl_sampler.h"
#include "cl_command_queue.h"
#include "cl_event.h"

#include <libelf.h>
#include <gelf.h>
#include <string.h>

/*************************************** Device ******************************************/
enum cl_internal_kernel_type_gen { // All internal kernel types for gen
  CL_INTERNAL_KERNEL_MIN = 0,
  CL_ENQUEUE_COPY_BUFFER_ALIGN4 = 0,
  CL_ENQUEUE_COPY_BUFFER_ALIGN16,
  CL_ENQUEUE_COPY_BUFFER_UNALIGN_SAME_OFFSET,
  CL_ENQUEUE_COPY_BUFFER_UNALIGN_DST_OFFSET,
  CL_ENQUEUE_COPY_BUFFER_UNALIGN_SRC_OFFSET,
  CL_ENQUEUE_COPY_BUFFER_RECT,
  CL_ENQUEUE_COPY_BUFFER_RECT_ALIGN4,
  CL_ENQUEUE_COPY_IMAGE_1D_TO_1D,             //copy image 1d to image 1d
  CL_ENQUEUE_COPY_IMAGE_2D_TO_2D,             //copy image 2d to image 2d
  CL_ENQUEUE_COPY_IMAGE_3D_TO_2D,             //copy image 3d to image 2d
  CL_ENQUEUE_COPY_IMAGE_2D_TO_3D,             //copy image 2d to image 3d
  CL_ENQUEUE_COPY_IMAGE_3D_TO_3D,             //copy image 3d to image 3d
  CL_ENQUEUE_COPY_IMAGE_2D_TO_2D_ARRAY,       //copy image 2d to image 2d array
  CL_ENQUEUE_COPY_IMAGE_1D_ARRAY_TO_1D_ARRAY, //copy image 1d array to image 1d array
  CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D_ARRAY, //copy image 2d array to image 2d array
  CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D,       //copy image 2d array to image 2d
  CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_3D,       //copy image 2d array to image 3d
  CL_ENQUEUE_COPY_IMAGE_3D_TO_2D_ARRAY,       //copy image 3d to image 2d array
  CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER,         //copy image 2d to buffer
  CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER_ALIGN16,
  CL_ENQUEUE_COPY_IMAGE_3D_TO_BUFFER, //copy image 3d tobuffer
  CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D, //copy buffer to image 2d
  CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D_ALIGN16,
  CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_3D, //copy buffer to image 3d
  CL_ENQUEUE_FILL_BUFFER_UNALIGN,     //fill buffer with 1 aligne pattern, pattern size=1
  CL_ENQUEUE_FILL_BUFFER_ALIGN2,      //fill buffer with 2 aligne pattern, pattern size=2
  CL_ENQUEUE_FILL_BUFFER_ALIGN4,      //fill buffer with 4 aligne pattern, pattern size=4
  CL_ENQUEUE_FILL_BUFFER_ALIGN8_8,    //fill buffer with 8 aligne pattern, pattern size=8
  CL_ENQUEUE_FILL_BUFFER_ALIGN8_16,   //fill buffer with 16 aligne pattern, pattern size=16
  CL_ENQUEUE_FILL_BUFFER_ALIGN8_32,   //fill buffer with 16 aligne pattern, pattern size=32
  CL_ENQUEUE_FILL_BUFFER_ALIGN8_64,   //fill buffer with 16 aligne pattern, pattern size=64
  CL_ENQUEUE_FILL_BUFFER_ALIGN128,    //fill buffer with 128 aligne pattern, pattern size=128
  CL_ENQUEUE_FILL_IMAGE_1D,           //fill image 1d
  CL_ENQUEUE_FILL_IMAGE_1D_ARRAY,     //fill image 1d array
  CL_ENQUEUE_FILL_IMAGE_2D,           //fill image 2d
  CL_ENQUEUE_FILL_IMAGE_2D_ARRAY,     //fill image 2d array
  CL_ENQUEUE_FILL_IMAGE_3D,           //fill image 3d
  CL_INTERNAL_KERNEL_MAX
};

typedef struct _cl_device_id_gen {
  _cl_device_id base;
  /* All programs internal used, for example clEnqueuexxx api use */
  cl_program internal_program[CL_INTERNAL_KERNEL_MAX];
  cl_kernel internal_kernels[CL_INTERNAL_KERNEL_MAX];
} _cl_device_id_gen;
typedef _cl_device_id_gen *cl_device_id_gen;

extern char *cl_internal_built_in_kernel_str_kernels;
extern char *cl_internal_built_in_kernel_str;
extern size_t cl_internal_built_in_kernel_str_size;
extern cl_device_id cl_device_get_id_gen(cl_platform_id platform);

/*********************************** Kernel *****************************************/
/* Special virtual registers for OpenCL */
typedef enum cl_gen_virt_reg {
  CL_GEN_VIRT_REG_LOCAL_ID_X = 0,
  CL_GEN_VIRT_REG_LOCAL_ID_Y,
  CL_GEN_VIRT_REG_LOCAL_ID_Z,
  CL_GEN_VIRT_REG_LOCAL_SIZE_X,
  CL_GEN_VIRT_REG_LOCAL_SIZE_Y,
  CL_GEN_VIRT_REG_LOCAL_SIZE_Z,
  CL_GEN_VIRT_REG_ENQUEUED_LOCAL_SIZE_X,
  CL_GEN_VIRT_REG_ENQUEUED_LOCAL_SIZE_Y,
  CL_GEN_VIRT_REG_ENQUEUED_LOCAL_SIZE_Z,
  CL_GEN_VIRT_REG_GLOBAL_SIZE_X,
  CL_GEN_VIRT_REG_GLOBAL_SIZE_Y,
  CL_GEN_VIRT_REG_GLOBAL_SIZE_Z,
  CL_GEN_VIRT_REG_GLOBAL_OFFSET_X,
  CL_GEN_VIRT_REG_GLOBAL_OFFSET_Y,
  CL_GEN_VIRT_REG_GLOBAL_OFFSET_Z,
  CL_GEN_VIRT_REG_GROUP_NUM_X,
  CL_GEN_VIRT_REG_GROUP_NUM_Y,
  CL_GEN_VIRT_REG_GROUP_NUM_Z,
  CL_GEN_VIRT_REG_WORK_DIM,
  CL_GEN_VIRT_REG_IMAGE_INFO,
  CL_GEN_VIRT_REG_KERNEL_ARGUMENT,
  CL_GEN_VIRT_REG_EXTRA_ARGUMENT,
  CL_GEN_VIRT_REG_BLOCK_IP,
  CL_GEN_VIRT_REG_DW_BLOCK_IP,
  CL_GEN_VIRT_REG_THREAD_NUM,
  CL_GEN_VIRT_REG_PROFILING_BUF_POINTER,
  CL_GEN_VIRT_REG_PROFILING_TIMESTAMP0,
  CL_GEN_VIRT_REG_PROFILING_TIMESTAMP1,
  CL_GEN_VIRT_REG_PROFILING_TIMESTAMP2,
  CL_GEN_VIRT_REG_PROFILING_TIMESTAMP3,
  CL_GEN_VIRT_REG_PROFILING_TIMESTAMP4,
  CL_GEN_VIRT_REG_THREAD_ID,
  CL_GEN_VIRT_REG_CONSTANT_ADDRSPACE,
  CL_GEN_VIRT_REG_STACK_SIZE,
  CL_GEN_VIRT_REG_ENQUEUE_BUF_POINTER,
  CL_GEN_VIRT_REG_LAST, // Invalid
} cl_gen_virt_reg;

typedef struct _cl_gen_virt_phy_offset {
  cl_int virt_reg;
  cl_int phy_offset;
  cl_uint size;
} _cl_gen_virt_phy_offset;
typedef _cl_gen_virt_phy_offset *cl_gen_virt_phy_offset;

typedef struct _cl_gen_image_info_offset {
  cl_int bti;
  cl_int width;
  cl_int height;
  cl_int depth;
  cl_int data_type;
  cl_int channel_order;
} _cl_gen_image_info_offset;
typedef _cl_gen_image_info_offset *cl_gen_image_info_offset;

typedef struct _cl_gen_arg_extra_info {
  cl_int arg_offset;
  cl_uint arg_align; // address align for ptr
  cl_int arg_misc;   //bti, image index
} _cl_gen_arg_extra_info;
typedef _cl_gen_arg_extra_info *cl_gen_arg_extra_info;

typedef struct _cl_kernel_gen {
  _cl_kernel_for_device kern_base;
  cl_uint local_mem_size;
  cl_uint barrier_slm_used;
  cl_uint simd_width;
  cl_uint scratch_size;
  cl_uint stack_size;
  cl_uint samper_info_num;
  cl_uint *samper_info;
  cl_uint arg_extra_info_num;
  cl_gen_arg_extra_info arg_extra_info;
  cl_uint image_info_num;
  cl_gen_image_info_offset image_info;
  cl_uint virt_reg_phy_offset_num; // The mapping between virtual reg and phy offset
  cl_gen_virt_phy_offset virt_reg_phy_offset;
  cl_uint printf_num;
  cl_int printf_bti;
  cl_uint *printf_ids;
  char **printf_strings;
} _cl_kernel_gen;
typedef _cl_kernel_gen *cl_kernel_gen;

extern size_t cl_kernel_get_max_workgroup_size_gen(cl_kernel kernel, cl_device_id device);
extern void cl_kernel_delete_gen(cl_device_id device, cl_kernel kernel);
extern cl_int cl_kernel_get_info_gen(cl_device_id device, cl_kernel kernel,
                                     cl_uint param_name, void *param_value);
extern cl_int cl_kernel_create_gen(cl_device_id device, cl_kernel kernel);
extern cl_int cl_enqueue_native_kernel(cl_event event, cl_int status);

/*********************************** Program *****************************************/
enum cl_gen_program_note_type {
  GEN_NOTE_TYPE_CL_VERSION = 1,
  GEN_NOTE_TYPE_GPU_VERSION = 2,
  GEN_NOTE_TYPE_GPU_INFO = 3,
  GEN_NOTE_TYPE_CL_INFO = 4,
  GEN_NOTE_TYPE_CL_DEVICE_ENQUEUE_INFO = 5,
  GEN_NOTE_TYPE_COMPILER_INFO = 6,
  GEN_NOTE_TYPE_CL_PRINTF = 7,
};

typedef struct _cl_program_gen_device_enqueue_info {
  cl_int index;
  char *kernel_name;
} _cl_program_gen_device_enqueue_info;
typedef _cl_program_gen_device_enqueue_info *cl_program_gen_device_enqueue_info;

typedef struct _cl_program_gen {
  _cl_program_for_device prog_base;
  Elf *elf;
  size_t sec_num;
  Elf_Scn *strtab;
  cl_int strtab_sec_index;
  Elf_Data *strtab_data;
  Elf_Scn *text;
  cl_int text_sec_index;
  Elf_Data *text_data;
  Elf_Scn *rodata;
  cl_int rodata_sec_index;
  Elf_Data *rodata_data;
  Elf_Scn *symtab;
  cl_int symtab_sec_index;
  Elf_Data *symtab_data;
  size_t symtab_entry_num;
  Elf_Scn *func_gpu_info;
  cl_int func_gpu_info_sec_index;
  Elf_Data *func_gpu_info_data;
  Elf_Scn *func_cl_info;
  cl_int func_cl_info_sec_index;
  Elf_Data *func_cl_info_data;
  Elf_Scn *ro_reloc;
  cl_int ro_reloc_index;
  Elf_Data *ro_reloc_data;
  char *global_mem_data;
  cl_uint global_mem_data_size;
  char *gpu_name;
  cl_uint gpu_version_major;
  cl_uint gpu_version_minor;
  char *compiler_name;
  cl_uint compiler_version_major;
  cl_uint compiler_version_minor;
  char *cl_version_str;
  cl_uint cl_version;
  cl_uint device_enqueue_info_num;
  cl_program_gen_device_enqueue_info device_enqueue_info;
} _cl_program_gen;
typedef _cl_program_gen *cl_program_gen;

#define GEN_ELF_RELOC_GET_SYM(PROG_GEN, RELOC_ENTRY) \
  gelf_getclass(PROG_GEN->elf) == ELFCLASS64 ? ELF64_R_SYM(RELOC_ENTRY->r_info) : ELF32_R_SYM(RELOC_ENTRY->r_info)
#define GEN_ELF_RELOC_GET_TYPE(PROG_GEN, RELOC_ENTRY) \
  gelf_getclass(PROG_GEN->elf) == ELFCLASS64 ? ELF64_R_TYPE(RELOC_ENTRY->r_info) : ELF32_R_TYPE(RELOC_ENTRY->r_info)
extern cl_int cl_program_create_gen(cl_device_id device, cl_program p);
extern void cl_program_delete_gen(cl_device_id device, cl_program p);
extern cl_int cl_program_load_binary_gen(cl_device_id device, cl_program prog);
extern cl_int cl_program_get_info_gen(cl_device_id device, cl_program program,
                                      cl_uint param_name, void *param_value);

/******************************** Command Queue *****************************************/
extern cl_int cl_command_queue_ND_range(cl_command_queue queue, cl_kernel ker, void *exec_ctx,
                                        cl_uint work_dim, size_t *global_wk_off, size_t *global_wk_sz,
                                        size_t *local_wk_sz);
extern cl_int cl_command_queue_ND_range_wrap(cl_command_queue queue, cl_kernel ker, cl_event e, cl_uint work_dim,
                                             size_t *global_wk_off, size_t *global_wk_sz, size_t *local_wk_sz);
extern cl_int cl_enqueue_handle_nd_range_gen(cl_event event, cl_int status);
extern int cl_command_queue_flush_gpgpu(void *gpgpu);
extern int cl_command_queue_finish_gpgpu(void *gpgpu);
extern void cl_enqueue_nd_range_delete_gen(cl_event event);
extern cl_int cl_command_queue_create_gen(cl_device_id device, cl_command_queue queue);
extern void cl_command_queue_delete_gen(cl_device_id device, cl_command_queue queue);
extern void cl_gen_output_printf(void *buf_addr, uint32_t buf_size, cl_uint *ids, char **fmts, uint32_t printf_num);

/************************************ Compiler ******************************************/
extern cl_int cl_compiler_load_gen(cl_device_id device);
extern cl_int cl_compiler_unload_gen(cl_device_id device);

/*************************************** Mem *******************************************/
#define CL_MEM_PINNABLE (1 << 10)

typedef struct _cl_mem_drm_bo {
  _cl_base_object base;
  drm_intel_bo *bo;      /* Data in GPU memory */
  size_t gpu_size;       /* Real size allocated */
  cl_bool host_coherent; /* Is host can access address */
  void *mapped_ptr;      /* Same as bo->virtual */
  size_t in_page_offset;
  cl_bool svm;
  cl_int drm_map_ref;
  cl_image_gen_tiling tiling; /* Supports TILE_[X,Y] (image only), Buffer always no tiling */
  size_t stride;              /* Tiling row stride */
} _cl_mem_drm_bo;
typedef _cl_mem_drm_bo *cl_mem_drm_bo;
#define CL_OBJECT_DRM_BO_MAGIC 0x5583C028339CAB10LL
#define CL_OBJECT_IS_DRM_BO(obj) ((obj &&                                                    \
                                   ((cl_base_object)obj)->magic == CL_OBJECT_DRM_BO_MAGIC && \
                                   CL_OBJECT_GET_REF(obj) >= 1))

typedef struct _cl_mem_gen {
  _cl_mem_for_device mem_base;
  cl_mem_drm_bo drm_bo;

  union {
    struct {
      cl_bool already_convert_image;
    } buffer;
    struct {
      uint32_t packet_size;
      uint32_t max_packets;
    } pipe;
    struct {
      uint32_t intel_fmt;                    /* format to provide in the surface state */
      size_t sub_offset;                     /* If create from a subbuffer */
      size_t gpu_w, gpu_h, gpu_depth;        /* May change by device, need to consider aligment */
      size_t gpu_row_pitch, gpu_slice_pitch; /* May change by device, need to consider aligment */
    } image;
  };
} _cl_mem_gen;
typedef _cl_mem_gen *cl_mem_gen;
extern cl_mem_drm_bo cl_mem_gen_create_drm_bo(dri_bufmgr *bufmgr, size_t size, size_t alignment,
                                              cl_image_gen_tiling tiling, size_t stride, void *orig_data);
extern cl_mem_drm_bo cl_mem_gen_create_drm_bo_from_hostptr(dri_bufmgr *bufmgr, cl_bool svm,
                                                           size_t size, cl_uint cacheline_size, void *host_ptr);
extern void cl_mem_gen_drm_bo_ref(cl_mem_drm_bo drm_bo);
extern void cl_mem_gen_drm_bo_delete(cl_mem_drm_bo drm_bo);
extern void *cl_mem_gen_drm_bo_map(cl_mem_drm_bo drm_bo, cl_bool unsync);
extern void cl_mem_gen_drm_bo_unmap(cl_mem_drm_bo drm_bo);
extern void cl_mem_gen_drm_bo_sync(cl_mem_drm_bo drm_bo);
extern cl_bool cl_mem_gen_drm_bo_expand(cl_mem_drm_bo drm_bo, size_t new_size, size_t alignment);
extern cl_bool cl_mem_gen_drm_bo_upload_data(cl_mem_drm_bo drm_bo, size_t offset, void *data, size_t size);
extern cl_int cl_mem_enqueue_copy_buffer_gen(cl_event event, cl_int status);
extern cl_int cl_mem_enqueue_fill_buffer_gen(cl_event event, cl_int status);
extern cl_int cl_mem_enqueue_copy_buffer_rect_gen(cl_event event, cl_int status);
extern cl_int cl_mem_allocate_gen(cl_device_id device, cl_mem mem);
extern void cl_mem_deallocate_gen(cl_device_id device, cl_mem mem);
extern cl_int cl_svm_create_gen(cl_device_id device, cl_mem svm_mem);
extern void cl_svm_delete_gen(cl_device_id device, cl_mem svm_mem);
extern cl_int cl_enqueue_map_mem_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_unmap_mem_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_read_buffer_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_write_buffer_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_write_buffer_rect_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_svm_map_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_svm_unmap_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_svm_fill_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_svm_copy_gen(cl_event event, cl_int status);

/*************************************** Image ******************************************/
extern cl_int cl_enqueue_image_fill_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_image_copy_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_copy_image_to_buffer_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_copy_buffer_to_image_gen(cl_event event, cl_int status);
extern cl_int cl_mem_allocate_image_gen(cl_device_id device, cl_mem mem);
extern cl_int cl_enqueue_handle_map_image_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_unmap_image_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_read_image_gen(cl_event event, cl_int status);
extern cl_int cl_enqueue_write_image_gen(cl_event event, cl_int status);
extern uint32_t cl_image_get_gen_format(const cl_image_format *fmt);
extern cl_int cl_image_format_support_gen(cl_device_id device, cl_mem_object_type image_type,
                                          cl_image_format *image_format);

/*********************************** Context *****************************************/
typedef struct _cl_context_gen {
  _cl_context_for_device ctx_base; /* Point to the device it belong to */
  intel_driver_t *drv;             /* Handles HW or simulator */
  uint32_t ver;                    /* Gen version */
} _cl_context_gen;
typedef _cl_context_gen *cl_context_gen;

extern cl_int cl_context_create_gen(cl_device_id device, cl_context ctx);
extern void cl_context_delete_gen(cl_device_id device, cl_context ctx);
extern cl_kernel cl_context_get_builtin_kernel_gen(cl_context ctx, cl_device_id device, cl_int index);

/*********************************** Sampler ******************************************/
#define GEN_IS_SAMPLER_ARG(v) (v & __CLK_SAMPLER_ARG_KEY_BIT)
#define GEN_SAMPLER_ARG_ID(v) ((v & __CLK_SAMPLER_ARG_MASK) >> __CLK_SAMPLER_ARG_BASE)

/************************************ Event *******************************************/
extern void cl_event_update_timestamp_gen(cl_event event, cl_int status);
extern cl_int cl_event_create_gen(cl_device_id device, cl_event event);
extern void cl_event_delete_gen(cl_device_id device, cl_event event);

/************************************ Sampler ******************************************/
extern cl_int cl_sampler_create_gen(cl_device_id device, cl_sampler sampler);
extern void cl_sampler_delete_gen(cl_device_id device, cl_sampler sampler);
#endif /* End of __CL_GEN_H__ */
