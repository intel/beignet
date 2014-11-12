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
 *         Alexei Soupikov <alexei.soupikov@intel.com>
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <errno.h>

#include "intel/intel_gpgpu.h"
#include "intel/intel_defines.h"
#include "intel/intel_structs.h"
#include "program.h" // for BTI_RESERVED_NUM

#include "cl_alloc.h"
#include "cl_utils.h"
#include "cl_sampler.h"

#ifndef CL_VERSION_1_2
#define CL_MEM_OBJECT_IMAGE1D                       0x10F4
#define CL_MEM_OBJECT_IMAGE1D_ARRAY                 0x10F5
#define CL_MEM_OBJECT_IMAGE1D_BUFFER                0x10F6
#define CL_MEM_OBJECT_IMAGE2D_ARRAY                 0x10F3
#endif

#define GEN_CMD_MEDIA_OBJECT  (0x71000000)
#define MO_TS_BIT             (1 << 24)
#define MO_RETAIN_BIT         (1 << 28)
#define SAMPLER_STATE_SIZE    (16)

#define TIMESTAMP_ADDR        0x2358

/* Stores both binding tables and surface states */
typedef struct surface_heap {
  uint32_t binding_table[256];
  char surface[256*sizeof(gen_surface_state_t)];
} surface_heap_t;

typedef struct intel_event {
  drm_intel_bo *buffer;
  drm_intel_bo *ts_buf;
  int status;
} intel_event_t;

#define MAX_IF_DESC    32

typedef struct intel_gpgpu intel_gpgpu_t;

typedef void (intel_gpgpu_set_L3_t)(intel_gpgpu_t *gpgpu, uint32_t use_slm);
intel_gpgpu_set_L3_t *intel_gpgpu_set_L3 = NULL;

typedef uint32_t (intel_gpgpu_get_scratch_index_t)(uint32_t size);
intel_gpgpu_get_scratch_index_t *intel_gpgpu_get_scratch_index = NULL;

typedef void (intel_gpgpu_post_action_t)(intel_gpgpu_t *gpgpu, int32_t flush_mode);
intel_gpgpu_post_action_t *intel_gpgpu_post_action = NULL;

typedef uint64_t (intel_gpgpu_read_ts_reg_t)(drm_intel_bufmgr *bufmgr);
intel_gpgpu_read_ts_reg_t *intel_gpgpu_read_ts_reg = NULL;


typedef void (intel_gpgpu_set_base_address_t)(intel_gpgpu_t *gpgpu);
intel_gpgpu_set_base_address_t *intel_gpgpu_set_base_address = NULL;

typedef void (intel_gpgpu_setup_bti_t)(intel_gpgpu_t *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                                       uint32_t size, unsigned char index, uint32_t format);
intel_gpgpu_setup_bti_t *intel_gpgpu_setup_bti = NULL;


typedef void (intel_gpgpu_load_vfe_state_t)(intel_gpgpu_t *gpgpu);
intel_gpgpu_load_vfe_state_t *intel_gpgpu_load_vfe_state = NULL;

typedef void (intel_gpgpu_build_idrt_t)(intel_gpgpu_t *gpgpu, cl_gpgpu_kernel *kernel);
intel_gpgpu_build_idrt_t *intel_gpgpu_build_idrt = NULL;


typedef void (intel_gpgpu_load_curbe_buffer_t)(intel_gpgpu_t *gpgpu);
intel_gpgpu_load_curbe_buffer_t *intel_gpgpu_load_curbe_buffer = NULL;


typedef void (intel_gpgpu_load_idrt_t)(intel_gpgpu_t *gpgpu);
intel_gpgpu_load_idrt_t *intel_gpgpu_load_idrt = NULL;

static void
intel_gpgpu_sync(void *buf)
{
  if (buf)
    drm_intel_bo_wait_rendering((drm_intel_bo *)buf);
}

static void *intel_gpgpu_ref_batch_buf(intel_gpgpu_t *gpgpu)
{
  if (gpgpu->batch->last_bo)
    drm_intel_bo_reference(gpgpu->batch->last_bo);

  return gpgpu->batch->last_bo;
}

static void intel_gpgpu_unref_batch_buf(void *buf)
{
  if (buf)
    drm_intel_bo_unreference((drm_intel_bo *)buf);
}

static void
intel_gpgpu_delete_finished(intel_gpgpu_t *gpgpu)
{
  if (gpgpu == NULL)
    return;
  if(gpgpu->time_stamp_b.bo)
    drm_intel_bo_unreference(gpgpu->time_stamp_b.bo);
  if(gpgpu->printf_b.bo)
    drm_intel_bo_unreference(gpgpu->printf_b.bo);
  if(gpgpu->printf_b.ibo)
    drm_intel_bo_unreference(gpgpu->printf_b.ibo);
  if (gpgpu->aux_buf.bo)
    drm_intel_bo_unreference(gpgpu->aux_buf.bo);
  if (gpgpu->perf_b.bo)
    drm_intel_bo_unreference(gpgpu->perf_b.bo);
  if (gpgpu->stack_b.bo)
    drm_intel_bo_unreference(gpgpu->stack_b.bo);
  if (gpgpu->scratch_b.bo)
    drm_intel_bo_unreference(gpgpu->scratch_b.bo);

  if(gpgpu->constant_b.bo)
    drm_intel_bo_unreference(gpgpu->constant_b.bo);

  intel_batchbuffer_delete(gpgpu->batch);
  cl_free(gpgpu);
}

/* Destroy the all intel_gpgpu, no matter finish or not, when driver destroy */
void intel_gpgpu_delete_all(intel_driver_t *drv)
{
  struct intel_gpgpu_node *p;
  if(drv->gpgpu_list == NULL)
    return;

  PPTHREAD_MUTEX_LOCK(drv);
  while(drv->gpgpu_list) {
    p = drv->gpgpu_list;
    drv->gpgpu_list = p->next;
    intel_gpgpu_delete_finished(p->gpgpu);
    cl_free(p);
  }
  PPTHREAD_MUTEX_UNLOCK(drv);
}

static void
intel_gpgpu_delete(intel_gpgpu_t *gpgpu)
{
  intel_driver_t *drv = gpgpu->drv;
  struct intel_gpgpu_node *p, *node;

  PPTHREAD_MUTEX_LOCK(drv);
  p = drv->gpgpu_list;
  if(p) {
    node = p->next;
    while(node) {
      if(node->gpgpu->batch && node->gpgpu->batch->buffer &&
         !drm_intel_bo_busy(node->gpgpu->batch->buffer)) {
        p->next = node->next;
        intel_gpgpu_delete_finished(node->gpgpu);
        cl_free(node);
        node = p->next;
      } else {
        p = node;
        node = node->next;
      }
    }
    node = drv->gpgpu_list;
    if(node->gpgpu->batch && node->gpgpu->batch->buffer &&
       !drm_intel_bo_busy(node->gpgpu->batch->buffer)) {
      drv->gpgpu_list = drv->gpgpu_list->next;
      intel_gpgpu_delete_finished(node->gpgpu);
      cl_free(node);
      node = p->next;
    }
  }
  if (gpgpu == NULL)
    return;

  if(gpgpu->batch && gpgpu->batch->buffer &&
     !drm_intel_bo_busy(gpgpu->batch->buffer)) {
    TRY_ALLOC_NO_ERR (node, CALLOC(struct intel_gpgpu_node));
    node->gpgpu = gpgpu;
    node->next = NULL;
    p = drv->gpgpu_list;
    if(p == NULL)
      drv->gpgpu_list= node;
    else {
      while(p->next)
        p = p->next;
      p->next = node;
    }
  } else
    intel_gpgpu_delete_finished(gpgpu);

error:
  PPTHREAD_MUTEX_UNLOCK(drv);
}

static intel_gpgpu_t*
intel_gpgpu_new(intel_driver_t *drv)
{
  intel_gpgpu_t *state = NULL;

  TRY_ALLOC_NO_ERR (state, CALLOC(intel_gpgpu_t));
  state->drv = drv;
  state->batch = intel_batchbuffer_new(state->drv);
  assert(state->batch);

exit:
  return state;
error:
  intel_gpgpu_delete(state);
  state = NULL;
  goto exit;
}

static void
intel_gpgpu_select_pipeline(intel_gpgpu_t *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 1);
  OUT_BATCH(gpgpu->batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_GPGPU);
  ADVANCE_BATCH(gpgpu->batch);
}

static uint32_t
intel_gpgpu_get_cache_ctrl_gen7()
{
  return cc_llc_l3;
}

static uint32_t
intel_gpgpu_get_cache_ctrl_gen75()
{
  return llccc_ec | l3cc_ec;
}
static uint32_t
intel_gpgpu_get_cache_ctrl_gen8()
{
  return tcc_llc_ec_l3 | mtllc_wb;
}

static void
intel_gpgpu_set_base_address_gen7(intel_gpgpu_t *gpgpu)
{
  const uint32_t def_cc = cl_gpgpu_get_cache_ctrl(); /* default Cache Control value */
  BEGIN_BATCH(gpgpu->batch, 10);
  OUT_BATCH(gpgpu->batch, CMD_STATE_BASE_ADDRESS | 8);
  /* 0, Gen State Mem Obj CC, Stateless Mem Obj CC, Stateless Access Write Back */
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 8) | (def_cc << 4) | (0 << 3)| BASE_ADDRESS_MODIFY);    /* General State Base Addr   */
  /* 0, State Mem Obj CC */
  /* We use a state base address for the surface heap since IVB clamp the
   * binding table pointer at 11 bits. So, we cannot use pointers directly while
   * using the surface heap
   */
  assert(gpgpu->aux_offset.surface_heap_offset % 4096 == 0);
  OUT_RELOC(gpgpu->batch, gpgpu->aux_buf.bo,
            I915_GEM_DOMAIN_INSTRUCTION,
            I915_GEM_DOMAIN_INSTRUCTION,
            gpgpu->aux_offset.surface_heap_offset + (0 | (def_cc << 8) | (def_cc << 4) | (0 << 3)| BASE_ADDRESS_MODIFY));

  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Dynamic State Base Addr */

  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Indirect Obj Base Addr */
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Instruction Base Addr  */
  /* If we output an AUB file, we limit the total size to 64MB */
#if USE_FULSIM
  OUT_BATCH(gpgpu->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* General State Access Upper Bound */
  OUT_BATCH(gpgpu->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Dynamic State Access Upper Bound */
  OUT_BATCH(gpgpu->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Indirect Obj Access Upper Bound */
  OUT_BATCH(gpgpu->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Instruction Access Upper Bound */
#else
  OUT_BATCH(gpgpu->batch, 0 | BASE_ADDRESS_MODIFY);
  /* According to mesa i965 driver code, we must set the dynamic state access upper bound
   * to a valid bound value, otherwise, the border color pointer may be rejected and you
   * may get incorrect border color. This is a known hardware bug. */
  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0 | BASE_ADDRESS_MODIFY);
#endif /* USE_FULSIM */
  ADVANCE_BATCH(gpgpu->batch);
}

static void
intel_gpgpu_set_base_address_gen8(intel_gpgpu_t *gpgpu)
{
    const uint32_t def_cc = cl_gpgpu_get_cache_ctrl(); /* default Cache Control value */
    BEGIN_BATCH(gpgpu->batch, 16);
    OUT_BATCH(gpgpu->batch, CMD_STATE_BASE_ADDRESS | 14);
    /* 0, Gen State Mem Obj CC, Stateless Mem Obj CC, Stateless Access Write Back */
    OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | (0 << 1)| BASE_ADDRESS_MODIFY);    /* General State Base Addr   */
    OUT_BATCH(gpgpu->batch, 0);
    OUT_BATCH(gpgpu->batch, 0 | (def_cc << 16));
    /* 0, State Mem Obj CC */
    /* We use a state base address for the surface heap since IVB clamp the
     * binding table pointer at 11 bits. So, we cannot use pointers directly while
     * using the surface heap
     */
    assert(gpgpu->aux_offset.surface_heap_offset % 4096 == 0);
    OUT_RELOC(gpgpu->batch, gpgpu->aux_buf.bo,
              I915_GEM_DOMAIN_SAMPLER,
              I915_GEM_DOMAIN_SAMPLER,
              gpgpu->aux_offset.surface_heap_offset + (0 | (def_cc << 4) | (0 << 1)| BASE_ADDRESS_MODIFY));
    OUT_BATCH(gpgpu->batch, 0);
    OUT_RELOC(gpgpu->batch, gpgpu->aux_buf.bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              (0 | (def_cc << 4) | (0 << 1)| BASE_ADDRESS_MODIFY)); /* Dynamic State Base Addr */
    OUT_BATCH(gpgpu->batch, 0);
    OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | BASE_ADDRESS_MODIFY); /* Indirect Obj Base Addr */
    OUT_BATCH(gpgpu->batch, 0);
    //OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | BASE_ADDRESS_MODIFY); /* Instruction Base Addr  */
    OUT_RELOC(gpgpu->batch, (drm_intel_bo *)gpgpu->ker->bo,
              I915_GEM_DOMAIN_INSTRUCTION,
              I915_GEM_DOMAIN_INSTRUCTION,
              0 + (0 | (def_cc << 4) | (0 << 1)| BASE_ADDRESS_MODIFY));
    OUT_BATCH(gpgpu->batch, 0);
    /* If we output an AUB file, we limit the total size to 64MB */
#if USE_FULSIM
    OUT_BATCH(gpgpu->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* General State Access Upper Bound */
    OUT_BATCH(gpgpu->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Dynamic State Access Upper Bound */
    OUT_BATCH(gpgpu->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Indirect Obj Access Upper Bound */
    OUT_BATCH(gpgpu->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Instruction Access Upper Bound */
#else
    OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
    /* According to mesa i965 driver code, we must set the dynamic state access upper bound
     * to a valid bound value, otherwise, the border color pointer may be rejected and you
     * may get incorrect border color. This is a known hardware bug. */
    OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
#endif /* USE_FULSIM */
    ADVANCE_BATCH(gpgpu->batch);
}

uint32_t intel_gpgpu_get_scratch_index_gen7(uint32_t size) {
  return size / 1024 - 1;
}

uint32_t intel_gpgpu_get_scratch_index_gen75(uint32_t size) {
    //align in backend, if non pow2, must align when alloc scratch bo.
    assert((size & (size - 1)) == 0);
    size = size >> 11;
    uint32_t index = 0;
    while((size >>= 1) > 0)
      index++;   //get leading one

    return index;
}

uint32_t intel_gpgpu_get_scratch_index_gen8(uint32_t size) {
    //align in backend, if non pow2, must align when alloc scratch bo.
    assert((size & (size - 1)) == 0);
    size = size >> 10;
    uint32_t index = 0;
    while((size >>= 1) > 0)
      index++;   //get leading one

    return index;
}


static cl_int
intel_gpgpu_get_max_curbe_size(uint32_t device_id)
{
  if (IS_BAYTRAIL_T(device_id) ||
      IS_IVB_GT1(device_id))
    return 992;
  else
    return 2016;
}

static cl_int
intel_gpgpu_get_curbe_size(intel_gpgpu_t *gpgpu)
{
  int curbe_size = gpgpu->curb.size_cs_entry * gpgpu->curb.num_cs_entries;
  int max_curbe_size = intel_gpgpu_get_max_curbe_size(gpgpu->drv->device_id);

  if (curbe_size > max_curbe_size) {
    fprintf(stderr, "warning, curbe size exceed limitation.\n");
    return max_curbe_size;
  } else
    return curbe_size;
}

static void
intel_gpgpu_load_vfe_state_gen7(intel_gpgpu_t *gpgpu)
{
  int32_t scratch_index;
  BEGIN_BATCH(gpgpu->batch, 8);
  OUT_BATCH(gpgpu->batch, CMD_MEDIA_STATE_POINTERS | (8-2));

  if(gpgpu->per_thread_scratch > 0) {
    scratch_index = intel_gpgpu_get_scratch_index(gpgpu->per_thread_scratch);
    OUT_RELOC(gpgpu->batch, gpgpu->scratch_b.bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              scratch_index);
  }
  else {
    OUT_BATCH(gpgpu->batch, 0);
  }
  /* max_thread | urb entries | (reset_gateway|bypass_gate_way | gpgpu_mode) */
  OUT_BATCH(gpgpu->batch, 0 | ((gpgpu->max_threads - 1) << 16) | (0 << 8) | 0xc4);
  OUT_BATCH(gpgpu->batch, 0);
  /* curbe_size */
  OUT_BATCH(gpgpu->batch, intel_gpgpu_get_curbe_size(gpgpu));
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
intel_gpgpu_load_vfe_state_gen8(intel_gpgpu_t *gpgpu)
{
  int32_t scratch_index;
  BEGIN_BATCH(gpgpu->batch, 9);
  OUT_BATCH(gpgpu->batch, CMD_MEDIA_STATE_POINTERS | (9-2));

  if(gpgpu->per_thread_scratch > 0) {
    scratch_index = intel_gpgpu_get_scratch_index(gpgpu->per_thread_scratch);
    OUT_RELOC(gpgpu->batch, gpgpu->scratch_b.bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              scratch_index);
  }
  else {
    OUT_BATCH(gpgpu->batch, 0);
  }
  OUT_BATCH(gpgpu->batch, 0);

  /* max_thread | urb entries | (reset_gateway|bypass_gate_way | gpgpu_mode) */
  OUT_BATCH(gpgpu->batch, 0 | ((gpgpu->max_threads - 1) << 16) | (2 << 8) | 0xc0); //urb entries can't be 0
  OUT_BATCH(gpgpu->batch, 0);
  /* urb entries size | curbe_size */
  OUT_BATCH(gpgpu->batch, 2<<16 | intel_gpgpu_get_curbe_size(gpgpu));
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
intel_gpgpu_load_curbe_buffer_gen7(intel_gpgpu_t *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 4);
  OUT_BATCH(gpgpu->batch, CMD(2,0,1) | (4 - 2));  /* length-2 */
  OUT_BATCH(gpgpu->batch, 0);                     /* mbz */
  OUT_BATCH(gpgpu->batch, intel_gpgpu_get_curbe_size(gpgpu) * 32);
  OUT_RELOC(gpgpu->batch, gpgpu->aux_buf.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, gpgpu->aux_offset.curbe_offset);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
intel_gpgpu_load_curbe_buffer_gen8(intel_gpgpu_t *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 4);
  OUT_BATCH(gpgpu->batch, CMD(2,0,1) | (4 - 2));  /* length-2 */
  OUT_BATCH(gpgpu->batch, 0);                     /* mbz */
  OUT_BATCH(gpgpu->batch, intel_gpgpu_get_curbe_size(gpgpu) * 32);
  OUT_BATCH(gpgpu->batch, gpgpu->aux_offset.curbe_offset);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
intel_gpgpu_load_idrt_gen7(intel_gpgpu_t *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 4);
  OUT_BATCH(gpgpu->batch, CMD(2,0,2) | (4 - 2)); /* length-2 */
  OUT_BATCH(gpgpu->batch, 0);                    /* mbz */
  OUT_BATCH(gpgpu->batch, 1 << 5);
  OUT_RELOC(gpgpu->batch, gpgpu->aux_buf.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, gpgpu->aux_offset.idrt_offset);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
intel_gpgpu_load_idrt_gen8(intel_gpgpu_t *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 4);
  OUT_BATCH(gpgpu->batch, CMD(2,0,2) | (4 - 2)); /* length-2 */
  OUT_BATCH(gpgpu->batch, 0);                    /* mbz */
  OUT_BATCH(gpgpu->batch, 1 << 5);
  OUT_BATCH(gpgpu->batch, gpgpu->aux_offset.idrt_offset);
  ADVANCE_BATCH(gpgpu->batch);
}


static const uint32_t gpgpu_l3_config_reg1[] = {
  0x00080040, 0x02040040, 0x00800040, 0x01000038,
  0x02000030, 0x01000038, 0x00000038, 0x00000040,
  0x0A140091, 0x09100091, 0x08900091, 0x08900091,
  0x010000a1
};

static const uint32_t gpgpu_l3_config_reg2[] = {
  0x00000000, 0x00000000, 0x00080410, 0x00080410,
  0x00040410, 0x00040420, 0x00080420, 0x00080020,
  0x00204080, 0x00244890, 0x00284490, 0x002444A0,
  0x00040810
};

/* Emit PIPE_CONTROLs to write the current GPU timestamp into a buffer. */
static void
intel_gpgpu_write_timestamp(intel_gpgpu_t *gpgpu, int idx)
{
  BEGIN_BATCH(gpgpu->batch, 5);
  OUT_BATCH(gpgpu->batch, CMD_PIPE_CONTROL | (5-2));
  OUT_BATCH(gpgpu->batch, GEN7_PIPE_CONTROL_WRITE_TIMESTAMP);
  OUT_RELOC(gpgpu->batch, gpgpu->time_stamp_b.bo,
          I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
          GEN7_PIPE_CONTROL_GLOBAL_GTT_WRITE | idx * sizeof(uint64_t));
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  ADVANCE_BATCH();
}

static void
intel_gpgpu_pipe_control(intel_gpgpu_t *gpgpu)
{
  gen6_pipe_control_t* pc = (gen6_pipe_control_t*)
    intel_batchbuffer_alloc_space(gpgpu->batch, sizeof(gen6_pipe_control_t));
  memset(pc, 0, sizeof(*pc));
  pc->dw0.length = SIZEOF32(gen6_pipe_control_t) - 2;
  pc->dw0.instruction_subopcode = GEN7_PIPE_CONTROL_SUBOPCODE_3D_CONTROL;
  pc->dw0.instruction_opcode = GEN7_PIPE_CONTROL_OPCODE_3D_CONTROL;
  pc->dw0.instruction_pipeline = GEN7_PIPE_CONTROL_3D;
  pc->dw0.instruction_type = GEN7_PIPE_CONTROL_INSTRUCTION_GFX;
  pc->dw1.render_target_cache_flush_enable = 1;
  pc->dw1.texture_cache_invalidation_enable = 1;
  pc->dw1.cs_stall = 1;
  pc->dw1.dc_flush_enable = 1;
  //pc->dw1.instruction_cache_invalidate_enable = 1;
  ADVANCE_BATCH(gpgpu->batch);
}

static void
intel_gpgpu_set_L3_gen7(intel_gpgpu_t *gpgpu, uint32_t use_slm)
{
  BEGIN_BATCH(gpgpu->batch, 9);
  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_SQC_REG1_ADDRESS_OFFSET);
  OUT_BATCH(gpgpu->batch, 0x00730000);

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_CNTL_REG2_ADDRESS_OFFSET);

  if (use_slm)
    OUT_BATCH(gpgpu->batch, gpgpu_l3_config_reg1[12]);
  else
    OUT_BATCH(gpgpu->batch, gpgpu_l3_config_reg1[4]);

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_CNTL_REG3_ADDRESS_OFFSET);
  if (use_slm)
    OUT_BATCH(gpgpu->batch, gpgpu_l3_config_reg2[12]);
  else
    OUT_BATCH(gpgpu->batch, gpgpu_l3_config_reg2[4]);
  ADVANCE_BATCH(gpgpu->batch);

  intel_gpgpu_pipe_control(gpgpu);
}

static void
intel_gpgpu_set_L3_baytrail(intel_gpgpu_t *gpgpu, uint32_t use_slm)
{
  BEGIN_BATCH(gpgpu->batch, 9);

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_SQC_REG1_ADDRESS_OFFSET);
  OUT_BATCH(gpgpu->batch, 0x00D30000);    /* General credit : High credit = 26 : 6 */

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_CNTL_REG2_ADDRESS_OFFSET);
  if (use_slm)
    OUT_BATCH(gpgpu->batch, 0x01020021);  /* {SLM=64, URB=96, DC=16, RO=16, Sum=192} */
  else
    OUT_BATCH(gpgpu->batch, 0x02040040);  /* {SLM=0, URB=128, DC=32, RO=32, Sum=192} */

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_CNTL_REG3_ADDRESS_OFFSET);
  OUT_BATCH(gpgpu->batch, 0x0);           /* {I/S=0, Const=0, Tex=0} */

  ADVANCE_BATCH(gpgpu->batch);

  intel_gpgpu_pipe_control(gpgpu);
}

static void
intel_gpgpu_set_L3_gen75(intel_gpgpu_t *gpgpu, uint32_t use_slm)
{
  /* still set L3 in batch buffer for fulsim. */
  BEGIN_BATCH(gpgpu->batch, 9);
  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_SQC_REG1_ADDRESS_OFFSET);
  OUT_BATCH(gpgpu->batch, 0x00610000);

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_CNTL_REG2_ADDRESS_OFFSET);

  if (use_slm)
    OUT_BATCH(gpgpu->batch, gpgpu_l3_config_reg1[12]);
  else
    OUT_BATCH(gpgpu->batch, gpgpu_l3_config_reg1[4]);

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_CNTL_REG3_ADDRESS_OFFSET);
  if (use_slm)
    OUT_BATCH(gpgpu->batch, gpgpu_l3_config_reg2[12]);
  else
    OUT_BATCH(gpgpu->batch, gpgpu_l3_config_reg2[4]);
    ADVANCE_BATCH(gpgpu->batch);

  //if(use_slm)
  //  gpgpu->batch->enable_slm = 1;
  intel_gpgpu_pipe_control(gpgpu);
}

static void
intel_gpgpu_set_L3_gen8(intel_gpgpu_t *gpgpu, uint32_t use_slm)
{
  BEGIN_BATCH(gpgpu->batch, 3);
  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN8_L3_CNTL_REG_ADDRESS_OFFSET);
  // FIXME, this is a workaround for switch SLM enable and disable random hang
  if(use_slm)
    OUT_BATCH(gpgpu->batch, 0x60000121);  /* {SLM=192, URB=128, Rest=384} */
  else
    OUT_BATCH(gpgpu->batch, 0x60000160);  /* {SLM=0, URB=384, Rest=384, Sum=768} */

  //if(use_slm)
  //  gpgpu->batch->enable_slm = 1;
  intel_gpgpu_pipe_control(gpgpu);
}

static void
intel_gpgpu_batch_start(intel_gpgpu_t *gpgpu)
{
  intel_batchbuffer_start_atomic(gpgpu->batch, 256);
  intel_gpgpu_pipe_control(gpgpu);
  assert(intel_gpgpu_set_L3);
  intel_gpgpu_set_L3(gpgpu, gpgpu->ker->use_slm);
  intel_gpgpu_select_pipeline(gpgpu);
  intel_gpgpu_set_base_address(gpgpu);
  intel_gpgpu_load_vfe_state(gpgpu);
  intel_gpgpu_load_curbe_buffer(gpgpu);
  intel_gpgpu_load_idrt(gpgpu);

  if (gpgpu->perf_b.bo) {
    BEGIN_BATCH(gpgpu->batch, 3);
    OUT_BATCH(gpgpu->batch,
              (0x28 << 23) | /* MI_REPORT_PERF_COUNT */
              (3 - 2));      /* length-2 */
    OUT_RELOC(gpgpu->batch, gpgpu->perf_b.bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              0 |  /* Offset for the start "counters" */
              1);  /* Use GTT and not PGTT */
    OUT_BATCH(gpgpu->batch, 0);
    ADVANCE_BATCH(gpgpu->batch);
  }

  /* Insert PIPE_CONTROL for time stamp of start*/
  if (gpgpu->time_stamp_b.bo)
    intel_gpgpu_write_timestamp(gpgpu, 0);
}

static void
intel_gpgpu_post_action_gen7(intel_gpgpu_t *gpgpu, int32_t flush_mode)
{
  if(flush_mode)
    intel_gpgpu_pipe_control(gpgpu);
}

static void
intel_gpgpu_post_action_gen75(intel_gpgpu_t *gpgpu, int32_t flush_mode)
{
  /* flush force for set L3 */
  intel_gpgpu_pipe_control(gpgpu);

  /* Restore L3 control to disable SLM mode,
     otherwise, may affect 3D pipeline */
  intel_gpgpu_set_L3(gpgpu, 0);
}

static void
intel_gpgpu_batch_end(intel_gpgpu_t *gpgpu, int32_t flush_mode)
{
  /* Insert PIPE_CONTROL for time stamp of end*/
  if (gpgpu->time_stamp_b.bo)
    intel_gpgpu_write_timestamp(gpgpu, 1);

  /* Insert the performance counter command */
  if (gpgpu->perf_b.bo) {
    BEGIN_BATCH(gpgpu->batch, 3);
    OUT_BATCH(gpgpu->batch,
              (0x28 << 23) | /* MI_REPORT_PERF_COUNT */
              (3 - 2));      /* length-2 */
    OUT_RELOC(gpgpu->batch, gpgpu->perf_b.bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              512 |  /* Offset for the end "counters" */
              1);    /* Use GTT and not PGTT */
    OUT_BATCH(gpgpu->batch, 0);
    ADVANCE_BATCH(gpgpu->batch);
  }

  intel_gpgpu_post_action(gpgpu, flush_mode);
  intel_batchbuffer_end_atomic(gpgpu->batch);
}

static int
intel_gpgpu_batch_reset(intel_gpgpu_t *gpgpu, size_t sz)
{
  return intel_batchbuffer_reset(gpgpu->batch, sz);
}

static void
intel_gpgpu_flush_batch_buffer(intel_batchbuffer_t *batch)
{
  assert(batch);
  intel_batchbuffer_emit_mi_flush(batch);
  intel_batchbuffer_flush(batch);
}

static void
intel_gpgpu_flush(intel_gpgpu_t *gpgpu)
{
  if (!gpgpu->batch || !gpgpu->batch->buffer)
    return;
  intel_gpgpu_flush_batch_buffer(gpgpu->batch);
  /* FIXME:
     Remove old assert here for binded buffer offset 0 which
     tried to guard possible NULL buffer pointer check in kernel, as
     in case like "runtime_null_kernel_arg", but that's wrong to just
     take buffer offset 0 as NULL, and cause failure for normal
     kernels which has no such NULL ptr check but with buffer offset 0
     (which is possible now and will be normal if full PPGTT is on).

     Need to fix NULL ptr check otherwise.
  */
}

static int
intel_gpgpu_state_init(intel_gpgpu_t *gpgpu,
                       uint32_t max_threads,
                       uint32_t size_cs_entry,
                       int profiling)
{
  drm_intel_bo *bo;

  /* Binded buffers */
  gpgpu->binded_n = 0;
  gpgpu->img_bitmap = 0;
  gpgpu->img_index_base = 3;
  gpgpu->sampler_bitmap = ~((1 << max_sampler_n) - 1);

  /* URB */
  gpgpu->curb.num_cs_entries = 64;
  gpgpu->curb.size_cs_entry = size_cs_entry;
  gpgpu->max_threads = max_threads;

  if (gpgpu->printf_b.ibo)
    dri_bo_unreference(gpgpu->printf_b.ibo);
  gpgpu->printf_b.ibo = NULL;
  if (gpgpu->printf_b.bo)
    dri_bo_unreference(gpgpu->printf_b.bo);
  gpgpu->printf_b.bo = NULL;

  /* Set the profile buffer*/
  if(gpgpu->time_stamp_b.bo)
    dri_bo_unreference(gpgpu->time_stamp_b.bo);
  gpgpu->time_stamp_b.bo = NULL;
  if (profiling) {
    bo = dri_bo_alloc(gpgpu->drv->bufmgr, "timestamp query", 4096, 4096);
    gpgpu->time_stamp_b.bo = bo;
    if (!bo)
      fprintf(stderr, "Could not allocate buffer for profiling.\n");
  }

  /* stack */
  if (gpgpu->stack_b.bo)
    dri_bo_unreference(gpgpu->stack_b.bo);
  gpgpu->stack_b.bo = NULL;

  /* Set the auxiliary buffer*/
  uint32_t size_aux = 0;
  if(gpgpu->aux_buf.bo)
    dri_bo_unreference(gpgpu->aux_buf.bo);
  gpgpu->aux_buf.bo = NULL;

  /* begin with surface heap to make sure it's page aligned,
     because state base address use 20bit for the address */
  gpgpu->aux_offset.surface_heap_offset = size_aux;
  size_aux += sizeof(surface_heap_t);

  //curbe must be 32 bytes aligned
  size_aux = ALIGN(size_aux, 64);
  gpgpu->aux_offset.curbe_offset = size_aux;
  size_aux += gpgpu->curb.num_cs_entries * gpgpu->curb.size_cs_entry * 32;

  //idrt must be 32 bytes aligned
  size_aux = ALIGN(size_aux, 32);
  gpgpu->aux_offset.idrt_offset = size_aux;
  size_aux += MAX_IF_DESC * sizeof(struct gen6_interface_descriptor);

  //sampler state must be 32 bytes aligned
  size_aux = ALIGN(size_aux, 32);
  gpgpu->aux_offset.sampler_state_offset = size_aux;
  size_aux += GEN_MAX_SAMPLERS * sizeof(gen6_sampler_state_t);

  //sampler border color state must be 32 bytes aligned
  size_aux = ALIGN(size_aux, 32);
  gpgpu->aux_offset.sampler_border_color_state_offset = size_aux;
  size_aux += GEN_MAX_SAMPLERS * sizeof(gen7_sampler_border_color_t);

  /* make sure aux buffer is page aligned */
  size_aux = ALIGN(size_aux, 4096);

  bo = dri_bo_alloc(gpgpu->drv->bufmgr, "AUX_BUFFER", size_aux, 4096);
  if (!bo || dri_bo_map(bo, 1) != 0) {
    fprintf(stderr, "%s:%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    if (bo)
      dri_bo_unreference(bo);
    if (profiling && gpgpu->time_stamp_b.bo)
      dri_bo_unreference(gpgpu->time_stamp_b.bo);
    gpgpu->time_stamp_b.bo = NULL;
    return -1;
  }
  memset(bo->virtual, 0, size_aux);
  gpgpu->aux_buf.bo = bo;
  return 0;
}

static void
intel_gpgpu_set_buf_reloc_gen7(intel_gpgpu_t *gpgpu, int32_t index, dri_bo* obj_bo, uint32_t obj_bo_offset)
{
  surface_heap_t *heap = gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.surface_heap_offset;
  heap->binding_table[index] = offsetof(surface_heap_t, surface) +
                               index * sizeof(gen7_surface_state_t);
  dri_bo_emit_reloc(gpgpu->aux_buf.bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    obj_bo_offset,
                    gpgpu->aux_offset.surface_heap_offset +
                    heap->binding_table[index] +
                    offsetof(gen7_surface_state_t, ss1),
                    obj_bo);
}

static dri_bo*
intel_gpgpu_alloc_constant_buffer(intel_gpgpu_t *gpgpu, uint32_t size, uint8_t bti)
{
  if(gpgpu->constant_b.bo)
    dri_bo_unreference(gpgpu->constant_b.bo);
  gpgpu->constant_b.bo = drm_intel_bo_alloc(gpgpu->drv->bufmgr, "CONSTANT_BUFFER", size, 64);
  if (gpgpu->constant_b.bo == NULL)
    return NULL;

  intel_gpgpu_setup_bti(gpgpu, gpgpu->constant_b.bo, 0, size, bti, I965_SURFACEFORMAT_R32G32B32A32_UINT);
  return gpgpu->constant_b.bo;
}

static void
intel_gpgpu_setup_bti_gen7(intel_gpgpu_t *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                                   uint32_t size, unsigned char index, uint32_t format)
{
  uint32_t s = size - 1;
  surface_heap_t *heap = gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.surface_heap_offset;
  gen7_surface_state_t *ss0 = (gen7_surface_state_t *) &heap->surface[index * sizeof(gen7_surface_state_t)];
  memset(ss0, 0, sizeof(gen7_surface_state_t));
  ss0->ss0.surface_type = I965_SURFACE_BUFFER;
  ss0->ss0.surface_format = format;
  ss0->ss2.width  = s & 0x7f;   /* bits 6:0 of sz */
  // Per bspec, I965_SURFACE_BUFFER and RAW format, size must be a multiple of 4 byte.
  if(format == I965_SURFACEFORMAT_RAW)
    assert((ss0->ss2.width & 0x03) == 3);
  ss0->ss2.height = (s >> 7) & 0x3fff; /* bits 20:7 of sz */
  ss0->ss3.depth  = (s >> 21) & 0x3ff; /* bits 30:21 of sz */
  ss0->ss5.cache_control = cl_gpgpu_get_cache_ctrl();
  heap->binding_table[index] = offsetof(surface_heap_t, surface) + index * sizeof(gen7_surface_state_t);

  ss0->ss1.base_addr = buf->offset + internal_offset;
  dri_bo_emit_reloc(gpgpu->aux_buf.bo,
                      I915_GEM_DOMAIN_RENDER,
                      I915_GEM_DOMAIN_RENDER,
                      internal_offset,
                      gpgpu->aux_offset.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen7_surface_state_t, ss1),
                      buf);
}

static void
intel_gpgpu_setup_bti_gen75(intel_gpgpu_t *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                                   uint32_t size, unsigned char index, uint32_t format)
{
  uint32_t s = size - 1;
  surface_heap_t *heap = gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.surface_heap_offset;
  gen7_surface_state_t *ss0 = (gen7_surface_state_t *) &heap->surface[index * sizeof(gen7_surface_state_t)];
  memset(ss0, 0, sizeof(gen7_surface_state_t));
  ss0->ss0.surface_type = I965_SURFACE_BUFFER;
  ss0->ss0.surface_format = format;
  if(format != I965_SURFACEFORMAT_RAW) {
    ss0->ss7.shader_r = I965_SURCHAN_SELECT_RED;
    ss0->ss7.shader_g = I965_SURCHAN_SELECT_GREEN;
    ss0->ss7.shader_b = I965_SURCHAN_SELECT_BLUE;
    ss0->ss7.shader_a = I965_SURCHAN_SELECT_ALPHA;
  }
  ss0->ss2.width  = s & 0x7f;   /* bits 6:0 of sz */
  // Per bspec, I965_SURFACE_BUFFER and RAW format, size must be a multiple of 4 byte.
  if(format == I965_SURFACEFORMAT_RAW)
    assert((ss0->ss2.width & 0x03) == 3);
  ss0->ss2.height = (s >> 7) & 0x3fff; /* bits 20:7 of sz */
  ss0->ss3.depth  = (s >> 21) & 0x3ff; /* bits 30:21 of sz */
  ss0->ss5.cache_control = cl_gpgpu_get_cache_ctrl();
  heap->binding_table[index] = offsetof(surface_heap_t, surface) + index * sizeof(gen7_surface_state_t);

  ss0->ss1.base_addr = buf->offset + internal_offset;
  dri_bo_emit_reloc(gpgpu->aux_buf.bo,
                      I915_GEM_DOMAIN_RENDER,
                      I915_GEM_DOMAIN_RENDER,
                      internal_offset,
                      gpgpu->aux_offset.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen7_surface_state_t, ss1),
                      buf);
}

static void
intel_gpgpu_setup_bti_gen8(intel_gpgpu_t *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                                   uint32_t size, unsigned char index, uint32_t format)
{
  uint32_t s = size - 1;
  surface_heap_t *heap = gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.surface_heap_offset;
  gen8_surface_state_t *ss0 = (gen8_surface_state_t *) &heap->surface[index * sizeof(gen8_surface_state_t)];
  memset(ss0, 0, sizeof(gen8_surface_state_t));
  ss0->ss0.surface_type = I965_SURFACE_BUFFER;
  ss0->ss0.surface_format = format;
  if(format != I965_SURFACEFORMAT_RAW) {
    ss0->ss7.shader_channel_select_red = I965_SURCHAN_SELECT_RED;
    ss0->ss7.shader_channel_select_green = I965_SURCHAN_SELECT_GREEN;
    ss0->ss7.shader_channel_select_blue = I965_SURCHAN_SELECT_BLUE;
    ss0->ss7.shader_channel_select_alpha = I965_SURCHAN_SELECT_ALPHA;
  }
  ss0->ss2.width  = s & 0x7f;   /* bits 6:0 of sz */
  // Per bspec, I965_SURFACE_BUFFER and RAW format, size must be a multiple of 4 byte.
  if(format == I965_SURFACEFORMAT_RAW)
    assert((ss0->ss2.width & 0x03) == 3);
  ss0->ss2.height = (s >> 7) & 0x3fff; /* bits 20:7 of sz */
  ss0->ss3.depth  = (s >> 21) & 0x3ff; /* bits 30:21 of sz */
  ss0->ss1.mem_obj_ctrl_state = cl_gpgpu_get_cache_ctrl();
  heap->binding_table[index] = offsetof(surface_heap_t, surface) + index * sizeof(gen8_surface_state_t);
  ss0->ss8.surface_base_addr_lo = (buf->offset64 + internal_offset) & 0xffffffff;
  ss0->ss9.surface_base_addr_hi = ((buf->offset64 + internal_offset) >> 32) & 0xffffffff;
  dri_bo_emit_reloc(gpgpu->aux_buf.bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    internal_offset,
                    gpgpu->aux_offset.surface_heap_offset +
                    heap->binding_table[index] +
                    offsetof(gen8_surface_state_t, ss8),
                    buf);
}

static int
intel_is_surface_array(cl_mem_object_type type)
{
  if (type == CL_MEM_OBJECT_IMAGE1D_ARRAY ||
        type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
    return 1;

  return 0;
}

static int
intel_get_surface_type(cl_mem_object_type type)
{
  switch (type) {
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
    return I965_SURFACE_1D;

  case CL_MEM_OBJECT_IMAGE2D:
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
    return I965_SURFACE_2D;

  case CL_MEM_OBJECT_IMAGE3D:
    return I965_SURFACE_3D;

  default:
      assert(0);
  }
  return 0;
}

/* Get fixed surface type. If it is a 1D array image with a large index,
   we need to fixup it to 2D type due to a Gen7/Gen75's sampler issue
   on a integer type surface with clamp address mode and nearest filter mode.
*/
static uint32_t get_surface_type(intel_gpgpu_t *gpgpu, int index, cl_mem_object_type type)
{
  uint32_t surface_type;
  if (((IS_IVYBRIDGE(gpgpu->drv->device_id) ||
        IS_HASWELL(gpgpu->drv->device_id) ||
        IS_BROADWELL(gpgpu->drv->device_id))) &&
      index >= BTI_MAX_IMAGE_NUM + BTI_RESERVED_NUM &&
      type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
    surface_type = I965_SURFACE_2D;
  else
    surface_type = intel_get_surface_type(type);
  return surface_type;
}

static void
intel_gpgpu_bind_image_gen7(intel_gpgpu_t *gpgpu,
                              uint32_t index,
                              dri_bo* obj_bo,
                              uint32_t obj_bo_offset,
                              uint32_t format,
                              cl_mem_object_type type,
                              int32_t w,
                              int32_t h,
                              int32_t depth,
                              int32_t pitch,
                              int32_t tiling)
{
  surface_heap_t *heap = gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.surface_heap_offset;
  gen7_surface_state_t *ss = (gen7_surface_state_t *) &heap->surface[index * sizeof(gen7_surface_state_t)];

  memset(ss, 0, sizeof(*ss));
  ss->ss0.vertical_line_stride = 0; // always choose VALIGN_2
  ss->ss0.surface_type = get_surface_type(gpgpu, index, type);
  if (intel_is_surface_array(type)) {
    ss->ss0.surface_array = 1;
    ss->ss0.surface_array_spacing = 1;
  }
  ss->ss0.surface_format = format;
  ss->ss1.base_addr = obj_bo->offset;
  ss->ss2.width = w - 1;

  ss->ss2.height = h - 1;
  ss->ss3.depth = depth - 1;
  ss->ss4.not_str_buf.rt_view_extent = depth - 1;
  ss->ss4.not_str_buf.min_array_element = 0;
  ss->ss3.pitch = pitch - 1;
  ss->ss5.cache_control = cl_gpgpu_get_cache_ctrl();
  if (tiling == GPGPU_TILE_X) {
    ss->ss0.tiled_surface = 1;
    ss->ss0.tile_walk = I965_TILEWALK_XMAJOR;
  } else if (tiling == GPGPU_TILE_Y) {
    ss->ss0.tiled_surface = 1;
    ss->ss0.tile_walk = I965_TILEWALK_YMAJOR;
  }
  ss->ss0.render_cache_rw_mode = 1; /* XXX do we need to set it? */
  intel_gpgpu_set_buf_reloc_gen7(gpgpu, index, obj_bo, obj_bo_offset);

  assert(index < GEN_MAX_SURFACES);
}

static void
intel_gpgpu_bind_image_gen75(intel_gpgpu_t *gpgpu,
                              uint32_t index,
                              dri_bo* obj_bo,
                              uint32_t obj_bo_offset,
                              uint32_t format,
                              cl_mem_object_type type,
                              int32_t w,
                              int32_t h,
                              int32_t depth,
                              int32_t pitch,
                              int32_t tiling)
{
  surface_heap_t *heap = gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.surface_heap_offset;
  gen7_surface_state_t *ss = (gen7_surface_state_t *) &heap->surface[index * sizeof(gen7_surface_state_t)];
  memset(ss, 0, sizeof(*ss));
  ss->ss0.vertical_line_stride = 0; // always choose VALIGN_2
  ss->ss0.surface_type = get_surface_type(gpgpu, index, type);
  if (intel_is_surface_array(type)) {
    ss->ss0.surface_array = 1;
    ss->ss0.surface_array_spacing = 1;
  }
  ss->ss0.surface_format = format;
  ss->ss1.base_addr = obj_bo->offset;
  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.depth = depth - 1;
  ss->ss4.not_str_buf.rt_view_extent = depth - 1;
  ss->ss4.not_str_buf.min_array_element = 0;
  ss->ss3.pitch = pitch - 1;
  ss->ss5.cache_control = cl_gpgpu_get_cache_ctrl();
  ss->ss7.shader_r = I965_SURCHAN_SELECT_RED;
  ss->ss7.shader_g = I965_SURCHAN_SELECT_GREEN;
  ss->ss7.shader_b = I965_SURCHAN_SELECT_BLUE;
  ss->ss7.shader_a = I965_SURCHAN_SELECT_ALPHA;
  if (tiling == GPGPU_TILE_X) {
    ss->ss0.tiled_surface = 1;
    ss->ss0.tile_walk = I965_TILEWALK_XMAJOR;
  } else if (tiling == GPGPU_TILE_Y) {
    ss->ss0.tiled_surface = 1;
    ss->ss0.tile_walk = I965_TILEWALK_YMAJOR;
  }
  ss->ss0.render_cache_rw_mode = 1; /* XXX do we need to set it? */
  intel_gpgpu_set_buf_reloc_gen7(gpgpu, index, obj_bo, obj_bo_offset);

  assert(index < GEN_MAX_SURFACES);
}

static void
intel_gpgpu_bind_image_gen8(intel_gpgpu_t *gpgpu,
                            uint32_t index,
                            dri_bo* obj_bo,
                            uint32_t obj_bo_offset,
                            uint32_t format,
                            cl_mem_object_type type,
                            int32_t w,
                            int32_t h,
                            int32_t depth,
                            int32_t pitch,
                            int32_t tiling)
{
  surface_heap_t *heap = gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.surface_heap_offset;
  gen8_surface_state_t *ss = (gen8_surface_state_t *) &heap->surface[index * sizeof(gen8_surface_state_t)];
  memset(ss, 0, sizeof(*ss));
  ss->ss0.vertical_line_stride = 0; // always choose VALIGN_2
  ss->ss0.surface_type = get_surface_type(gpgpu, index, type);
  ss->ss0.surface_format = format;
  if (intel_is_surface_array(type)) {
    ss->ss0.surface_array = 1;
    ss->ss1.surface_qpitch = (h + 3)/4;
  }
  ss->ss0.horizontal_alignment = 1;
  ss->ss0.vertical_alignment = 1;

  if (tiling == GPGPU_TILE_X) {
    ss->ss0.tile_mode = GEN8_TILEMODE_XMAJOR;
  } else if (tiling == GPGPU_TILE_Y) {
    ss->ss0.tile_mode = GEN8_TILEMODE_YMAJOR;
  } else
    assert(tiling == GPGPU_NO_TILE);// W mode is not supported now.

  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.depth = depth - 1;

  ss->ss8.surface_base_addr_lo = obj_bo->offset64 & 0xffffffff;
  ss->ss9.surface_base_addr_hi = (obj_bo->offset64 >> 32) & 0xffffffff;

  ss->ss4.render_target_view_ext = depth - 1;
  ss->ss4.min_array_elt = 0;
  ss->ss3.surface_pitch = pitch - 1;

  ss->ss1.mem_obj_ctrl_state = cl_gpgpu_get_cache_ctrl();
  ss->ss7.shader_channel_select_red = I965_SURCHAN_SELECT_RED;
  ss->ss7.shader_channel_select_green = I965_SURCHAN_SELECT_GREEN;
  ss->ss7.shader_channel_select_blue = I965_SURCHAN_SELECT_BLUE;
  ss->ss7.shader_channel_select_alpha = I965_SURCHAN_SELECT_ALPHA;
  ss->ss0.render_cache_rw_mode = 1; /* XXX do we need to set it? */

  heap->binding_table[index] = offsetof(surface_heap_t, surface) +
                               index * surface_state_sz;
  dri_bo_emit_reloc(gpgpu->aux_buf.bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    obj_bo_offset,
                    gpgpu->aux_offset.surface_heap_offset +
                    heap->binding_table[index] +
                    offsetof(gen8_surface_state_t, ss8),
                    obj_bo);

  assert(index < GEN_MAX_SURFACES);
}

static void
intel_gpgpu_bind_buf(intel_gpgpu_t *gpgpu, drm_intel_bo *buf, uint32_t offset,
                     uint32_t internal_offset, uint32_t size, uint8_t bti)
{
  assert(gpgpu->binded_n < max_buf_n);
  gpgpu->binded_buf[gpgpu->binded_n] = buf;
  gpgpu->target_buf_offset[gpgpu->binded_n] = internal_offset;
  gpgpu->binded_offset[gpgpu->binded_n] = offset;
  gpgpu->binded_n++;
  intel_gpgpu_setup_bti(gpgpu, buf, internal_offset, size, bti, I965_SURFACEFORMAT_RAW);
}

static int
intel_gpgpu_set_scratch(intel_gpgpu_t * gpgpu, uint32_t per_thread_size)
{
  drm_intel_bufmgr *bufmgr = gpgpu->drv->bufmgr;
  drm_intel_bo* old = gpgpu->scratch_b.bo;
  uint32_t total = per_thread_size * gpgpu->max_threads;
  /* Per Bspec, scratch should 2X the desired size, otherwise luxmark may hang */
  if (IS_HASWELL(gpgpu->drv->device_id))
      total *= 2;

  gpgpu->per_thread_scratch = per_thread_size;

  if(old && old->size < total) {
    drm_intel_bo_unreference(old);
    old = NULL;
  }

  if(!old && total) {
    gpgpu->scratch_b.bo = drm_intel_bo_alloc(bufmgr, "SCRATCH_BO", total, 4096);
    if (gpgpu->scratch_b.bo == NULL)
      return -1;
  }
  return 0;
}
static void
intel_gpgpu_set_stack(intel_gpgpu_t *gpgpu, uint32_t offset, uint32_t size, uint8_t bti)
{
  drm_intel_bufmgr *bufmgr = gpgpu->drv->bufmgr;
  gpgpu->stack_b.bo = drm_intel_bo_alloc(bufmgr, "STACK", size, 64);

  cl_gpgpu_bind_buf((cl_gpgpu)gpgpu, (cl_buffer)gpgpu->stack_b.bo, offset, 0, size, bti);
}

static void
intel_gpgpu_build_idrt_gen7(intel_gpgpu_t *gpgpu, cl_gpgpu_kernel *kernel)
{
  gen6_interface_descriptor_t *desc;
  drm_intel_bo *ker_bo = NULL;

  desc = (gen6_interface_descriptor_t*) (gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.idrt_offset);

  memset(desc, 0, sizeof(*desc));
  ker_bo = (drm_intel_bo *) kernel->bo;
  desc->desc0.kernel_start_pointer = ker_bo->offset >> 6; /* reloc */
  desc->desc1.single_program_flow = 0;
  desc->desc1.floating_point_mode = 0; /* use IEEE-754 rule */
  desc->desc5.rounding_mode = 0; /* round to nearest even */

  assert((gpgpu->aux_buf.bo->offset + gpgpu->aux_offset.sampler_state_offset) % 32 == 0);
  desc->desc2.sampler_state_pointer = (gpgpu->aux_buf.bo->offset + gpgpu->aux_offset.sampler_state_offset) >> 5;
  desc->desc3.binding_table_entry_count = 0; /* no prefetch */
  desc->desc3.binding_table_pointer = 0;
  desc->desc4.curbe_read_len = kernel->curbe_sz / 32;
  desc->desc4.curbe_read_offset = 0;

  /* Barriers / SLM are automatically handled on Gen7+ */
  if (gpgpu->drv->gen_ver == 7 || gpgpu->drv->gen_ver == 75) {
    size_t slm_sz = kernel->slm_sz;
    desc->desc5.group_threads_num = kernel->use_slm ? kernel->thread_n : 0;
    desc->desc5.barrier_enable = kernel->use_slm;
    if (slm_sz <= 4*KB)
      slm_sz = 4*KB;
    else if (slm_sz <= 8*KB)
      slm_sz = 8*KB;
    else if (slm_sz <= 16*KB)
      slm_sz = 16*KB;
    else if (slm_sz <= 32*KB)
      slm_sz = 32*KB;
    else
      slm_sz = 64*KB;
    slm_sz = slm_sz >> 12;
    desc->desc5.slm_sz = slm_sz;
  }
  else
    desc->desc5.group_threads_num = kernel->barrierID; /* BarrierID on GEN6 */

  dri_bo_emit_reloc(gpgpu->aux_buf.bo,
                    I915_GEM_DOMAIN_INSTRUCTION, 0,
                    0,
                    gpgpu->aux_offset.idrt_offset + offsetof(gen6_interface_descriptor_t, desc0),
                    ker_bo);

  dri_bo_emit_reloc(gpgpu->aux_buf.bo,
                    I915_GEM_DOMAIN_SAMPLER, 0,
                    gpgpu->aux_offset.sampler_state_offset,
                    gpgpu->aux_offset.idrt_offset + offsetof(gen6_interface_descriptor_t, desc2),
                    gpgpu->aux_buf.bo);
}

static void
intel_gpgpu_build_idrt_gen8(intel_gpgpu_t *gpgpu, cl_gpgpu_kernel *kernel)
{
  gen8_interface_descriptor_t *desc;

  desc = (gen8_interface_descriptor_t*) (gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.idrt_offset);

  memset(desc, 0, sizeof(*desc));
  desc->desc0.kernel_start_pointer = 0; /* reloc */
  desc->desc2.single_program_flow = 0;
  desc->desc2.floating_point_mode = 0; /* use IEEE-754 rule */
  desc->desc6.rounding_mode = 0; /* round to nearest even */

  assert((gpgpu->aux_buf.bo->offset + gpgpu->aux_offset.sampler_state_offset) % 32 == 0);
  desc->desc3.sampler_state_pointer = gpgpu->aux_offset.sampler_state_offset >> 5;
  desc->desc4.binding_table_entry_count = 0; /* no prefetch */
  desc->desc4.binding_table_pointer = 0;
  desc->desc5.curbe_read_len = kernel->curbe_sz / 32;
  desc->desc5.curbe_read_offset = 0;

  /* Barriers / SLM are automatically handled on Gen7+ */
  size_t slm_sz = kernel->slm_sz;
  /* group_threads_num should not be set to 0 even if the barrier is disabled per bspec */
  desc->desc6.group_threads_num = kernel->thread_n;
  desc->desc6.barrier_enable = kernel->use_slm;
  if (slm_sz == 0)
    slm_sz = 0;
  else if (slm_sz <= 4*KB)
    slm_sz = 4*KB;
  else if (slm_sz <= 8*KB)
    slm_sz = 8*KB;
  else if (slm_sz <= 16*KB)
    slm_sz = 16*KB;
  else if (slm_sz <= 32*KB)
    slm_sz = 32*KB;
  else
    slm_sz = 64*KB;
  slm_sz = slm_sz >> 12;
  desc->desc6.slm_sz = slm_sz;
}

static int
intel_gpgpu_upload_curbes(intel_gpgpu_t *gpgpu, const void* data, uint32_t size)
{
  unsigned char *curbe = NULL;
  cl_gpgpu_kernel *k = gpgpu->ker;
  uint32_t i, j;

  /* Upload the data first */
  if (dri_bo_map(gpgpu->aux_buf.bo, 1) != 0) {
    fprintf(stderr, "%s:%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  }
  assert(gpgpu->aux_buf.bo->virtual);
  curbe = (unsigned char *) (gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.curbe_offset);
  memcpy(curbe, data, size);

  /* Now put all the relocations for our flat address space */
  for (i = 0; i < k->thread_n; ++i)
    for (j = 0; j < gpgpu->binded_n; ++j) {
      *(uint32_t*)(curbe + gpgpu->binded_offset[j]+i*k->curbe_sz) = gpgpu->binded_buf[j]->offset + gpgpu->target_buf_offset[j];
      drm_intel_bo_emit_reloc(gpgpu->aux_buf.bo,
                              gpgpu->aux_offset.curbe_offset + gpgpu->binded_offset[j]+i*k->curbe_sz,
                              gpgpu->binded_buf[j],
                              gpgpu->target_buf_offset[j],
                              I915_GEM_DOMAIN_RENDER,
                              I915_GEM_DOMAIN_RENDER);
    }
  dri_bo_unmap(gpgpu->aux_buf.bo);
  return 0;
}

static void
intel_gpgpu_upload_samplers(intel_gpgpu_t *gpgpu, const void *data, uint32_t n)
{
  if (n) {
    const size_t sz = n * sizeof(gen6_sampler_state_t);
    memcpy(gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.sampler_state_offset, data, sz);
  }
}

int translate_wrap_mode(uint32_t cl_address_mode, int using_nearest)
{
   switch( cl_address_mode ) {
   case CLK_ADDRESS_NONE:
   case CLK_ADDRESS_REPEAT:
      return GEN_TEXCOORDMODE_WRAP;
   case CLK_ADDRESS_CLAMP:
      return GEN_TEXCOORDMODE_CLAMP_BORDER;
   case CLK_ADDRESS_CLAMP_TO_EDGE:
      return GEN_TEXCOORDMODE_CLAMP;
   case CLK_ADDRESS_MIRRORED_REPEAT:
      return GEN_TEXCOORDMODE_MIRROR;
   default:
      return GEN_TEXCOORDMODE_WRAP;
   }
}

static void
intel_gpgpu_insert_sampler_gen7(intel_gpgpu_t *gpgpu, uint32_t index, uint32_t clk_sampler)
{
  int using_nearest = 0;
  uint32_t wrap_mode;
  gen7_sampler_state_t *sampler;

  sampler = (gen7_sampler_state_t *)(gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.sampler_state_offset)  + index;
  memset(sampler, 0, sizeof(*sampler));
  assert((gpgpu->aux_buf.bo->offset + gpgpu->aux_offset.sampler_border_color_state_offset) % 32 == 0);
  sampler->ss2.default_color_pointer = (gpgpu->aux_buf.bo->offset + gpgpu->aux_offset.sampler_border_color_state_offset) >> 5;
  if ((clk_sampler & __CLK_NORMALIZED_MASK) == CLK_NORMALIZED_COORDS_FALSE)
    sampler->ss3.non_normalized_coord = 1;
  else
    sampler->ss3.non_normalized_coord = 0;

  switch (clk_sampler & __CLK_FILTER_MASK) {
  case CLK_FILTER_NEAREST:
    sampler->ss0.min_filter = GEN_MAPFILTER_NEAREST;
    sampler->ss0.mip_filter = GEN_MIPFILTER_NONE;
    sampler->ss0.mag_filter = GEN_MAPFILTER_NEAREST;
    using_nearest = 1;
    break;
  case CLK_FILTER_LINEAR:
    sampler->ss0.min_filter = GEN_MAPFILTER_LINEAR;
    sampler->ss0.mip_filter = GEN_MIPFILTER_NONE;
    sampler->ss0.mag_filter = GEN_MAPFILTER_LINEAR;
    break;
  }

  wrap_mode = translate_wrap_mode(clk_sampler & __CLK_ADDRESS_MASK, using_nearest);
  sampler->ss3.s_wrap_mode = wrap_mode;
  /* XXX mesa i965 driver code point out that if the surface is a 1D surface, we may need
   * to set t_wrap_mode to GEN_TEXCOORDMODE_WRAP. */
  sampler->ss3.t_wrap_mode = wrap_mode;
  sampler->ss3.r_wrap_mode = wrap_mode;

  sampler->ss0.lod_preclamp = 1; /* OpenGL mode */
  sampler->ss0.default_color_mode = 0; /* OpenGL/DX10 mode */

  sampler->ss0.base_level = 0;

  sampler->ss1.max_lod = 0;
  sampler->ss1.min_lod = 0;

  if (sampler->ss0.min_filter != GEN_MAPFILTER_NEAREST)
     sampler->ss3.address_round |= GEN_ADDRESS_ROUNDING_ENABLE_U_MIN |
                                   GEN_ADDRESS_ROUNDING_ENABLE_V_MIN |
                                   GEN_ADDRESS_ROUNDING_ENABLE_R_MIN;
  if (sampler->ss0.mag_filter != GEN_MAPFILTER_NEAREST)
     sampler->ss3.address_round |= GEN_ADDRESS_ROUNDING_ENABLE_U_MAG |
                                   GEN_ADDRESS_ROUNDING_ENABLE_V_MAG |
                                   GEN_ADDRESS_ROUNDING_ENABLE_R_MAG;

  dri_bo_emit_reloc(gpgpu->aux_buf.bo,
                    I915_GEM_DOMAIN_SAMPLER, 0,
                    gpgpu->aux_offset.sampler_border_color_state_offset,
                    gpgpu->aux_offset.sampler_state_offset +
                    index * sizeof(gen7_sampler_state_t) +
                    offsetof(gen7_sampler_state_t, ss2),
                    gpgpu->aux_buf.bo);

}


static void
intel_gpgpu_insert_sampler_gen8(intel_gpgpu_t *gpgpu, uint32_t index, uint32_t clk_sampler)
{
  int using_nearest = 0;
  uint32_t wrap_mode;
  gen8_sampler_state_t *sampler;

  sampler = (gen8_sampler_state_t *)(gpgpu->aux_buf.bo->virtual + gpgpu->aux_offset.sampler_state_offset)  + index;
  memset(sampler, 0, sizeof(*sampler));
  assert((gpgpu->aux_buf.bo->offset + gpgpu->aux_offset.sampler_border_color_state_offset) % 32 == 0);
  if ((clk_sampler & __CLK_NORMALIZED_MASK) == CLK_NORMALIZED_COORDS_FALSE)
    sampler->ss3.non_normalized_coord = 1;
  else
    sampler->ss3.non_normalized_coord = 0;

  switch (clk_sampler & __CLK_FILTER_MASK) {
  case CLK_FILTER_NEAREST:
    sampler->ss0.min_filter = GEN_MAPFILTER_NEAREST;
    sampler->ss0.mip_filter = GEN_MIPFILTER_NONE;
    sampler->ss0.mag_filter = GEN_MAPFILTER_NEAREST;
    using_nearest = 1;
    break;
  case CLK_FILTER_LINEAR:
    sampler->ss0.min_filter = GEN_MAPFILTER_LINEAR;
    sampler->ss0.mip_filter = GEN_MIPFILTER_NONE;
    sampler->ss0.mag_filter = GEN_MAPFILTER_LINEAR;
    break;
  }

  wrap_mode = translate_wrap_mode(clk_sampler & __CLK_ADDRESS_MASK, using_nearest);
  sampler->ss3.s_wrap_mode = wrap_mode;
  /* XXX mesa i965 driver code point out that if the surface is a 1D surface, we may need
   * to set t_wrap_mode to GEN_TEXCOORDMODE_WRAP. */
  sampler->ss3.t_wrap_mode = wrap_mode;
  sampler->ss3.r_wrap_mode = wrap_mode;

  sampler->ss0.lod_preclamp = 1; /* OpenGL mode */
  sampler->ss0.default_color_mode = 0; /* OpenGL/DX10 mode */

  sampler->ss0.base_level = 0;

  sampler->ss1.max_lod = 0;
  sampler->ss1.min_lod = 0;

  if (sampler->ss0.min_filter != GEN_MAPFILTER_NEAREST)
     sampler->ss3.address_round |= GEN_ADDRESS_ROUNDING_ENABLE_U_MIN |
                                   GEN_ADDRESS_ROUNDING_ENABLE_V_MIN |
                                   GEN_ADDRESS_ROUNDING_ENABLE_R_MIN;
  if (sampler->ss0.mag_filter != GEN_MAPFILTER_NEAREST)
     sampler->ss3.address_round |= GEN_ADDRESS_ROUNDING_ENABLE_U_MAG |
                                   GEN_ADDRESS_ROUNDING_ENABLE_V_MAG |
                                   GEN_ADDRESS_ROUNDING_ENABLE_R_MAG;
}

static void
intel_gpgpu_bind_sampler_gen7(intel_gpgpu_t *gpgpu, uint32_t *samplers, size_t sampler_sz)
{
  int index;
  assert(sampler_sz <= GEN_MAX_SAMPLERS);
  for(index = 0; index < sampler_sz; index++)
    intel_gpgpu_insert_sampler_gen7(gpgpu, index, samplers[index]);
}

static void
intel_gpgpu_bind_sampler_gen8(intel_gpgpu_t *gpgpu, uint32_t *samplers, size_t sampler_sz)
{
  int index;
  assert(sampler_sz <= GEN_MAX_SAMPLERS);
  for(index = 0; index < sampler_sz; index++)
    intel_gpgpu_insert_sampler_gen8(gpgpu, index, samplers[index]);
}

static void
intel_gpgpu_states_setup(intel_gpgpu_t *gpgpu, cl_gpgpu_kernel *kernel)
{
  gpgpu->ker = kernel;
  intel_gpgpu_build_idrt(gpgpu, kernel);
  dri_bo_unmap(gpgpu->aux_buf.bo);
}

static void
intel_gpgpu_set_perf_counters(intel_gpgpu_t *gpgpu, cl_buffer *perf)
{
  if (gpgpu->perf_b.bo)
    drm_intel_bo_unreference(gpgpu->perf_b.bo);
  drm_intel_bo_reference((drm_intel_bo*) perf);
  gpgpu->perf_b.bo = (drm_intel_bo*) perf;
}

static void
intel_gpgpu_walker_gen7(intel_gpgpu_t *gpgpu,
                   uint32_t simd_sz,
                   uint32_t thread_n,
                   const size_t global_wk_off[3],
                   const size_t global_wk_sz[3],
                   const size_t local_wk_sz[3])
{
  const uint32_t global_wk_dim[3] = {
    global_wk_sz[0] / local_wk_sz[0],
    global_wk_sz[1] / local_wk_sz[1],
    global_wk_sz[2] / local_wk_sz[2]
  };
  uint32_t right_mask = ~0x0;
  size_t group_sz = local_wk_sz[0] * local_wk_sz[1] * local_wk_sz[2];

  assert(simd_sz == 8 || simd_sz == 16);

  uint32_t shift = (group_sz & (simd_sz - 1));
  shift = (shift == 0) ? simd_sz : shift;
  right_mask = (1 << shift) - 1;

  BEGIN_BATCH(gpgpu->batch, 11);
  OUT_BATCH(gpgpu->batch, CMD_GPGPU_WALKER | 9);
  OUT_BATCH(gpgpu->batch, 0);                        /* kernel index == 0 */
  assert(thread_n <= 64);
  if (simd_sz == 16)
    OUT_BATCH(gpgpu->batch, (1 << 30) | (thread_n-1)); /* SIMD16 | thread max */
  else
    OUT_BATCH(gpgpu->batch, (0 << 30) | (thread_n-1)); /* SIMD8  | thread max */
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[0]);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[1]);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[2]);
  OUT_BATCH(gpgpu->batch, right_mask);
  OUT_BATCH(gpgpu->batch, ~0x0);                     /* we always set height as 1, so set bottom mask as all 1*/
  ADVANCE_BATCH(gpgpu->batch);

  BEGIN_BATCH(gpgpu->batch, 2);
  OUT_BATCH(gpgpu->batch, CMD_MEDIA_STATE_FLUSH | 0);
  OUT_BATCH(gpgpu->batch, 0);                        /* kernel index == 0 */
  ADVANCE_BATCH(gpgpu->batch);
}

static void
intel_gpgpu_walker_gen8(intel_gpgpu_t *gpgpu,
                   uint32_t simd_sz,
                   uint32_t thread_n,
                   const size_t global_wk_off[3],
                   const size_t global_wk_sz[3],
                   const size_t local_wk_sz[3])
{
  const uint32_t global_wk_dim[3] = {
    global_wk_sz[0] / local_wk_sz[0],
    global_wk_sz[1] / local_wk_sz[1],
    global_wk_sz[2] / local_wk_sz[2]
  };
  uint32_t right_mask = ~0x0;
  size_t group_sz = local_wk_sz[0] * local_wk_sz[1] * local_wk_sz[2];

  assert(simd_sz == 8 || simd_sz == 16);

  uint32_t shift = (group_sz & (simd_sz - 1));
  shift = (shift == 0) ? simd_sz : shift;
  right_mask = (1 << shift) - 1;

  BEGIN_BATCH(gpgpu->batch, 15);
  OUT_BATCH(gpgpu->batch, CMD_GPGPU_WALKER | 13);
  OUT_BATCH(gpgpu->batch, 0);                        /* kernel index == 0 */
  OUT_BATCH(gpgpu->batch, 0);                        /* Indirect Data Length */
  OUT_BATCH(gpgpu->batch, 0);                        /* Indirect Data Start Address */
  assert(thread_n <= 64);
  if (simd_sz == 16)
    OUT_BATCH(gpgpu->batch, (1 << 30) | (thread_n-1)); /* SIMD16 | thread max */
  else
    OUT_BATCH(gpgpu->batch, (0 << 30) | (thread_n-1)); /* SIMD8  | thread max */
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[0]);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[1]);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[2]);
  OUT_BATCH(gpgpu->batch, right_mask);
  OUT_BATCH(gpgpu->batch, ~0x0);                     /* we always set height as 1, so set bottom mask as all 1*/
  ADVANCE_BATCH(gpgpu->batch);

  BEGIN_BATCH(gpgpu->batch, 2);
  OUT_BATCH(gpgpu->batch, CMD_MEDIA_STATE_FLUSH | 0);
  OUT_BATCH(gpgpu->batch, 0);                        /* kernel index == 0 */
  ADVANCE_BATCH(gpgpu->batch);
}

static intel_event_t*
intel_gpgpu_event_new(intel_gpgpu_t *gpgpu)
{
  intel_event_t *event = NULL;
  TRY_ALLOC_NO_ERR (event, CALLOC(intel_event_t));

  event->buffer = gpgpu->batch->buffer;
  if (event->buffer)
    drm_intel_bo_reference(event->buffer);
  event->status = command_queued;

  if(gpgpu->time_stamp_b.bo) {
    event->ts_buf = gpgpu->time_stamp_b.bo;
    drm_intel_bo_reference(event->ts_buf);
  }

exit:
  return event;
error:
  cl_free(event);
  event = NULL;
  goto exit;
}

/*
   The upper layer already flushed the batch buffer, just update
   internal status to command_submitted.
*/
static void
intel_gpgpu_event_flush(intel_event_t *event)
{
  assert(event->status == command_queued);
  event->status = command_running;
}

static int
intel_gpgpu_event_update_status(intel_event_t *event, int wait)
{
  if(event->status == command_complete)
    return event->status;

  if (event->buffer &&
      event->status == command_running &&
      !drm_intel_bo_busy(event->buffer)) {
    event->status = command_complete;
    drm_intel_bo_unreference(event->buffer);
    event->buffer = NULL;
    return event->status;
  }

  if(wait == 0)
    return event->status;

  if (event->buffer) {
    drm_intel_bo_wait_rendering(event->buffer);
    event->status = command_complete;
    drm_intel_bo_unreference(event->buffer);
    event->buffer = NULL;
  }
  return event->status;
}

static void
intel_gpgpu_event_delete(intel_event_t *event)
{
  if(event->buffer)
    drm_intel_bo_unreference(event->buffer);
  if(event->ts_buf)
    drm_intel_bo_unreference(event->ts_buf);
  cl_free(event);
}

/* IVB and HSW's result MUST shift in x86_64 system */
static uint64_t
intel_gpgpu_read_ts_reg_gen7(drm_intel_bufmgr *bufmgr)
{
  uint64_t result = 0;
  drm_intel_reg_read(bufmgr, TIMESTAMP_ADDR, &result);
  /* In x86_64 system, the low 32bits of timestamp count are stored in the high 32 bits of
     result which got from drm_intel_reg_read, and 32-35 bits are lost; but match bspec in
     i386 system. It seems the kernel readq bug. So shift 32 bit in x86_64, and only remain
     32 bits data in i386.
  */
#ifdef __i386__
  return result & 0x0ffffffff;
#else
  return result >> 32;
#endif  /* __i386__  */
}

/* baytrail's result should clear high 4 bits */
static uint64_t
intel_gpgpu_read_ts_reg_baytrail(drm_intel_bufmgr *bufmgr)
{
  uint64_t result = 0;
  drm_intel_reg_read(bufmgr, TIMESTAMP_ADDR, &result);
  return result & 0x0ffffffff;
}

/* We want to get the current time of GPU. */
static void
intel_gpgpu_event_get_gpu_cur_timestamp(intel_gpgpu_t* gpgpu, uint64_t* ret_ts)
{
  uint64_t result = 0;
  drm_intel_bufmgr *bufmgr = gpgpu->drv->bufmgr;

  /* Get the ts that match the bspec */
  result = intel_gpgpu_read_ts_reg(bufmgr);
  result *= 80;

  *ret_ts = result;
  return;
}

/* Get the GPU execute time. */
static void
intel_gpgpu_event_get_exec_timestamp(intel_gpgpu_t* gpgpu, intel_event_t *event,
				     int index, uint64_t* ret_ts)
{
  uint64_t result = 0;

  assert(event->ts_buf != NULL);
  assert(index == 0 || index == 1);
  drm_intel_gem_bo_map_gtt(event->ts_buf);
  uint64_t* ptr = event->ts_buf->virtual;
  result = ptr[index];

  /* According to BSpec, the timestamp counter should be 36 bits,
     but comparing to the timestamp counter from IO control reading,
     we find the first 4 bits seems to be fake. In order to keep the
     timestamp counter conformable, we just skip the first 4 bits.
  */
  result = (result & 0x0FFFFFFFF) * 80; //convert to nanoseconds
  *ret_ts = result;

  drm_intel_gem_bo_unmap_gtt(event->ts_buf);
}

static int
intel_gpgpu_set_printf_buf(intel_gpgpu_t *gpgpu, uint32_t i, uint32_t size, uint32_t offset, uint8_t bti)
{
  drm_intel_bo *bo = NULL;
  if (i == 0) { // the index buffer.
    if (gpgpu->printf_b.ibo)
      dri_bo_unreference(gpgpu->printf_b.ibo);
    gpgpu->printf_b.ibo = dri_bo_alloc(gpgpu->drv->bufmgr, "Printf index buffer", size, 4096);
    bo = gpgpu->printf_b.ibo;
  } else if (i == 1) {
    if (gpgpu->printf_b.bo)
      dri_bo_unreference(gpgpu->printf_b.bo);
    gpgpu->printf_b.bo = dri_bo_alloc(gpgpu->drv->bufmgr, "Printf output buffer", size, 4096);
    bo = gpgpu->printf_b.bo;
  } else
    assert(0);

  if (!bo || (drm_intel_bo_map(bo, 1) != 0)) {
    if (gpgpu->printf_b.bo)
      drm_intel_bo_unreference(gpgpu->printf_b.bo);
    gpgpu->printf_b.bo = NULL;
    fprintf(stderr, "%s:%d: %s.\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  }
  memset(bo->virtual, 0, size);
  drm_intel_bo_unmap(bo);
  cl_gpgpu_bind_buf((cl_gpgpu)gpgpu, (cl_buffer)bo, offset, 0, size, bti);
  return 0;
}

static void*
intel_gpgpu_map_printf_buf(intel_gpgpu_t *gpgpu, uint32_t i)
{
  drm_intel_bo *bo = NULL;
  if (i == 0) {
    bo = gpgpu->printf_b.ibo;
  } else if (i == 1) {
    bo = gpgpu->printf_b.bo;
  } else
    assert(0);

  drm_intel_bo_map(bo, 1);
  return bo->virtual;
}

static void
intel_gpgpu_unmap_printf_buf_addr(intel_gpgpu_t *gpgpu, uint32_t i)
{
  drm_intel_bo *bo = NULL;
  if (i == 0) {
    bo = gpgpu->printf_b.ibo;
  } else if (i == 1) {
    bo = gpgpu->printf_b.bo;
  } else
  assert(0);

  drm_intel_bo_unmap(bo);
}

static void
intel_gpgpu_release_printf_buf(intel_gpgpu_t *gpgpu, uint32_t i)
{
  if (i == 0) {
    drm_intel_bo_unreference(gpgpu->printf_b.ibo);
    gpgpu->printf_b.ibo = NULL;
  } else if (i == 1) {
    drm_intel_bo_unreference(gpgpu->printf_b.bo);
    gpgpu->printf_b.bo = NULL;
  } else
    assert(0);
}

static void
intel_gpgpu_set_printf_info(intel_gpgpu_t *gpgpu, void* printf_info, size_t * global_sz)
{
  gpgpu->printf_info = printf_info;
  gpgpu->global_wk_sz[0] = global_sz[0];
  gpgpu->global_wk_sz[1] = global_sz[1];
  gpgpu->global_wk_sz[2] = global_sz[2];
}

static void*
intel_gpgpu_get_printf_info(intel_gpgpu_t *gpgpu, size_t * global_sz)
{
  global_sz[0] = gpgpu->global_wk_sz[0];
  global_sz[1] = gpgpu->global_wk_sz[1];
  global_sz[2] = gpgpu->global_wk_sz[2];
  return gpgpu->printf_info;
}

LOCAL void
intel_set_gpgpu_callbacks(int device_id)
{
  cl_gpgpu_new = (cl_gpgpu_new_cb *) intel_gpgpu_new;
  cl_gpgpu_delete = (cl_gpgpu_delete_cb *) intel_gpgpu_delete;
  cl_gpgpu_sync = (cl_gpgpu_sync_cb *) intel_gpgpu_sync;
  cl_gpgpu_bind_buf = (cl_gpgpu_bind_buf_cb *) intel_gpgpu_bind_buf;
  cl_gpgpu_set_stack = (cl_gpgpu_set_stack_cb *) intel_gpgpu_set_stack;
  cl_gpgpu_state_init = (cl_gpgpu_state_init_cb *) intel_gpgpu_state_init;
  cl_gpgpu_set_perf_counters = (cl_gpgpu_set_perf_counters_cb *) intel_gpgpu_set_perf_counters;
  cl_gpgpu_upload_curbes = (cl_gpgpu_upload_curbes_cb *) intel_gpgpu_upload_curbes;
  cl_gpgpu_alloc_constant_buffer  = (cl_gpgpu_alloc_constant_buffer_cb *) intel_gpgpu_alloc_constant_buffer;
  cl_gpgpu_states_setup = (cl_gpgpu_states_setup_cb *) intel_gpgpu_states_setup;
  cl_gpgpu_upload_samplers = (cl_gpgpu_upload_samplers_cb *) intel_gpgpu_upload_samplers;
  cl_gpgpu_batch_reset = (cl_gpgpu_batch_reset_cb *) intel_gpgpu_batch_reset;
  cl_gpgpu_batch_start = (cl_gpgpu_batch_start_cb *) intel_gpgpu_batch_start;
  cl_gpgpu_batch_end = (cl_gpgpu_batch_end_cb *) intel_gpgpu_batch_end;
  cl_gpgpu_flush = (cl_gpgpu_flush_cb *) intel_gpgpu_flush;
  cl_gpgpu_bind_sampler = (cl_gpgpu_bind_sampler_cb *) intel_gpgpu_bind_sampler_gen7;
  cl_gpgpu_set_scratch = (cl_gpgpu_set_scratch_cb *) intel_gpgpu_set_scratch;
  cl_gpgpu_event_new = (cl_gpgpu_event_new_cb *)intel_gpgpu_event_new;
  cl_gpgpu_event_flush = (cl_gpgpu_event_flush_cb *)intel_gpgpu_event_flush;
  cl_gpgpu_event_update_status = (cl_gpgpu_event_update_status_cb *)intel_gpgpu_event_update_status;
  cl_gpgpu_event_delete = (cl_gpgpu_event_delete_cb *)intel_gpgpu_event_delete;
  cl_gpgpu_event_get_exec_timestamp = (cl_gpgpu_event_get_exec_timestamp_cb *)intel_gpgpu_event_get_exec_timestamp;
  cl_gpgpu_event_get_gpu_cur_timestamp = (cl_gpgpu_event_get_gpu_cur_timestamp_cb *)intel_gpgpu_event_get_gpu_cur_timestamp;
  cl_gpgpu_ref_batch_buf = (cl_gpgpu_ref_batch_buf_cb *)intel_gpgpu_ref_batch_buf;
  cl_gpgpu_unref_batch_buf = (cl_gpgpu_unref_batch_buf_cb *)intel_gpgpu_unref_batch_buf;
  cl_gpgpu_set_printf_buffer = (cl_gpgpu_set_printf_buffer_cb *)intel_gpgpu_set_printf_buf;
  cl_gpgpu_map_printf_buffer = (cl_gpgpu_map_printf_buffer_cb *)intel_gpgpu_map_printf_buf;
  cl_gpgpu_unmap_printf_buffer = (cl_gpgpu_unmap_printf_buffer_cb *)intel_gpgpu_unmap_printf_buf_addr;
  cl_gpgpu_release_printf_buffer = (cl_gpgpu_release_printf_buffer_cb *)intel_gpgpu_release_printf_buf;
  cl_gpgpu_set_printf_info = (cl_gpgpu_set_printf_info_cb *)intel_gpgpu_set_printf_info;
  cl_gpgpu_get_printf_info = (cl_gpgpu_get_printf_info_cb *)intel_gpgpu_get_printf_info;

  if (IS_BROADWELL(device_id)) {
    cl_gpgpu_bind_image = (cl_gpgpu_bind_image_cb *) intel_gpgpu_bind_image_gen8;
    intel_gpgpu_set_L3 = intel_gpgpu_set_L3_gen8;
    cl_gpgpu_get_cache_ctrl = (cl_gpgpu_get_cache_ctrl_cb *)intel_gpgpu_get_cache_ctrl_gen8;
    intel_gpgpu_get_scratch_index = intel_gpgpu_get_scratch_index_gen8;
    intel_gpgpu_post_action = intel_gpgpu_post_action_gen7; //BDW need not restore SLM, same as gen7
    intel_gpgpu_read_ts_reg = intel_gpgpu_read_ts_reg_gen7;
    intel_gpgpu_set_base_address = intel_gpgpu_set_base_address_gen8;
    intel_gpgpu_setup_bti = intel_gpgpu_setup_bti_gen8;
    intel_gpgpu_load_vfe_state = intel_gpgpu_load_vfe_state_gen8;
    cl_gpgpu_walker = (cl_gpgpu_walker_cb *)intel_gpgpu_walker_gen8;
    intel_gpgpu_build_idrt = intel_gpgpu_build_idrt_gen8;
    intel_gpgpu_load_curbe_buffer = intel_gpgpu_load_curbe_buffer_gen8;
    intel_gpgpu_load_idrt = intel_gpgpu_load_idrt_gen8;
    cl_gpgpu_bind_sampler = (cl_gpgpu_bind_sampler_cb *) intel_gpgpu_bind_sampler_gen8;
    return;
  }

  intel_gpgpu_set_base_address = intel_gpgpu_set_base_address_gen7;
  intel_gpgpu_load_vfe_state = intel_gpgpu_load_vfe_state_gen7;
  cl_gpgpu_walker = (cl_gpgpu_walker_cb *)intel_gpgpu_walker_gen7;
  intel_gpgpu_build_idrt = intel_gpgpu_build_idrt_gen7;
  intel_gpgpu_load_curbe_buffer = intel_gpgpu_load_curbe_buffer_gen7;
  intel_gpgpu_load_idrt = intel_gpgpu_load_idrt_gen7;

  if (IS_HASWELL(device_id)) {
    cl_gpgpu_bind_image = (cl_gpgpu_bind_image_cb *) intel_gpgpu_bind_image_gen75;
    intel_gpgpu_set_L3 = intel_gpgpu_set_L3_gen75;
    cl_gpgpu_get_cache_ctrl = (cl_gpgpu_get_cache_ctrl_cb *)intel_gpgpu_get_cache_ctrl_gen75;
    intel_gpgpu_get_scratch_index = intel_gpgpu_get_scratch_index_gen75;
    intel_gpgpu_post_action = intel_gpgpu_post_action_gen75;
    intel_gpgpu_read_ts_reg = intel_gpgpu_read_ts_reg_gen7; //HSW same as ivb
    intel_gpgpu_setup_bti = intel_gpgpu_setup_bti_gen75;
  }
  else if (IS_IVYBRIDGE(device_id)) {
    cl_gpgpu_bind_image = (cl_gpgpu_bind_image_cb *) intel_gpgpu_bind_image_gen7;
    if (IS_BAYTRAIL_T(device_id)) {
      intel_gpgpu_set_L3 = intel_gpgpu_set_L3_baytrail;
      intel_gpgpu_read_ts_reg = intel_gpgpu_read_ts_reg_baytrail;
    } else {
      intel_gpgpu_set_L3 = intel_gpgpu_set_L3_gen7;
      intel_gpgpu_read_ts_reg = intel_gpgpu_read_ts_reg_gen7;
    }
    cl_gpgpu_get_cache_ctrl = (cl_gpgpu_get_cache_ctrl_cb *)intel_gpgpu_get_cache_ctrl_gen7;
    intel_gpgpu_get_scratch_index = intel_gpgpu_get_scratch_index_gen7;
    intel_gpgpu_post_action = intel_gpgpu_post_action_gen7;
    intel_gpgpu_setup_bti = intel_gpgpu_setup_bti_gen7;
  }
}
