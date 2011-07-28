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

#include "intel/genx_gpgpu.h"
#include "intel/genx_defines.h"
#include "intel/genx_structs.h"
#include "intel/intel_batchbuffer.h"
#include "intel/intel_driver.h"
#include "intel/genx_defines.h"

#include "cl_alloc.h"
#include "cl_utils.h"

#define GEN_CMD_MEDIA_OBJECT  (0x71000000)
#define MO_TS_BIT             (1 << 24)
#define MO_RETAIN_BIT         (1 << 28)
#define SAMPLER_STATE_SIZE    (16)

typedef struct gen6_surface_state
{
  struct {
    uint32_t cube_pos_z:1;
    uint32_t cube_neg_z:1;
    uint32_t cube_pos_y:1;
    uint32_t cube_neg_y:1;
    uint32_t cube_pos_x:1;
    uint32_t cube_neg_x:1;
    uint32_t pad:2;
    uint32_t render_cache_read_mode:1;
    uint32_t cube_map_corner_mode:1;
    uint32_t mipmap_layout_mode:1;
    uint32_t vert_line_stride_ofs:1;
    uint32_t vert_line_stride:1;
    uint32_t color_blend:1;
    uint32_t writedisable_blue:1;
    uint32_t writedisable_green:1;
    uint32_t writedisable_red:1;
    uint32_t writedisable_alpha:1;
    uint32_t surface_format:9;
    uint32_t data_return_format:1;
    uint32_t pad0:1;
    uint32_t surface_type:3;
  } ss0;

  struct {
    uint32_t base_addr;
  } ss1;

  struct {
    uint32_t render_target_rotation:2;
    uint32_t mip_count:4;
    uint32_t width:13;
    uint32_t height:13;
  } ss2;

  struct {
    uint32_t tile_walk:1;
    uint32_t tiled_surface:1;
    uint32_t pad:1;
    uint32_t pitch:18;
    uint32_t depth:11;
  } ss3;

  struct {
    uint32_t multisample_pos_index:3;
    uint32_t pad:1;
    uint32_t multisample_count:3;
    uint32_t pad1:1;
    uint32_t rt_view_extent:9;
    uint32_t min_array_elt:11;
    uint32_t min_lod:4;
  } ss4;

  struct {
    uint32_t pad:16;
    uint32_t cache_control:2;  /* different values for GT and IVB */
    uint32_t gfdt:1;           /* allows selective flushing of LLC (e.g. for scanout) */
    uint32_t encrypted_data:1;
    uint32_t y_offset:4;
    uint32_t vertical_alignment:1;
    uint32_t x_offset:7;
  } ss5;

  uint32_t ss6; /* unused */
  uint32_t ss7; /* unused */
} gen6_surface_state_t;

typedef struct gen7_surface_state
{
  struct {
    uint32_t cube_pos_z:1;
    uint32_t cube_neg_z:1;
    uint32_t cube_pos_y:1;
    uint32_t cube_neg_y:1;
    uint32_t cube_pos_x:1;
    uint32_t cube_neg_x:1;
    uint32_t media_boundary_pixel_mode:2;
    uint32_t render_cache_rw_mode:1;
    uint32_t pad1:1;
    uint32_t surface_array_spacing:1;
    uint32_t vertical_line_stride_offset:1;
    uint32_t vertical_line_stride:1;
    uint32_t tile_walk:1;
    uint32_t tiled_surface:1;
    uint32_t horizontal_alignment:1;
    uint32_t vertical_alignment:2;
    uint32_t surface_format:9;
    uint32_t pad0:1;
    uint32_t surface_array:1;
    uint32_t surface_type:3;
  } ss0;

  struct {
    uint32_t base_addr;
  } ss1;

  struct {
    uint32_t width:14;
    uint32_t pad1:2;
    uint32_t height:14;
    uint32_t pad0:2;
  } ss2;

  struct {
    uint32_t pitch:18;
    uint32_t pad0:3;
    uint32_t depth:11;
  } ss3;

  uint32_t ss4;

  struct {
    uint32_t mip_count:4;
    uint32_t surface_min_load:4;
    uint32_t pad2:6;
    uint32_t coherence_type:1;
    uint32_t stateless_force_write_thru:1;
    uint32_t surface_object_control_state:4;
    uint32_t y_offset:4;
    uint32_t pad0:1;
    uint32_t x_offset:7;
  } ss5;

  uint32_t ss6; /* unused */
  uint32_t ss7; /* unused */

} gen7_surface_state_t;

#define GEN7_CACHED_IN_LLC 3

STATIC_ASSERT(sizeof(gen6_surface_state_t) == sizeof(gen7_surface_state_t));
static const size_t surface_state_sz = sizeof(gen6_surface_state_t);

typedef struct gen6_vfe_state_inline
{
  struct {
    uint32_t per_thread_scratch_space:4;
    uint32_t pad3:3;
    uint32_t extend_vfe_state_present:1;
    uint32_t pad2:2;
    uint32_t scratch_base:22;
  } vfe0;

  struct {
    uint32_t debug_counter_control:2;
    uint32_t gpgpu_mode:1;          /* 0 for SNB!!! */
    uint32_t gateway_mmio_access:2;
    uint32_t fast_preempt:1;
    uint32_t bypass_gateway_ctl:1;  /* 0 - legacy, 1 - no open/close */
    uint32_t reset_gateway_timer:1;
    uint32_t urb_entries:8;
    uint32_t max_threads:16;
  } vfe1;

  struct {
    uint32_t pad8:8;
    uint32_t debug_object_id:24;
  } vfe2;

  struct {
    uint32_t curbe_size:16; /* in GRFs */
    uint32_t urbe_size:16;  /* in GRFs */
  } vfe3;

  struct {
    uint32_t scoreboard_mask:8;  /* 1 - enable the corresponding dependency */
    uint32_t pad22:22;
    uint32_t scoreboard_type:1;   /* 0 stalling, 1 - non-stalling */
    uint32_t scoreboard_enable:1; /* 0 disabled, 1 enabled */

  } vfe4;

  struct {
    uint32_t scoreboard0_dx:4;
    uint32_t scoreboard0_dy:4;
    uint32_t scoreboard1_dx:4;
    uint32_t scoreboard1_dy:4;
    uint32_t scoreboard2_dx:4;
    uint32_t scoreboard2_dy:4;
    uint32_t scoreboard3_dx:4;
    uint32_t scoreboard3_dy:4;
  } vfe5;

  struct {
    uint32_t scoreboard4_dx:4;
    uint32_t scoreboard4_dy:4;
    uint32_t scoreboard5_dx:4;
    uint32_t scoreboard5_dy:4;
    uint32_t scoreboard6_dx:4;
    uint32_t scoreboard6_dy:4;
    uint32_t scoreboard7_dx:4;
    uint32_t scoreboard7_dy:4;
  } vfe6;
} gen6_vfe_state_inline_t;

typedef struct gen6_interface_descriptor
{
  struct {
    uint32_t pad6:6;
    uint32_t kernel_start_pointer:26;
  } desc0;

  struct {
    uint32_t pad:7;
    uint32_t software_exception:1;
    uint32_t pad2:3;
    uint32_t maskstack_exception:1;
    uint32_t pad3:1;
    uint32_t illegal_opcode_exception:1;
    uint32_t pad4:2;
    uint32_t floating_point_mode:1;
    uint32_t thread_priority:1;
    uint32_t single_program_flow:1;
    uint32_t pad5:1;
    uint32_t pad6:6;
    uint32_t pad7:6;
  } desc1;

  struct {
    uint32_t pad:2;
    uint32_t sampler_count:3;
    uint32_t sampler_state_pointer:27;
  } desc2;

  struct {
    uint32_t binding_table_entry_count:5;  /* prefetch entries only */
    uint32_t binding_table_pointer:27;     /* 11 bit only on IVB+ */
  } desc3;

  struct {
    uint32_t curbe_read_offset:16;         /* in GRFs */
    uint32_t curbe_read_len:16;            /* in GRFs */
  } desc4;

  struct {
    uint32_t group_threads_num:8;        /* 0..64, 0 - no barrier use */
    uint32_t barrier_return_byte:8;
    uint32_t shared_local_mem_size:5;    /* 0..16 - 0K..64K */
    uint32_t barrier_enable:1;
    uint32_t rounding_mode:2;
    uint32_t barrier_return_grf_offset:8;
  } desc5;

  struct {
    uint32_t reserved_mbz;
  } desc6;

  struct {
    uint32_t reserved_mbz;
  } desc7;
} gen6_interface_descriptor_t;

/* No dependency on Gen specific structures */
struct opaque_sampler_state {
  char opaque[SAMPLER_STATE_SIZE];
};

#define MAX_IF_DESC    32
#define MAX_THREADS    72
#define MAX_SCRATCH_KB 12

/* Device abstraction */
struct intel_driver;

/* Handle GPGPU state (actually "media" state) */
struct genx_gpgpu_state
{
  intel_driver_t *drv;
  intel_batchbuffer_t *batch;

  struct {
    dri_bo *bo;
    uint32_t num;
  } idrt_b;
  struct { dri_bo *bo; } surface_state_b[MAX_SURFACES]; 
  struct { dri_bo *bo; } binding_table_b;
  struct { dri_bo *bo; } vfe_state_b;
  struct { dri_bo *bo; } curbe_b;
  struct { dri_bo *bo; } sampler_state_b;
  struct { dri_bo *bo; } perf_b;

  /* we will just copy them into the bo */
  struct opaque_sampler_state samplers[MAX_SAMPLERS];

  struct {
    uint32_t vfe_start;         /* start of vfe entries in urb */
    uint32_t cs_start;          /* start of cs entries in urb */
    uint32_t num_vfe_entries;
    uint32_t num_cs_entries;
    uint32_t size_vfe_entry;    /* size of one entry in 512bit elements */
    uint32_t size_cs_entry;     /* size of one entry in 512bit elements */
  } urb;

  uint32_t max_threads;         /* max threads requested by the user */
};

/* Be sure that the size is still valid */
STATIC_ASSERT(sizeof(struct opaque_sampler_state) == sizeof(struct i965_sampler_state));

LOCAL genx_gpgpu_state_t*
genx_gpgpu_state_new(intel_driver_t *drv)
{
  genx_gpgpu_state_t *state = NULL;

  TRY_ALLOC_NO_ERR (state, CALLOC(genx_gpgpu_state_t));
  state->drv = drv;
  state->batch = intel_batchbuffer_new(state->drv);
  assert(state->batch);
  intel_batchbuffer_init(state->batch, state->drv);

exit:
  return state;
error:
  genx_gpgpu_state_delete(state);
  state = NULL;
  goto exit;
}

LOCAL void
genx_gpgpu_state_delete(genx_gpgpu_state_t *state)
{
  uint32_t i;

  if (state == NULL)
    return;
  for (i = 0; i < MAX_SURFACES; ++i)
    if (state->surface_state_b[i].bo)
      drm_intel_bo_unreference(state->surface_state_b[i].bo);
  if (state->binding_table_b.bo)
    drm_intel_bo_unreference(state->binding_table_b.bo);
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
gpgpu_select_pipeline(genx_gpgpu_state_t *state)
{
  BEGIN_BATCH(state->batch, 1);
  OUT_BATCH(state->batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA);
  ADVANCE_BATCH(state->batch);
}

static void
gpgpu_set_base_address(genx_gpgpu_state_t *state)
{
  const uint32_t def_cc = cc_llc_mlc; /* default Cache Control value */
  BEGIN_BATCH(state->batch, 10);
  OUT_BATCH(state->batch, CMD_STATE_BASE_ADDRESS | 8);
  /* 0, Gen State Mem Obj CC, Stateless Mem Obj CC, Stateless Access Write Back */
  OUT_BATCH(state->batch, 0 | (def_cc << 8) | (def_cc << 4) | (0 << 3)| BASE_ADDRESS_MODIFY);    /* General State Base Addr   */
  /* 0, State Mem Obj CC */
  OUT_BATCH(state->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Surface State Base Addr */
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
gpgpu_load_vfe_state(genx_gpgpu_state_t *state)
{
  BEGIN_BATCH(state->batch, 8);
  OUT_BATCH(state->batch, CMD_MEDIA_STATE_POINTERS | (8-2));

  struct gen6_vfe_state_inline* vfe = (struct gen6_vfe_state_inline*)
    intel_batchbuffer_alloc_space(state->batch,0);

  memset(vfe, 0, sizeof(struct gen6_vfe_state_inline));
  vfe->vfe1.fast_preempt = 1;
  vfe->vfe1.bypass_gateway_ctl = 1;
  vfe->vfe1.reset_gateway_timer = 1;
  vfe->vfe1.urb_entries = state->urb.num_vfe_entries;
  vfe->vfe3.urbe_size = state->urb.size_vfe_entry;
  vfe->vfe3.curbe_size = state->urb.size_cs_entry*state->urb.num_cs_entries;
  vfe->vfe1.max_threads = state->max_threads - 1;
/*  vfe->vfe3.curbe_size = 63; */
/*  vfe->vfe3.urbe_size = 13; */
  vfe->vfe4.scoreboard_enable = 1;
  intel_batchbuffer_alloc_space(state->batch, sizeof(struct gen6_vfe_state_inline));
  ADVANCE_BATCH(state->batch);
}

static void
gpgpu_load_constant_buffer(genx_gpgpu_state_t *state) 
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
gpgpu_load_idrt(genx_gpgpu_state_t *state) 
{
  BEGIN_BATCH(state->batch, 4);
  OUT_BATCH(state->batch, CMD(2,0,2) | (4 - 2)); /* length-2 */
  OUT_BATCH(state->batch, 0);                    /* mbz */
  OUT_BATCH(state->batch, state->idrt_b.num*32);
  OUT_RELOC(state->batch, state->idrt_b.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
  ADVANCE_BATCH(state->batch);
}

LOCAL void
gpgpu_batch_start(genx_gpgpu_state_t *state)
{
  intel_batchbuffer_start_atomic(state->batch, 256);
  intel_batchbuffer_emit_mi_flush(state->batch);
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
gpgpu_batch_end(genx_gpgpu_state_t *state, int32_t flush_mode)
{
  /* Insert the performance counter command */
  if (state->perf_b.bo) {
    BEGIN_BATCH(state->batch, 3);
    OUT_BATCH(state->batch, (0x28 << 23) | /* MI_REPORT_PERF_COUNT */
                     (3 - 2));      /* length-2 */
    OUT_RELOC(state->batch, state->perf_b.bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              512 |  /* Offset for the end "counters" */
              1);    /* Use GTT and not PGTT */
    OUT_BATCH(state->batch, 0);
    ADVANCE_BATCH(state->batch);
  }

  if(flush_mode) {
    BEGIN_BATCH(state->batch, 1);
    OUT_BATCH(state->batch, MI_FLUSH);
    ADVANCE_BATCH(state->batch);
  }
  intel_batchbuffer_end_atomic(state->batch);
}

LOCAL void
gpgpu_batch_reset(genx_gpgpu_state_t *state, size_t sz)
{
  intel_batchbuffer_reset(state->batch, sz);
}

LOCAL void
gpgpu_flush(genx_gpgpu_state_t *state)
{
  intel_batchbuffer_flush(state->batch);
}

LOCAL void
gpgpu_state_init(genx_gpgpu_state_t *state,
                 uint32_t max_threads,
                 uint32_t size_vfe_entry,
                 uint32_t num_vfe_entries,
                 uint32_t size_cs_entry,
                 uint32_t num_cs_entries)
{
  dri_bo *bo;
  int32_t i;

  /* URB */
  state->urb.vfe_start = 0;
  state->urb.num_vfe_entries = num_vfe_entries;
  state->urb.size_vfe_entry = size_vfe_entry;
  state->urb.num_cs_entries = num_cs_entries;
  state->urb.size_cs_entry = size_cs_entry;
  state->urb.cs_start = state->urb.vfe_start + state->urb.num_vfe_entries * state->urb.size_vfe_entry;
  state->max_threads = max_threads;

  /* constant buffer */
  if(state->curbe_b.bo)
    dri_bo_unreference(state->curbe_b.bo);
  uint32_t size_cb = state->urb.num_cs_entries * state->urb.size_cs_entry * (512/8);
  size_cb = (size_cb + (4096 - 1)) & (~(4096-1)); /* roundup to 4K */
  bo = dri_bo_alloc(state->drv->bufmgr,
                    "CONSTANT_BUFFER",
                    size_cb,
                    64);
  assert(bo);
  state->curbe_b.bo = bo;

  /* surface state */
  for (i = 0; i < (int) MAX_SURFACES; i++) {
    if(state->surface_state_b[i].bo)
      dri_bo_unreference(state->surface_state_b[i].bo);
    state->surface_state_b[i].bo = NULL;
  }

  /* binding table */
  if(state->binding_table_b.bo)
    dri_bo_unreference(state->binding_table_b.bo);
  bo = dri_bo_alloc(state->drv->bufmgr, 
                    "SS_SURF_BIND",
                    MAX_SURFACES * sizeof(uint32_t),
                    32);
  assert(bo);
  state->binding_table_b.bo = bo;

  /* IDRT */
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
  bo = NULL;

  /* sampler state */
  if (state->sampler_state_b.bo)
    dri_bo_unreference(state->sampler_state_b.bo);
  bo = dri_bo_alloc(state->drv->bufmgr, 
                    "sample states",
                    MAX_SAMPLERS * sizeof(struct i965_sampler_state),
                    32);
  assert(bo);
  state->sampler_state_b.bo = bo;
  memset(state->samplers, 0, sizeof(state->samplers));
}

LOCAL void
gpgpu_bind_surf_2d(genx_gpgpu_state_t *state,
                   int32_t index,
                   dri_bo* obj_bo,
                   uint32_t offset,
                   uint32_t format,
                   int32_t w, int32_t h,
                   uint32_t is_dst,
                   int32_t vert_line_stride,
                   int32_t vert_line_stride_ofs,
                   uint32_t cchint)
{
  gen6_surface_state_t *ss;
  dri_bo *bo;
  uint32_t write_domain, read_domain;

  if(state->surface_state_b[index].bo) {
    dri_bo_unreference(state->surface_state_b[index].bo);
    state->surface_state_b[index].bo = NULL;
  }

  bo = dri_bo_alloc(state->drv->bufmgr, "surface state", surface_state_sz, 32);
  assert(bo);
  dri_bo_map(bo, 1);
  assert(bo->virtual);
  ss = (gen6_surface_state_t*) bo->virtual;
  memset(ss, 0, sizeof(*ss));
  ss->ss0.surface_type = I965_SURFACE_2D;
  ss->ss0.surface_format = format;
  ss->ss0.vert_line_stride = vert_line_stride;
  ss->ss0.vert_line_stride_ofs = vert_line_stride_ofs;
  ss->ss1.base_addr = obj_bo->offset + offset;
  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.pitch = (w*4) - 1; /* TEMP patch */

  /* TODO: parse GFDT bit as well */
  if(state->drv->gen_ver == 6)
    ss->ss5.cache_control = cchint;

  if (is_dst) {
    write_domain = I915_GEM_DOMAIN_RENDER;
    read_domain = I915_GEM_DOMAIN_RENDER;
  } else {
    write_domain = 0;
    read_domain = I915_GEM_DOMAIN_SAMPLER;
  }

  dri_bo_emit_reloc(bo,
                    read_domain, write_domain,
                    offset,
                    offsetof(gen6_surface_state_t, ss1),
                    obj_bo);
  dri_bo_unmap(bo);

  assert(index < (int) MAX_SURFACES);
  state->surface_state_b[index].bo = bo;
}

LOCAL void
gpgpu_bind_shared_surf_2d(genx_gpgpu_state_t *state,
                          int32_t index,
                          dri_bo* obj_bo,
                          uint32_t offset,
                          uint32_t format,
                          int32_t w,
                          int32_t h,
                          uint32_t is_dst,
                          int32_t vert_line_stride,
                          int32_t vert_line_stride_ofs,
                          uint32_t cchint)
{
  gen6_surface_state_t *ss;
  dri_bo *bo;
  uint32_t write_domain, read_domain;

  uint32_t tiling = 0;
  uint32_t swizzle = 0;

  /* query tiling from shared surface bo */
#ifndef NDEBUG
  int32_t ret =
#endif /* NDEBUG */
  dri_bo_get_tiling(obj_bo, &tiling, &swizzle);
  assert(ret == 0);

  /*TODO: choose between delete/reuse */
  if(state->surface_state_b[index].bo) {
    dri_bo_unreference(state->surface_state_b[index].bo);
    state->surface_state_b[index].bo = NULL;
  }

  bo = dri_bo_alloc(state->drv->bufmgr, 
                    "surface state", 
                    sizeof(gen6_surface_state_t),
                    32);
  assert(bo);
  dri_bo_map(bo, 1);
  assert(bo->virtual);
  ss = (gen6_surface_state_t *)bo->virtual;
  memset(ss, 0, sizeof(*ss));
  ss->ss0.surface_type = I965_SURFACE_2D;
  ss->ss0.surface_format = format;
  ss->ss0.vert_line_stride = vert_line_stride;
  ss->ss0.vert_line_stride_ofs = vert_line_stride_ofs;
  ss->ss1.base_addr = obj_bo->offset + offset;
  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.pitch = (w*4) - 1; /* TEMP patch */

  /* TODO: parse GFDT bit as well */
  if(state->drv->gen_ver == 6)
    ss->ss5.cache_control = cchint;

  switch (tiling) {
    case I915_TILING_NONE:
      ss->ss3.tiled_surface = 0;
      ss->ss3.tile_walk = 0;
      break;
    case I915_TILING_X:
      ss->ss3.tiled_surface = 1;
      ss->ss3.tile_walk = I965_TILEWALK_XMAJOR;
      break;
    case I915_TILING_Y:
      ss->ss3.tiled_surface = 1;
      ss->ss3.tile_walk = I965_TILEWALK_YMAJOR;
      break;
  }

  if (is_dst) {
    write_domain = I915_GEM_DOMAIN_RENDER;
    read_domain = I915_GEM_DOMAIN_RENDER;
  } else {
    write_domain = 0;
    read_domain = I915_GEM_DOMAIN_SAMPLER;
  }

  dri_bo_emit_reloc(bo,
                    read_domain, write_domain,
                    offset,
                    offsetof(gen6_surface_state_t, ss1),
                    obj_bo);
  dri_bo_unmap(bo);

  assert(index < (int) MAX_SURFACES);
  state->surface_state_b[index].bo = bo;
}

LOCAL void
gpgpu_bind_buf(genx_gpgpu_state_t *state,
               int32_t index,
               dri_bo* obj_bo,
               uint32_t offset,
               uint32_t size,
               uint32_t cchint)
{
  dri_bo *bo;
  uint32_t write_domain, read_domain;

  assert(offset < MAX_SURFACES);

  /*TODO: choose between delete/reuse */
  if(state->surface_state_b[index].bo) {
    dri_bo_unreference(state->surface_state_b[index].bo);
    state->surface_state_b[index].bo = NULL;
  }

  bo = dri_bo_alloc(state->drv->bufmgr, "SS_SURFACE", surface_state_sz, 32);
  assert(bo);
  dri_bo_map(bo, 1);
  assert(bo->virtual);
  write_domain = I915_GEM_DOMAIN_RENDER;
  read_domain = I915_GEM_DOMAIN_RENDER;

  if(state->drv->gen_ver == 6) {
    gen6_surface_state_t *ss = (gen6_surface_state_t *) bo->virtual;
    const uint32_t size_ss = ((size+0xf) >> 4) - 1; /* ceil(size/16) - 1 */
    memset(ss, 0, sizeof(*ss));
    ss->ss0.surface_type = I965_SURFACE_BUFFER;
    ss->ss0.surface_format = I965_SURFACEFORMAT_R32G32B32A32_FLOAT;
    ss->ss1.base_addr = obj_bo->offset + offset;
    ss->ss2.width = size_ss & 0x7f; /* bits 6:0 of size_ss */
    ss->ss2.height = (size_ss >> 7) & 0x1fff; /* bits 19:7 of size_ss */
    ss->ss3.pitch = 0xf;
    ss->ss3.depth = size_ss >> 20; /* bits 26:20 of size_ss */
    ss->ss5.cache_control = cchint;
    dri_bo_emit_reloc(bo,
                      read_domain,
                      write_domain,
                      offset,
                      offsetof(gen6_surface_state_t, ss1),
                      obj_bo);
  } else if (state->drv->gen_ver == 7) {
    gen7_surface_state_t *ss = (gen7_surface_state_t *) bo->virtual;
    const uint32_t size_ss = size - 1;
    memset(ss, 0, sizeof(*ss));
    ss->ss0.surface_type = I965_SURFACE_BUFFER;
    ss->ss0.surface_format = I965_SURFACEFORMAT_RAW;
    ss->ss1.base_addr = obj_bo->offset + offset;
    ss->ss2.width  = size_ss & 0x7f; /* bits 6:0 of size_ss */
    ss->ss2.height = (size_ss & 0x1fff80) >> 7; /* bits 20:7 of size_ss */
    ss->ss3.depth  = (size_ss & 0xffe00000) >> 20; /* bits 27:21 of size_ss */
    ss->ss5.surface_object_control_state = GEN7_CACHED_IN_LLC;
    dri_bo_emit_reloc(bo,
                      read_domain,
                      write_domain,
                      offset,
                      offsetof(gen7_surface_state_t, ss1),
                      obj_bo);
  }

  dri_bo_unmap(bo);
  assert(index < (int) MAX_SURFACES);
  state->surface_state_b[index].bo = bo;
}

LOCAL void
gpgpu_set_sampler(genx_gpgpu_state_t *state, uint32_t index, uint32_t non_normalized)
{
  struct i965_sampler_state *sampler = NULL;
  assert(index < (int) MAX_SAMPLERS);

#ifndef NDEBUG
  if (non_normalized && state->drv->gen_ver == 5)
    assert("Non normalized coordinates are unsupported on Gen5" && 0);
#endif /* NDEBUG */

  sampler = (struct i965_sampler_state *) state->samplers[index].opaque;
  if (non_normalized) {
    sampler->ss3.gen6_non_normalized = 1;
    sampler->ss1.r_wrap_mode = I965_TEXCOORDMODE_CLAMP;
    sampler->ss1.t_wrap_mode = I965_TEXCOORDMODE_CLAMP;
    sampler->ss1.s_wrap_mode = I965_TEXCOORDMODE_CLAMP;
  }
  else
    sampler->ss3.gen6_non_normalized = 0;
}

static void
gpgpu_build_sampler_table(genx_gpgpu_state_t *state)
{
  dri_bo_subdata(state->sampler_state_b.bo,
                 0,
                 sizeof(state->samplers),
                 state->samplers);
}

static void
gpgpu_build_binding_table(genx_gpgpu_state_t *state)
{
  uint32_t *binding_table;
  dri_bo *bo = state->binding_table_b.bo;
  uint32_t i;

  dri_bo_map(bo, 1);
  assert(bo->virtual);
  binding_table = (uint32_t *)bo->virtual;
  memset(binding_table, 0, bo->size);

  for (i = 0; i < MAX_SURFACES; i++) {
    if (state->surface_state_b[i].bo) {
      binding_table[i] = state->surface_state_b[i].bo->offset;
      dri_bo_emit_reloc(bo,
                        I915_GEM_DOMAIN_INSTRUCTION, 0,
                        0,
                        i * sizeof(*binding_table),
                        state->surface_state_b[i].bo);
    }
  }

  dri_bo_unmap(state->binding_table_b.bo);
}

static void
gpgpu_build_idrt(genx_gpgpu_state_t *state,
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
    /* desc->desc1 = 0; - no exception control, no SPF */
    /* desc->desc2 = 0; - no samplers */
    desc->desc2.sampler_state_pointer = state->sampler_state_b.bo->offset >> 5;
    desc->desc3.binding_table_entry_count = 0; /* no prefetch */
    desc->desc3.binding_table_pointer = state->binding_table_b.bo->offset >> 5;
    desc->desc4.curbe_read_len = kernel[i].cst_sz / 32;
    desc->desc4.curbe_read_offset = 0;
    desc->desc5.group_threads_num = kernel[i].barrierID; /* BarrierID on GEN6 */
    /* desc->desc5 = 0; - no barriers, groups, etc. */
    /* desc->desc6 = 0; - mbz */
    /* desc->desc7 = 0; - mbz */

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      0,
                      i * sizeof(*desc) + offsetof(gen6_interface_descriptor_t, desc0),
                      kernel[i].bo);

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      desc->desc3.binding_table_entry_count,
                      i * sizeof(*desc) + offsetof(gen6_interface_descriptor_t, desc3),
                      state->binding_table_b.bo);

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
gpgpu_upload_constants(genx_gpgpu_state_t *state, void* data, uint32_t size)
{
  unsigned char *constant_buffer = NULL;

  dri_bo_map(state->curbe_b.bo, 1);
  assert(state->curbe_b.bo->virtual);
  constant_buffer = (unsigned char *) state->curbe_b.bo->virtual;
  memcpy(constant_buffer, data, size);
  dri_bo_unmap(state->curbe_b.bo);
}

LOCAL void
gpgpu_states_setup(genx_gpgpu_state_t *state, genx_gpgpu_kernel_t *kernel, uint32_t ker_n)
{
  gpgpu_build_sampler_table(state);
  gpgpu_build_binding_table(state);
  gpgpu_build_idrt(state, kernel, ker_n);
}

LOCAL void 
gpgpu_update_barrier(genx_gpgpu_state_t *state, uint32_t barrierID, uint32_t thread_n)
{
  BEGIN_BATCH(state->batch, 4);
  OUT_BATCH(state->batch, CMD_MEDIA_STATE_FLUSH | 0);
  OUT_BATCH(state->batch, 1 << barrierID);
  OUT_BATCH(state->batch, CMD_MEDIA_GATEWAY_STATE | 0);
  OUT_BATCH(state->batch, (barrierID << 16) | thread_n);
  ADVANCE_BATCH(state->batch);
}

LOCAL void
gpgpu_set_perf_counters(genx_gpgpu_state_t *state, dri_bo *perf)
{
  if (state->perf_b.bo)
    drm_intel_bo_unreference(state->perf_b.bo);
  drm_intel_bo_reference(perf);
  state->perf_b.bo = perf;
}

LOCAL void
gpgpu_run(genx_gpgpu_state_t *state, int32_t ki)
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
gpgpu_run_with_inline(genx_gpgpu_state_t *state, int32_t ki, size_t sz)
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

