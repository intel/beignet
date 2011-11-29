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

#include "intel/intel_gpgpu.h"
#include "intel/intel_defines.h"
#include "intel/intel_structs.h"
#include "intel/intel_batchbuffer.h"
#include "intel/intel_driver.h"

#include "cl_alloc.h"
#include "cl_utils.h"

#define GEN_CMD_MEDIA_OBJECT  (0x71000000)
#define MO_TS_BIT             (1 << 24)
#define MO_RETAIN_BIT         (1 << 28)
#define SAMPLER_STATE_SIZE    (16)

/* Stores both binding tables and surface states */
typedef struct surface_heap {
  uint32_t binding_table[256];
  char surface[256][sizeof(gen6_surface_state_t)];
} surface_heap_t;

#define MAX_IF_DESC    32

/* Device abstraction */
struct intel_driver;

/* Handle GPGPU state (actually "media" state) */
struct intel_gpgpu
{
  intel_driver_t *drv;
  intel_batchbuffer_t *batch;
  genx_gpgpu_kernel_t *ker;

  struct {
    dri_bo *bo;
    uint32_t num;
  } idrt_b;
  struct { dri_bo *bo; } surface_heap_b;
  struct { dri_bo *bo; } vfe_state_b;
  struct { dri_bo *bo; } curbe_b;
  struct { dri_bo *bo; } sampler_state_b;
  struct { dri_bo *bo; } perf_b;

  struct {
    uint32_t num_cs_entries;
    uint32_t size_cs_entry;  /* size of one entry in 512bit elements */
  } urb;

  uint32_t max_threads;      /* max threads requested by the user */
};

LOCAL intel_gpgpu_t*
intel_gpgpu_new(intel_driver_t *drv)
{
  intel_gpgpu_t *state = NULL;

  TRY_ALLOC_NO_ERR (state, CALLOC(intel_gpgpu_t));
  state->drv = drv;
  state->batch = intel_batchbuffer_new(state->drv);
  assert(state->batch);
  intel_batchbuffer_init(state->batch, state->drv);

exit:
  return state;
error:
  intel_gpgpu_delete(state);
  state = NULL;
  goto exit;
}

LOCAL void
intel_gpgpu_delete(intel_gpgpu_t *state)
{
  if (state == NULL)
    return;
  if (state->surface_heap_b.bo)
    drm_intel_bo_unreference(state->surface_heap_b.bo);
  if (state->idrt_b.bo)
    drm_intel_bo_unreference(state->idrt_b.bo);
  if (state->vfe_state_b.bo)
    drm_intel_bo_unreference(state->vfe_state_b.bo);
  if (state->curbe_b.bo)
    drm_intel_bo_unreference(state->curbe_b.bo);
  if (state->sampler_state_b.bo)
    drm_intel_bo_unreference(state->sampler_state_b.bo);
  if (state->perf_b.bo)
    drm_intel_bo_unreference(state->perf_b.bo);
  intel_batchbuffer_delete(state->batch);
  cl_free(state);
}

static void
gpgpu_select_pipeline(intel_gpgpu_t *state)
{
  BEGIN_BATCH(state->batch, 1);
  OUT_BATCH(state->batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA);
  ADVANCE_BATCH(state->batch);
}

static void
gpgpu_set_base_address(intel_gpgpu_t *state)
{
  const uint32_t def_cc = cc_llc_l3; /* default Cache Control value */
  BEGIN_BATCH(state->batch, 10);
  OUT_BATCH(state->batch, CMD_STATE_BASE_ADDRESS | 8);
  /* 0, Gen State Mem Obj CC, Stateless Mem Obj CC, Stateless Access Write Back */
  OUT_BATCH(state->batch, 0 | (def_cc << 8) | (def_cc << 4) | (0 << 3)| BASE_ADDRESS_MODIFY);    /* General State Base Addr   */
  /* 0, State Mem Obj CC */
  /* We use a state base address for the surface heap since IVB clamp the
   * binding table pointer at 11 bits. So, we cannot use pointers directly while
   * using the surface heap
   */
  OUT_RELOC(state->batch, state->surface_heap_b.bo,
            I915_GEM_DOMAIN_INSTRUCTION,
            I915_GEM_DOMAIN_INSTRUCTION,
            0 | (def_cc << 8) | (def_cc << 4) | (0 << 3)| BASE_ADDRESS_MODIFY);
  OUT_BATCH(state->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Dynamic State Base Addr */
  OUT_BATCH(state->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Indirect Obj Base Addr */
  OUT_BATCH(state->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Instruction Base Addr  */
  /* If we output an AUB file, we limit the total size to 64MB */
#if USE_FULSIM
  OUT_BATCH(state->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* General State Access Upper Bound */
  OUT_BATCH(state->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Dynamic State Access Upper Bound */
  OUT_BATCH(state->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Indirect Obj Access Upper Bound */
  OUT_BATCH(state->batch, 0x04000000 | BASE_ADDRESS_MODIFY); /* Instruction Access Upper Bound */
#else
  OUT_BATCH(state->batch, 0 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(state->batch, 0 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(state->batch, 0 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(state->batch, 0 | BASE_ADDRESS_MODIFY);
#endif
  ADVANCE_BATCH(state->batch);
}

static void
gpgpu_load_vfe_state(intel_gpgpu_t *state)
{
  BEGIN_BATCH(state->batch, 8);
  OUT_BATCH(state->batch, CMD_MEDIA_STATE_POINTERS | (8-2));

  gen6_vfe_state_inline_t* vfe = (gen6_vfe_state_inline_t*)
    intel_batchbuffer_alloc_space(state->batch,0);

  memset(vfe, 0, sizeof(struct gen6_vfe_state_inline));
  vfe->vfe1.gpgpu_mode =
    (state->drv->gen_ver == 7 || state->drv->gen_ver == 75) ? 1 : 0;
  vfe->vfe1.bypass_gateway_ctl = 1;
  vfe->vfe1.reset_gateway_timer = 1;
  vfe->vfe1.max_threads = state->max_threads - 1;
  vfe->vfe1.urb_entries = 64;
  vfe->vfe3.curbe_size = 63;
  vfe->vfe3.urbe_size = 13;
  vfe->vfe4.scoreboard_mask =
    (state->drv->gen_ver == 7 || state->drv->gen_ver == 75) ? 0 : 0x80000000;
  intel_batchbuffer_alloc_space(state->batch, sizeof(gen6_vfe_state_inline_t));
  ADVANCE_BATCH(state->batch);
}

static void
gpgpu_load_constant_buffer(intel_gpgpu_t *state) 
{
  BEGIN_BATCH(state->batch, 4);
  OUT_BATCH(state->batch, CMD(2,0,1) | (4 - 2));  /* length-2 */
  OUT_BATCH(state->batch, 0);                     /* mbz */
  OUT_BATCH(state->batch,
            state->urb.size_cs_entry*
            state->urb.num_cs_entries*32);
  OUT_RELOC(state->batch, state->curbe_b.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
  ADVANCE_BATCH(state->batch);
}

static void
gpgpu_load_idrt(intel_gpgpu_t *state) 
{
  BEGIN_BATCH(state->batch, 4);
  OUT_BATCH(state->batch, CMD(2,0,2) | (4 - 2)); /* length-2 */
  OUT_BATCH(state->batch, 0);                    /* mbz */
  OUT_BATCH(state->batch, state->idrt_b.num*32);
  OUT_RELOC(state->batch, state->idrt_b.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
  ADVANCE_BATCH(state->batch);
}

static const uint32_t gpgpu_l3_config_reg1[] =
{
              // SLM    URB     DC      RO      I/S     C       T
  0x00080040, //{ 0,    256,    0,      256,    0,      0,      0,      }
  0x02040040, //{ 0,    256,    128,    128,    0,      0,      0,      }
  0x00800040, //{ 0,    256,    32,     0,      64,     32,     128,    }
  0x01000038, //{ 0,    224,    64,     0,      64,     32,     128,    }
  0x02000030, //{ 0,    224,    128,    0,      64,     32,     64,     }
  0x01000038, //{ 0,    224,    64,     0,      128,    32,     64,     }
  0x00000038, //{ 0,    224,    0,      0,      128,    32,     128,    }
  0x00000040, //{ 0,    256,    0,      0,      128,    0,      128,    }
  0x0A140091, //{ 128,  128,    128,    128,    0,      0,      0,      }
  0x09100091, //{ 128,  128,    64,     0,      64,     64,     64,     }
  0x08900091, //{ 128,  128,    32,     0,      64,     32,     128,    }
  0x08900091  //{ 128,  128,    32,     0,      128,    32,     64,     }
};

static const uint32_t gpgpu_l3_config_reg2[] =
{
              // SLM    URB     DC      RO      I/S     C       T
  0x00000000, //{ 0,    256,    0,      256,    0,      0,      0,      }
  0x00000000, //{ 0,    256,    128,    128,    0,      0,      0,      }
  0x00080410, //{ 0,    256,    32,     0,      64,     32,     128,    }
  0x00080410, //{ 0,    224,    64,     0,      64,     32,     128,    }
  0x00040410, //{ 0,    224,    128,    0,      64,     32,     64,     }
  0x00040420, //{ 0,    224,    64,     0,      128,    32,     64,     }
  0x00080420, //{ 0,    224,    0,      0,      128,    32,     128,    }
  0x00080020, //{ 0,    256,    0,      0,      128,    0,      128,    }
  0x00204080, //{ 128,  128,    128,    128,    0,      0,      0,      }
  0x00244890, //{ 128,  128,    64,     0,      64,     64,     64,     }
  0x00284490, //{ 128,  128,    32,     0,      64,     32,     128,    }
  0x002444A0  //{ 128,  128,    32,     0,      128,    32,     64,     }
};

// L3 cache stuff 
#define L3_CNTL_REG2_ADDRESS_OFFSET         (0xB020)
#define L3_CNTL_REG3_ADDRESS_OFFSET         (0xB024)

enum INSTRUCTION_PIPELINE
{
  PIPE_COMMON       = 0x0,
  PIPE_SINGLE_DWORD = 0x1,
  PIPE_COMMON_CTG   = 0x1,
  PIPE_MEDIA        = 0x2,
  PIPE_3D           = 0x3
};

enum GFX_OPCODE
{
  GFXOP_PIPELINED     = 0x0,
  GFXOP_NONPIPELINED  = 0x1,
  GFXOP_3DPRIMITIVE   = 0x3
};

enum INSTRUCTION_TYPE
{
  INSTRUCTION_MI      = 0x0,
  INSTRUCTION_TRUSTED = 0x1,
  INSTRUCTION_2D      = 0x2,
  INSTRUCTION_GFX     = 0x3
};

enum GFX3DCONTROL_SUBOPCODE
{
  GFX3DSUBOP_3DCONTROL    = 0x00
};

enum GFX3D_OPCODE
{
  GFX3DOP_3DSTATE_PIPELINED       = 0x0,
  GFX3DOP_3DSTATE_NONPIPELINED    = 0x1,
  GFX3DOP_3DCONTROL               = 0x2,
  GFX3DOP_3DPRIMITIVE             = 0x3
};

enum GFX3DSTATE_PIPELINED_SUBOPCODE
{
  GFX3DSUBOP_3DSTATE_PIPELINED_POINTERS       = 0x00,
  GFX3DSUBOP_3DSTATE_BINDING_TABLE_POINTERS   = 0x01,
  GFX3DSUBOP_3DSTATE_STATE_POINTER_INVALIDATE = 0x02,
  GFX3DSUBOP_3DSTATE_VERTEX_BUFFERS           = 0x08,
  GFX3DSUBOP_3DSTATE_VERTEX_ELEMENTS          = 0x09,
  GFX3DSUBOP_3DSTATE_INDEX_BUFFER             = 0x0A,
  GFX3DSUBOP_3DSTATE_VF_STATISTICS            = 0x0B,
  GFX3DSUBOP_3DSTATE_CC_STATE_POINTERS        = 0x0E
};

static void
gpgpu_pipe_control(intel_gpgpu_t *state)
{
  BEGIN_BATCH(state->batch, SIZEOF32(gen6_pipe_control_t));
  gen6_pipe_control_t* pc = (gen6_pipe_control_t*)
    intel_batchbuffer_alloc_space(state->batch, 0);
  memset(pc, 0, sizeof(*pc));
  pc->dw0.length = SIZEOF32(gen6_pipe_control_t) - 2;
  pc->dw0.instruction_subopcode = GFX3DSUBOP_3DCONTROL;
  pc->dw0.instruction_opcode = GFX3DOP_3DCONTROL;
  pc->dw0.instruction_pipeline = PIPE_3D;
  pc->dw0.instruction_type = INSTRUCTION_GFX;
  pc->dw1.render_target_cache_flush_enable = 1;
  pc->dw1.cs_stall = 1;
  pc->dw1.dc_flush_enable = 1;
  ADVANCE_BATCH(state->batch);
}

LOCAL void
intel_gpgpu_set_L3(intel_gpgpu_t *state, uint32_t use_barrier)
{
  BEGIN_BATCH(state->batch, 6);
  OUT_BATCH(state->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(state->batch, L3_CNTL_REG2_ADDRESS_OFFSET);
  if (use_barrier)
    OUT_BATCH(state->batch, gpgpu_l3_config_reg1[8]);
  else
    OUT_BATCH(state->batch, gpgpu_l3_config_reg1[4]);

  OUT_BATCH(state->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(state->batch, L3_CNTL_REG3_ADDRESS_OFFSET);
  if (use_barrier)
    OUT_BATCH(state->batch, gpgpu_l3_config_reg2[8]);
  else
    OUT_BATCH(state->batch, gpgpu_l3_config_reg2[4]);
  ADVANCE_BATCH(state->batch);

  gpgpu_pipe_control(state);
}

LOCAL void
gpgpu_batch_start(intel_gpgpu_t *state)
{
  intel_batchbuffer_start_atomic(state->batch, 256);
  gpgpu_pipe_control(state);
  if (state->drv->gen_ver == 7 || state->drv->gen_ver == 75)
    intel_gpgpu_set_L3(state, state->ker->use_barrier);
  gpgpu_select_pipeline(state);
  gpgpu_set_base_address(state);
  gpgpu_load_vfe_state(state);
  gpgpu_load_constant_buffer(state);
  gpgpu_load_idrt(state);

  if (state->perf_b.bo) {
    BEGIN_BATCH(state->batch, 3);
    OUT_BATCH(state->batch,
              (0x28 << 23) | /* MI_REPORT_PERF_COUNT */
              (3 - 2));      /* length-2 */
    OUT_RELOC(state->batch, state->perf_b.bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              0 |  /* Offset for the start "counters" */
              1);  /* Use GTT and not PGTT */
    OUT_BATCH(state->batch, 0);
    ADVANCE_BATCH(state->batch);
  }
}

LOCAL void
gpgpu_batch_end(intel_gpgpu_t *state, int32_t flush_mode)
{
  /* Insert the performance counter command */
  if (state->perf_b.bo) {
    BEGIN_BATCH(state->batch, 3);
    OUT_BATCH(state->batch,
              (0x28 << 23) | /* MI_REPORT_PERF_COUNT */
              (3 - 2));      /* length-2 */
    OUT_RELOC(state->batch, state->perf_b.bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              512 |  /* Offset for the end "counters" */
              1);    /* Use GTT and not PGTT */
    OUT_BATCH(state->batch, 0);
    ADVANCE_BATCH(state->batch);
  }

  if(flush_mode) gpgpu_pipe_control(state);
  intel_batchbuffer_end_atomic(state->batch);
}

LOCAL void
gpgpu_batch_reset(intel_gpgpu_t *state, size_t sz)
{
  intel_batchbuffer_reset(state->batch, sz);
}

LOCAL void
gpgpu_flush(intel_gpgpu_t *state)
{
  intel_batchbuffer_flush(state->batch);
}

LOCAL void
gpgpu_state_init(intel_gpgpu_t *state,
                 uint32_t max_threads,
                 uint32_t size_cs_entry)
{
  dri_bo *bo;

  /* URB */
  state->urb.num_cs_entries = 64;
  state->urb.size_cs_entry = size_cs_entry;
  state->max_threads = max_threads;

  /* constant buffer */
  if(state->curbe_b.bo)
    dri_bo_unreference(state->curbe_b.bo);
  uint32_t size_cb = state->urb.num_cs_entries * state->urb.size_cs_entry * 64;
  size_cb = ALIGN(size_cb, 4096);
  bo = dri_bo_alloc(state->drv->bufmgr,
                    "CONSTANT_BUFFER",
                    size_cb,
                    64);
  assert(bo);
  state->curbe_b.bo = bo;

  /* surface state */
  if(state->surface_heap_b.bo)
    dri_bo_unreference(state->surface_heap_b.bo);
  bo = dri_bo_alloc(state->drv->bufmgr, 
                    "interface descriptor", 
                    sizeof(surface_heap_t),
                    32);
  assert(bo);
  dri_bo_map(bo, 1);
  memset(bo->virtual, 0, sizeof(surface_heap_t));
  state->surface_heap_b.bo = bo;

  /* Interface descriptor remap table */
  if(state->idrt_b.bo)
    dri_bo_unreference(state->idrt_b.bo);
  bo = dri_bo_alloc(state->drv->bufmgr, 
                    "interface discriptor", 
                    MAX_IF_DESC * sizeof(struct gen6_interface_descriptor),
                    32);
  assert(bo);
  state->idrt_b.bo = bo;

  /* vfe state */
  if(state->vfe_state_b.bo)
    dri_bo_unreference(state->vfe_state_b.bo);
  state->vfe_state_b.bo = NULL;

  /* sampler state */
  if (state->sampler_state_b.bo)
    dri_bo_unreference(state->sampler_state_b.bo);
  bo = dri_bo_alloc(state->drv->bufmgr, 
                    "sample states",
                    MAX_SAMPLERS * sizeof(gen6_sampler_state_t),
                    32);
  assert(bo);
  dri_bo_map(bo, 1);
  memset(bo->virtual, 0, sizeof(gen6_sampler_state_t) * MAX_SAMPLERS);
  state->sampler_state_b.bo = bo;
}

static void
gpgpu_set_buf_reloc_gen6(intel_gpgpu_t *state, int32_t index, dri_bo* obj_bo)
{
  surface_heap_t *heap = state->surface_heap_b.bo->virtual;
  heap->binding_table[index] = offsetof(surface_heap_t, surface) +
                               index * sizeof(gen6_surface_state_t);
  dri_bo_emit_reloc(state->surface_heap_b.bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    0,
                    heap->binding_table[index] +
                    offsetof(gen6_surface_state_t, ss1),
                    obj_bo);
}

static void
gpgpu_set_buf_reloc_gen7(intel_gpgpu_t *state, int32_t index, dri_bo* obj_bo)
{
  surface_heap_t *heap = state->surface_heap_b.bo->virtual;
  heap->binding_table[index] = offsetof(surface_heap_t, surface) +
                               index * sizeof(gen7_surface_state_t);
  dri_bo_emit_reloc(state->surface_heap_b.bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    0,
                    heap->binding_table[index] +
                    offsetof(gen7_surface_state_t, ss1),
                    obj_bo);
}

static void
gpgpu_bind_buf_gen6(intel_gpgpu_t *state,
                    int32_t index,
                    dri_bo* obj_bo,
                    uint32_t size,
                    uint32_t cchint)
{
  surface_heap_t *heap = state->surface_heap_b.bo->virtual;
  gen6_surface_state_t *ss = (gen6_surface_state_t *) heap->surface[index];
  const uint32_t size_ss = ((size+0xf) >> 4) - 1; /* ceil(size/16) - 1 */
  memset(ss, 0, sizeof(*ss));
  ss->ss0.surface_type = I965_SURFACE_BUFFER;
  ss->ss0.surface_format = I965_SURFACEFORMAT_R32G32B32A32_FLOAT;
  ss->ss1.base_addr = obj_bo->offset;
  ss->ss2.width = size_ss & 0x7f;           /* bits 6:0 of size_ss */
  ss->ss2.height = (size_ss >> 7) & 0x1fff; /* bits 19:7 of size_ss */
  ss->ss3.depth = size_ss >> 20;            /* bits 26:20 of size_ss */
  ss->ss3.pitch = 0xf;                      /* sizeof(RGBA32) - 1 */;
  ss->ss5.cache_control = cchint;
  gpgpu_set_buf_reloc_gen6(state, index, obj_bo);
}

static void
gpgpu_bind_buf_gen7(intel_gpgpu_t *state,
                    int32_t index,
                    dri_bo* obj_bo,
                    uint32_t size,
                    uint32_t cchint)
{
  surface_heap_t *heap = state->surface_heap_b.bo->virtual;
  gen7_surface_state_t *ss = (gen7_surface_state_t *) heap->surface[index];
  const uint32_t size_ss = size - 1;
  memset(ss, 0, sizeof(*ss));
  ss->ss0.surface_type = I965_SURFACE_BUFFER;
  ss->ss0.surface_format = I965_SURFACEFORMAT_RAW;
  ss->ss1.base_addr = obj_bo->offset;
  ss->ss2.width  = size_ss & 0x7f;               /* bits 6:0 of size_ss */
  ss->ss2.height = (size_ss & 0x1fff80) >> 7;    /* bits 20:7 of size_ss */
  ss->ss3.depth  = (size_ss & 0xffe00000) >> 20; /* bits 27:21 of size_ss */
  ss->ss5.cache_control = cc_llc_l3;
  gpgpu_set_buf_reloc_gen7(state, index, obj_bo);
}

static void
gpgpu_bind_image2D_gen6(intel_gpgpu_t *state,
                        int32_t index,
                        dri_bo* obj_bo,
                        uint32_t format,
                        int32_t w,
                        int32_t h,
                        int bpp,
                        uint32_t cchint)
{
  surface_heap_t *heap = state->surface_heap_b.bo->virtual;
  gen6_surface_state_t *ss = (gen6_surface_state_t *) heap->surface[index];
  memset(ss, 0, sizeof(*ss));
  ss->ss0.surface_type = I965_SURFACE_2D;
  ss->ss0.surface_format = format;
  ss->ss1.base_addr = obj_bo->offset;
  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.pitch = w*bpp - 1;
  ss->ss5.cache_control = cchint;
  gpgpu_set_buf_reloc_gen6(state, index, obj_bo);
}

static void
gpgpu_bind_image2D_gen7(intel_gpgpu_t *state,
                        int32_t index,
                        dri_bo* obj_bo,
                        uint32_t format,
                        int32_t w,
                        int32_t h,
                        int bpp,
                        uint32_t cchint)
{
  surface_heap_t *heap = state->surface_heap_b.bo->virtual;
  gen7_surface_state_t *ss = (gen7_surface_state_t *) heap->surface[index];
  memset(ss, 0, sizeof(*ss));
  ss->ss0.surface_type = I965_SURFACE_2D;
  ss->ss0.surface_format = format;
  ss->ss1.base_addr = obj_bo->offset;
  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.pitch = w*bpp - 1;
  ss->ss5.cache_control = cchint;
  gpgpu_set_buf_reloc_gen7(state, index, obj_bo);
}

LOCAL void
gpgpu_bind_buf(intel_gpgpu_t *state,
               int32_t index,
               dri_bo* obj_bo,
               uint32_t size,
               uint32_t cchint)
{
  assert(index < MAX_SURFACES);
  if(state->drv->gen_ver == 6)
    gpgpu_bind_buf_gen6(state, index, obj_bo, size, cchint);
  else if (state->drv->gen_ver == 7 || state->drv->gen_ver == 75)
    gpgpu_bind_buf_gen7(state, index, obj_bo, size, cchint);
  else
    NOT_IMPLEMENTED;
}

LOCAL void
gpgpu_bind_image2D(intel_gpgpu_t *state,
                   int32_t index,
                   dri_bo* obj_bo,
                   uint32_t format,
                   int32_t w,
                   int32_t h,
                   int bpp,
                   uint32_t cchint)
{
  assert(index < MAX_SURFACES);
  if(state->drv->gen_ver == 6)
    gpgpu_bind_image2D_gen6(state, index, obj_bo, format, w, h, bpp, cchint);
  else if (state->drv->gen_ver == 7 || state->drv->gen_ver == 75)
    gpgpu_bind_image2D_gen7(state, index, obj_bo, format, w, h, bpp, cchint);
  else
    NOT_IMPLEMENTED;
}

static void
gpgpu_build_idrt(intel_gpgpu_t *state,
                 genx_gpgpu_kernel_t *kernel,
                 uint32_t ker_n)
{
  gen6_interface_descriptor_t *desc;
  dri_bo *bo;
  uint32_t i;

  bo = state->idrt_b.bo;
  dri_bo_map(bo, 1);
  assert(bo->virtual);
  desc = (gen6_interface_descriptor_t*) bo->virtual;

  for (i = 0; i < ker_n; i++) {
    memset(desc, 0, sizeof(*desc));
    desc->desc0.kernel_start_pointer = kernel[i].bo->offset >> 6; /* reloc */
    desc->desc2.sampler_state_pointer = state->sampler_state_b.bo->offset >> 5;
    desc->desc3.binding_table_entry_count = 0; /* no prefetch */
    desc->desc3.binding_table_pointer = 0;
    desc->desc4.curbe_read_len = kernel[i].cst_sz / 32;
    desc->desc4.curbe_read_offset = 0;

    /* Barriers / SLM are automatically handled on Gen7+ */
    if (state->drv->gen_ver == 7 || state->drv->gen_ver == 75) {
      size_t slm_sz = kernel[i].slm_sz;
      desc->desc5.group_threads_num = kernel[i].use_barrier ? kernel[i].thread_n : 0;
      desc->desc5.barrier_enable = kernel[i].use_barrier;
      if (slm_sz > 0) {
        if (slm_sz <= 4 * KB)
          slm_sz = 4 * KB; //4KB
        else if (slm_sz <= 8 * KB)
          slm_sz = 8 * KB; //8KB
        else if (slm_sz <= 16 * KB)
          slm_sz = 16 * KB; //16KB
        else if (slm_sz <= 32 * KB)
          slm_sz = 32 * KB; //32KB
        else if (slm_sz <= 64 * KB)
          slm_sz = 64 * KB; //64KB
        slm_sz = slm_sz >> 12;
      }
      desc->desc5.slm_sz = slm_sz;
    }
    else
      desc->desc5.group_threads_num = kernel[i].barrierID; /* BarrierID on GEN6 */

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      0,
                      i * sizeof(*desc) + offsetof(gen6_interface_descriptor_t, desc0),
                      kernel[i].bo);

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      0,
                      i * sizeof(*desc) + offsetof(gen6_interface_descriptor_t, desc2),
                      state->sampler_state_b.bo);
    desc++;
  }
  state->idrt_b.num = ker_n;
  dri_bo_unmap(bo);
}

LOCAL void
gpgpu_upload_constants(intel_gpgpu_t *state, const void* data, uint32_t size)
{
  unsigned char *constant_buffer = NULL;

  dri_bo_map(state->curbe_b.bo, 1);
  assert(state->curbe_b.bo->virtual);
  constant_buffer = (unsigned char *) state->curbe_b.bo->virtual;
  memcpy(constant_buffer, data, size);
  dri_bo_unmap(state->curbe_b.bo);
}

LOCAL void
gpgpu_upload_samplers(intel_gpgpu_t *state, const void *data, uint32_t n)
{
  if (n) {
    /*sizeof(gen6_sampler_state_t) == sizeof(gen7_surface_state_t) */
    const size_t sz = n * sizeof(gen6_sampler_state_t);
    memcpy(state->sampler_state_b.bo->virtual, data, sz);
  }
}

LOCAL void
gpgpu_states_setup(intel_gpgpu_t *state, genx_gpgpu_kernel_t *kernel, uint32_t ker_n)
{
  state->ker = kernel;
  gpgpu_build_idrt(state, kernel, ker_n);
  dri_bo_unmap(state->surface_heap_b.bo);
  dri_bo_unmap(state->sampler_state_b.bo);
}

LOCAL void 
gpgpu_update_barrier(intel_gpgpu_t *state, uint32_t barrierID, uint32_t thread_n)
{
  BEGIN_BATCH(state->batch, 4);
  OUT_BATCH(state->batch, CMD_MEDIA_STATE_FLUSH | 0);
  OUT_BATCH(state->batch, 1 << barrierID);
  OUT_BATCH(state->batch, CMD_MEDIA_GATEWAY_STATE | 0);
  OUT_BATCH(state->batch, (barrierID << 16) | thread_n);
  ADVANCE_BATCH(state->batch);
}

LOCAL void
gpgpu_set_perf_counters(intel_gpgpu_t *state, dri_bo *perf)
{
  if (state->perf_b.bo)
    drm_intel_bo_unreference(state->perf_b.bo);
  drm_intel_bo_reference(perf);
  state->perf_b.bo = perf;
}

LOCAL void
gpgpu_run(intel_gpgpu_t *state, int32_t ki)
{
  BEGIN_BATCH(state->batch, 6);
  OUT_BATCH(state->batch, GEN_CMD_MEDIA_OBJECT | 5);
  OUT_BATCH(state->batch, ki); /* kernel index + DBGOBJ=0 << 8 */
  OUT_BATCH(state->batch, 0);  /* no children, no thread sync, no scoreboard, any half-slice, indirect data size = 0 */
  OUT_BATCH(state->batch, 0);  /* no indirect data ptr */
  OUT_BATCH(state->batch, 0);  /* scoreboard Y,X = 0 or just 0 in reg.0 */
  OUT_BATCH(state->batch, 0);  /* scoreboard color, mask = 0  or just 0 in reg.1 */
  OUT_BATCH(state->batch, 0);  /* scoreboard color, mask = 0  or just 0 in reg.1 */
  ADVANCE_BATCH(state->batch);
}

LOCAL char*
gpgpu_run_with_inline(intel_gpgpu_t *state, int32_t ki, size_t sz)
{
  const uint32_t len = (uint32_t) (sz >> 2);

  assert(sz % sizeof(int32_t) == 0);
  BEGIN_BATCH(state->batch, len + 6);
  OUT_BATCH(state->batch, GEN_CMD_MEDIA_OBJECT | (len + 4));
  OUT_BATCH(state->batch, ki); /* kernel index + DBGOBJ=0 << 8 */
  OUT_BATCH(state->batch, 0);  /* no children, no thread sync, no scoreboard, any half-slice, indirect data size = 0 */
  OUT_BATCH(state->batch, 0);  /* no indirect data ptr */
  OUT_BATCH(state->batch, 0);  /* scoreboard Y,X = 0 or just 0 in reg.0 */
  OUT_BATCH(state->batch, 0);  /* scoreboard color, mask = 0  or just 0 in reg.1 */
  return (char*) intel_batchbuffer_alloc_space(state->batch,sz);
}

LOCAL void
gpgpu_walker(intel_gpgpu_t *state,
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

  BEGIN_BATCH(state->batch, 11);
  OUT_BATCH(state->batch, CMD_GPGPU_WALKER | 9);
  OUT_BATCH(state->batch, 0);                        /* kernel index == 0 */
  OUT_BATCH(state->batch, (1 << 30) | (thread_n-1)); /* SIMD16 | thread max */
  OUT_BATCH(state->batch, global_wk_off[0]);
  OUT_BATCH(state->batch, global_wk_dim[0]);
  OUT_BATCH(state->batch, global_wk_off[1]);
  OUT_BATCH(state->batch, global_wk_dim[1]);
  OUT_BATCH(state->batch, global_wk_off[2]);
  OUT_BATCH(state->batch, global_wk_dim[2]);
  OUT_BATCH(state->batch, ~0x0);
  OUT_BATCH(state->batch, ~0x0);
  ADVANCE_BATCH(state->batch);

  BEGIN_BATCH(state->batch, 2);
  OUT_BATCH(state->batch, CMD_MEDIA_STATE_FLUSH | 0);
  OUT_BATCH(state->batch, 0);                        /* kernel index == 0 */
  ADVANCE_BATCH(state->batch);
}

LOCAL int32_t
intel_gpgpu_version(intel_gpgpu_t *gpgpu)
{
  return gpgpu->drv->gen_ver;
}

