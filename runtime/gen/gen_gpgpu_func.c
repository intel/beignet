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
 */

#include <sys/utsname.h>

/* Cache control options for gen7 */
typedef enum cl_cache_control {
  cc_gtt = 0x0,
  cc_l3 = 0x1,
  cc_llc = 0x2,
  cc_llc_l3 = 0x3
} cl_cache_control;

/* LLCCC Cache control options for gen75 */
typedef enum cl_llccc_cache_control {
  llccc_pte = 0x0 << 1,
  llccc_uc = 0x1 << 1,
  llccc_ec = 0x2 << 1,
  llccc_ucllc = 0x3 << 1
} cl_llccc_cache_control;

/* L3 Cache control options for gen75 */
typedef enum cl_l3_cache_control {
  l3cc_uc = 0x0,
  l3cc_ec = 0x1
} cl_l3_cache_control;

/* Target Cache control options for gen8 */
typedef enum cl_target_cache_control {
  tcc_ec_only = 0x0 << 3,
  tcc_llc_only = 0x1 << 3,
  tcc_llc_ec = 0x2 << 3,
  tcc_llc_ec_l3 = 0x3 << 3
} cl_target_cache_control;

/* Memory type LLC/ELLC Cache control options for gen8 */
typedef enum cl_mtllc_cache_control {
  mtllc_pte = 0x0 << 5,
  mtllc_none = 0x1 << 5,
  mtllc_wt = 0x2 << 5,
  mtllc_wb = 0x3 << 5
} cl_mtllc_cache_control;

/* Various limitations we should remove actually */
#define GEN_MAX_SURFACES 256
#define GEN_MAX_SAMPLERS 16
#define GEN_MAX_VME_STATES 8

/* Describe texture tiling */
typedef enum cl_gpgpu_tiling {
  GPGPU_NO_TILE = 0,
  GPGPU_TILE_X = 1,
  GPGPU_TILE_Y = 2,
} cl_gpgpu_tiling;

static uint32_t
__gen_gpgpu_get_cache_ctrl_gen7(void)
{
  return cc_llc_l3;
}

static uint32_t
__gen_gpgpu_get_cache_ctrl_gen75(void)
{
  return llccc_ec | l3cc_ec;
}

static uint32_t
__gen_gpgpu_get_cache_ctrl_gen8(void)
{
  return tcc_llc_ec_l3 | mtllc_wb;
}

static uint32_t
__gen_gpgpu_get_cache_ctrl_gen9(void)
{
  //Kernel-defined cache control registers 2:
  //L3CC: WB; LeCC: WB; TC: LLC/eLLC;
  int major = 0, minor = 0;
  int mocs_index = 0x2;

  struct utsname buf;
  uname(&buf);
  sscanf(buf.release, "%d.%d", &major, &minor);
  //From linux 4.3, kernel redefined the mocs table's value,
  //But before 4.3, still used the hw defautl value.
  if (strcmp(buf.sysname, "Linux") == 0 &&
      major == 4 && minor < 3) { /* linux kernel support skl from  4.x, so check from 4 */
    mocs_index = 0x9;
  }

  return (mocs_index << 1);
}

static uint32_t
gen_gpgpu_get_cache_ctrl(gen_gpgpu *gpgpu)
{
  if (IS_BROADWELL(gpgpu->device->device_id) || IS_CHERRYVIEW(gpgpu->device->device_id))
    return __gen_gpgpu_get_cache_ctrl_gen8();

  if (IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_get_cache_ctrl_gen9();

  if (IS_HASWELL(gpgpu->device->device_id))
    return __gen_gpgpu_get_cache_ctrl_gen75();

  if (IS_IVYBRIDGE(gpgpu->device->device_id))
    return __gen_gpgpu_get_cache_ctrl_gen7();

  assert(0);
  return 0;
}

static void
__gen_gpgpu_setup_bti_gen7(gen_gpgpu *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                           size_t size, unsigned char index, uint32_t format)
{
  assert(size <= (2ul << 30));
  size_t s = size - 1;
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  gen7_surface_state_t *ss0 = (gen7_surface_state_t *)&heap->surface[index * sizeof(gen7_surface_state_t)];
  memset(ss0, 0, sizeof(gen7_surface_state_t));
  ss0->ss0.surface_type = I965_SURFACE_BUFFER;
  ss0->ss0.surface_format = format;
  ss0->ss2.width = s & 0x7f; /* bits 6:0 of sz */
  // Per bspec, I965_SURFACE_BUFFER and RAW format, size must be a multiple of 4 byte.
  if (format == I965_SURFACEFORMAT_RAW)
    assert((ss0->ss2.width & 0x03) == 3);
  ss0->ss2.height = (s >> 7) & 0x3fff; /* bits 20:7 of sz */
  ss0->ss3.depth = (s >> 21) & 0x3ff;  /* bits 30:21 of sz */
  ss0->ss5.cache_control = gen_gpgpu_get_cache_ctrl(gpgpu);
  heap->binding_table[index] = offsetof(surface_heap_t, surface) + index * sizeof(gen7_surface_state_t);

  ss0->ss1.base_addr = buf->offset + internal_offset;
  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    internal_offset,
                    gpgpu->aux.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen7_surface_state_t, ss1),
                    buf);
}

static void
__gen_gpgpu_setup_bti_gen75(gen_gpgpu *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                            size_t size, unsigned char index, uint32_t format)
{
  assert(size <= (2ul << 30));
  size_t s = size - 1;
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  gen7_surface_state_t *ss0 = (gen7_surface_state_t *)&heap->surface[index * sizeof(gen7_surface_state_t)];
  memset(ss0, 0, sizeof(gen7_surface_state_t));
  ss0->ss0.surface_type = I965_SURFACE_BUFFER;
  ss0->ss0.surface_format = format;
  if (format != I965_SURFACEFORMAT_RAW) {
    ss0->ss7.shader_r = I965_SURCHAN_SELECT_RED;
    ss0->ss7.shader_g = I965_SURCHAN_SELECT_GREEN;
    ss0->ss7.shader_b = I965_SURCHAN_SELECT_BLUE;
    ss0->ss7.shader_a = I965_SURCHAN_SELECT_ALPHA;
  }
  ss0->ss2.width = s & 0x7f; /* bits 6:0 of sz */
  // Per bspec, I965_SURFACE_BUFFER and RAW format, size must be a multiple of 4 byte.
  if (format == I965_SURFACEFORMAT_RAW)
    assert((ss0->ss2.width & 0x03) == 3);
  ss0->ss2.height = (s >> 7) & 0x3fff; /* bits 20:7 of sz */
  ss0->ss3.depth = (s >> 21) & 0x3ff;  /* bits 30:21 of sz */
  ss0->ss5.cache_control = gen_gpgpu_get_cache_ctrl(gpgpu);
  heap->binding_table[index] = offsetof(surface_heap_t, surface) + index * sizeof(gen7_surface_state_t);

  ss0->ss1.base_addr = buf->offset + internal_offset;
  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    internal_offset,
                    gpgpu->aux.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen7_surface_state_t, ss1),
                    buf);
}

static void
__gen_gpgpu_setup_bti_gen8(gen_gpgpu *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                           size_t size, unsigned char index, uint32_t format)
{
  assert(size <= (2ul << 30));
  size_t s = size - 1;
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  gen8_surface_state_t *ss0 = (gen8_surface_state_t *)&heap->surface[index * sizeof(gen8_surface_state_t)];
  memset(ss0, 0, sizeof(gen8_surface_state_t));
  ss0->ss0.surface_type = I965_SURFACE_BUFFER;
  ss0->ss0.surface_format = format;
  if (format != I965_SURFACEFORMAT_RAW) {
    ss0->ss7.shader_channel_select_red = I965_SURCHAN_SELECT_RED;
    ss0->ss7.shader_channel_select_green = I965_SURCHAN_SELECT_GREEN;
    ss0->ss7.shader_channel_select_blue = I965_SURCHAN_SELECT_BLUE;
    ss0->ss7.shader_channel_select_alpha = I965_SURCHAN_SELECT_ALPHA;
  }
  ss0->ss2.width = s & 0x7f; /* bits 6:0 of sz */
  // Per bspec, I965_SURFACE_BUFFER and RAW format, size must be a multiple of 4 byte.
  if (format == I965_SURFACEFORMAT_RAW)
    assert((ss0->ss2.width & 0x03) == 3);
  ss0->ss2.height = (s >> 7) & 0x3fff; /* bits 20:7 of sz */
  ss0->ss3.depth = (s >> 21) & 0x3ff;  /* bits 30:21 of sz */
  ss0->ss1.mem_obj_ctrl_state = gen_gpgpu_get_cache_ctrl(gpgpu);
  heap->binding_table[index] = offsetof(surface_heap_t, surface) + index * sizeof(gen8_surface_state_t);
  ss0->ss8.surface_base_addr_lo = (buf->offset64 + internal_offset) & 0xffffffff;
  ss0->ss9.surface_base_addr_hi = ((buf->offset64 + internal_offset) >> 32) & 0xffffffff;
  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    internal_offset,
                    gpgpu->aux.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen8_surface_state_t, ss8),
                    buf);
}

static void
__gen_gpgpu_setup_bti_gen9(gen_gpgpu *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                           size_t size, unsigned char index, uint32_t format)
{
  assert(size <= (4ul << 30));
  size_t s = size - 1;
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  gen8_surface_state_t *ss0 = (gen8_surface_state_t *)&heap->surface[index * sizeof(gen8_surface_state_t)];
  memset(ss0, 0, sizeof(gen8_surface_state_t));
  ss0->ss0.surface_type = I965_SURFACE_BUFFER;
  ss0->ss0.surface_format = format;
  if (format != I965_SURFACEFORMAT_RAW) {
    ss0->ss7.shader_channel_select_red = I965_SURCHAN_SELECT_RED;
    ss0->ss7.shader_channel_select_green = I965_SURCHAN_SELECT_GREEN;
    ss0->ss7.shader_channel_select_blue = I965_SURCHAN_SELECT_BLUE;
    ss0->ss7.shader_channel_select_alpha = I965_SURCHAN_SELECT_ALPHA;
  }
  ss0->ss2.width = s & 0x7f; /* bits 6:0 of sz */
  // Per bspec, I965_SURFACE_BUFFER and RAW format, size must be a multiple of 4 byte.
  if (format == I965_SURFACEFORMAT_RAW)
    assert((ss0->ss2.width & 0x03) == 3);
  ss0->ss2.height = (s >> 7) & 0x3fff; /* bits 20:7 of sz */
  ss0->ss3.depth = (s >> 21) & 0x7ff;  /* bits 31:21 of sz, from bespec only gen 9 support that*/
  ss0->ss1.mem_obj_ctrl_state = gen_gpgpu_get_cache_ctrl(gpgpu);
  heap->binding_table[index] = offsetof(surface_heap_t, surface) + index * sizeof(gen8_surface_state_t);
  ss0->ss8.surface_base_addr_lo = (buf->offset64 + internal_offset) & 0xffffffff;
  ss0->ss9.surface_base_addr_hi = ((buf->offset64 + internal_offset) >> 32) & 0xffffffff;
  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    internal_offset,
                    gpgpu->aux.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen8_surface_state_t, ss8),
                    buf);
}

static void
gen_gpgpu_setup_bti(gen_gpgpu *gpgpu, drm_intel_bo *buf, uint32_t internal_offset,
                    size_t size, unsigned char index, uint32_t format)
{
  if (IS_BROADWELL(gpgpu->device->device_id) || IS_CHERRYVIEW(gpgpu->device->device_id))
    return __gen_gpgpu_setup_bti_gen8(gpgpu, buf, internal_offset, size, index, format);

  if (IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_setup_bti_gen9(gpgpu, buf, internal_offset, size, index, format);

  if (IS_HASWELL(gpgpu->device->device_id))
    return __gen_gpgpu_setup_bti_gen75(gpgpu, buf, internal_offset, size, index, format);

  if (IS_IVYBRIDGE(gpgpu->device->device_id))
    return __gen_gpgpu_setup_bti_gen7(gpgpu, buf, internal_offset, size, index, format);

  assert(0);
}

static cl_int
gen_gpgpu_setup_aux(gen_gpgpu *gpu)
{
  uint32_t size_aux = 0;

  /* begin with surface heap to make sure it's page aligned,
     because state base address use 20bit for the address */
  gpu->aux.surface_heap_offset = size_aux;
  size_aux += sizeof(surface_heap_t);

  //curbe must be 32 bytes aligned
  size_aux = ALIGN(size_aux, 64);
  gpu->aux.curbe_offset = size_aux;
  size_aux += gpu->thread.num_cs_entries * gpu->thread.size_cs_entry * 32;

  //idrt must be 32 bytes aligned
  size_aux = ALIGN(size_aux, 32);
  gpu->aux.idrt_offset = size_aux;
  size_aux += MAX_IF_DESC * sizeof(struct gen6_interface_descriptor);

  //must be 32 bytes aligned
  //sampler state and vme state share the same buffer,
  size_aux = ALIGN(size_aux, 32);
  gpu->aux.sampler_state_offset = size_aux;
  size_aux += MAX(GEN_MAX_SAMPLERS * sizeof(gen6_sampler_state_t),
                  GEN_MAX_VME_STATES * sizeof(gen7_vme_state_t));

  //sampler border color state must be 32 bytes aligned
  size_aux = ALIGN(size_aux, 32);
  gpu->aux.sampler_border_color_state_offset = size_aux;
  size_aux += GEN_MAX_SAMPLERS * sizeof(gen7_sampler_border_color_t);

  /* make sure aux buffer is page aligned */
  size_aux = ALIGN(size_aux, 4096);

  gpu->aux.aux_bo = dri_bo_alloc(gpu->bufmgr, "AUX_BUFFER", size_aux, 4096);
  if (gpu->aux.aux_bo == NULL)
    return CL_OUT_OF_RESOURCES;

  if (dri_bo_map(gpu->aux.aux_bo, 1) != 0)
    return CL_OUT_OF_RESOURCES;

  memset(gpu->aux.aux_bo->virtual, 0, size_aux);
  return CL_SUCCESS;
}

static void
gen_gpgpu_finish_aux(gen_gpgpu *gpu)
{
  if (gpu->aux.aux_bo && gpu->aux.aux_bo->virtual)
    dri_bo_unmap(gpu->aux.aux_bo);
}

static void
__gen_gpgpu_set_buf_reloc_gen7(gen_gpgpu *gpgpu, int32_t index, dri_bo *obj_bo, uint32_t obj_bo_offset)
{
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  heap->binding_table[index] = offsetof(surface_heap_t, surface) +
                               index * sizeof(gen7_surface_state_t);
  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    obj_bo_offset,
                    gpgpu->aux.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen7_surface_state_t, ss1),
                    obj_bo);
}

static int
__gen_is_surface_array(cl_mem_object_type type)
{
  if (type == CL_MEM_OBJECT_IMAGE1D_ARRAY ||
      type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
    return 1;

  return 0;
}

static int
__get_surface_type(cl_mem_object_type type)
{
  switch (type) {
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
    return I965_SURFACE_1D;

  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
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
static uint32_t
__gen_get_surface_type(gen_gpgpu *gpgpu, int index, cl_mem_object_type type)
{
  uint32_t surface_type;
  //Now all platforms need it, so disable platform, re-enable it
  //when some platform don't need this workaround
  if (/*((IS_IVYBRIDGE(gpgpu->drv->device_id) ||
        IS_HASWELL(gpgpu->drv->device_id) ||
        IS_BROADWELL(gpgpu->drv->device_id) ||
        IS_CHERRYVIEW(gpgpu->drv->device_id) ||
        IS_SKYLAKE(gpgpu->drv->device_id) ||
        IS_BROXTON(gpgpu->drv->device_id) ||
        IS_KABYLAKE(gpgpu->drv_device_id))) && */
      index >= BTI_WORKAROUND_IMAGE_OFFSET + BTI_RESERVED_NUM &&
      type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
    surface_type = I965_SURFACE_2D;
  else
    surface_type = __get_surface_type(type);
  return surface_type;
}

static void
__gen_gpgpu_bind_image_gen7(gen_gpgpu *gpgpu, uint32_t index, dri_bo *obj_bo, uint32_t obj_bo_offset,
                            uint32_t format, cl_mem_object_type type, uint32_t bpp, int32_t w, int32_t h,
                            int32_t depth, int32_t pitch, int32_t slice_pitch, int32_t tiling)
{
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  gen7_surface_state_t *ss = (gen7_surface_state_t *)&heap->surface[index * sizeof(gen7_surface_state_t)];

  memset(ss, 0, sizeof(*ss));
  ss->ss0.vertical_line_stride = 0; // always choose VALIGN_2
  ss->ss0.surface_type = __gen_get_surface_type(gpgpu, index, type);
  if (__gen_is_surface_array(type)) {
    ss->ss0.surface_array = 1;
    ss->ss0.surface_array_spacing = 1;
  }
  ss->ss0.surface_format = format;
  ss->ss1.base_addr = obj_bo->offset + obj_bo_offset;
  ss->ss2.width = w - 1;

  ss->ss2.height = h - 1;
  ss->ss3.depth = depth - 1;
  ss->ss4.not_str_buf.rt_view_extent = depth - 1;
  ss->ss4.not_str_buf.min_array_element = 0;
  ss->ss3.pitch = pitch - 1;
  ss->ss5.cache_control = gen_gpgpu_get_cache_ctrl(gpgpu);
  if (tiling == GPGPU_TILE_X) {
    ss->ss0.tiled_surface = 1;
    ss->ss0.tile_walk = I965_TILEWALK_XMAJOR;
  } else if (tiling == GPGPU_TILE_Y) {
    ss->ss0.tiled_surface = 1;
    ss->ss0.tile_walk = I965_TILEWALK_YMAJOR;
  }
  ss->ss0.render_cache_rw_mode = 1; /* XXX do we need to set it? */
  __gen_gpgpu_set_buf_reloc_gen7(gpgpu, index, obj_bo, obj_bo_offset);

  assert(index < GEN_MAX_SURFACES);
}

static void
__gen_gpgpu_bind_image_gen75(gen_gpgpu *gpgpu, uint32_t index, dri_bo *obj_bo, uint32_t obj_bo_offset,
                             uint32_t format, cl_mem_object_type type, uint32_t bpp, int32_t w, int32_t h,
                             int32_t depth, int32_t pitch, int32_t slice_pitch, int32_t tiling)
{
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  gen7_surface_state_t *ss = (gen7_surface_state_t *)&heap->surface[index * sizeof(gen7_surface_state_t)];
  memset(ss, 0, sizeof(*ss));
  ss->ss0.vertical_line_stride = 0; // always choose VALIGN_2
  ss->ss0.surface_type = __gen_get_surface_type(gpgpu, index, type);
  if (__gen_is_surface_array(type)) {
    ss->ss0.surface_array = 1;
    ss->ss0.surface_array_spacing = 1;
  }
  ss->ss0.surface_format = format;
  ss->ss1.base_addr = obj_bo->offset + obj_bo_offset;
  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.depth = depth - 1;
  ss->ss4.not_str_buf.rt_view_extent = depth - 1;
  ss->ss4.not_str_buf.min_array_element = 0;
  ss->ss3.pitch = pitch - 1;
  ss->ss5.cache_control = gen_gpgpu_get_cache_ctrl(gpgpu);
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
  __gen_gpgpu_set_buf_reloc_gen7(gpgpu, index, obj_bo, obj_bo_offset);

  assert(index < GEN_MAX_SURFACES);
}

static void
__gen_gpgpu_bind_image_gen8(gen_gpgpu *gpgpu, uint32_t index, dri_bo *obj_bo, uint32_t obj_bo_offset,
                            uint32_t format, cl_mem_object_type type, uint32_t bpp, int32_t w, int32_t h,
                            int32_t depth, int32_t pitch, int32_t slice_pitch, int32_t tiling)
{
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  gen8_surface_state_t *ss = (gen8_surface_state_t *)&heap->surface[index * sizeof(gen8_surface_state_t)];
  memset(ss, 0, sizeof(*ss));
  ss->ss0.vertical_line_stride = 0; // always choose VALIGN_2
  ss->ss0.surface_type = __gen_get_surface_type(gpgpu, index, type);
  ss->ss0.surface_format = format;
  if (__gen_is_surface_array(type)) {
    ss->ss0.surface_array = 1;
    ss->ss1.surface_qpitch = (h + 3) / 4;
  }
  ss->ss0.horizontal_alignment = 1;
  ss->ss0.vertical_alignment = 1;

  if (tiling == GPGPU_TILE_X) {
    ss->ss0.tile_mode = GEN8_TILEMODE_XMAJOR;
  } else if (tiling == GPGPU_TILE_Y) {
    ss->ss0.tile_mode = GEN8_TILEMODE_YMAJOR;
  } else
    assert(tiling == GPGPU_NO_TILE); // W mode is not supported now.

  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.depth = depth - 1;

  ss->ss8.surface_base_addr_lo = (obj_bo->offset64 + obj_bo_offset) & 0xffffffff;
  ss->ss9.surface_base_addr_hi = ((obj_bo->offset64 + obj_bo_offset) >> 32) & 0xffffffff;

  ss->ss4.render_target_view_ext = depth - 1;
  ss->ss4.min_array_elt = 0;
  ss->ss3.surface_pitch = pitch - 1;

  ss->ss1.mem_obj_ctrl_state = gen_gpgpu_get_cache_ctrl(gpgpu);
  ss->ss7.shader_channel_select_red = I965_SURCHAN_SELECT_RED;
  ss->ss7.shader_channel_select_green = I965_SURCHAN_SELECT_GREEN;
  ss->ss7.shader_channel_select_blue = I965_SURCHAN_SELECT_BLUE;
  ss->ss7.shader_channel_select_alpha = I965_SURCHAN_SELECT_ALPHA;
  ss->ss0.render_cache_rw_mode = 1; /* XXX do we need to set it? */

  heap->binding_table[index] = offsetof(surface_heap_t, surface) +
                               index * surface_state_sz;
  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    obj_bo_offset,
                    gpgpu->aux.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen8_surface_state_t, ss8),
                    obj_bo);

  assert(index < GEN_MAX_SURFACES);
}

static void
__gen_gpgpu_bind_image_gen9(gen_gpgpu *gpgpu, uint32_t index, dri_bo *obj_bo, uint32_t obj_bo_offset,
                            uint32_t format, cl_mem_object_type type, uint32_t bpp, int32_t w, int32_t h,
                            int32_t depth, int32_t pitch, int32_t slice_pitch, int32_t tiling)
{
  surface_heap_t *heap = gpgpu->aux.aux_bo->virtual + gpgpu->aux.surface_heap_offset;
  gen8_surface_state_t *ss = (gen8_surface_state_t *)&heap->surface[index * sizeof(gen8_surface_state_t)];
  memset(ss, 0, sizeof(*ss));
  ss->ss0.vertical_line_stride = 0; // always choose VALIGN_2
  ss->ss0.surface_type = __gen_get_surface_type(gpgpu, index, type);
  ss->ss0.surface_format = format;
  if (__gen_is_surface_array(type) && ss->ss0.surface_type == I965_SURFACE_1D) {
    ss->ss0.surface_array = 1;
    ss->ss1.surface_qpitch = (slice_pitch / bpp + 3) / 4; //align_h
  }

  if (__gen_is_surface_array(type) && ss->ss0.surface_type == I965_SURFACE_2D) {
    ss->ss0.surface_array = 1;
    ss->ss1.surface_qpitch = (slice_pitch / pitch + 3) / 4;
  }

  if (ss->ss0.surface_type == I965_SURFACE_3D)
    ss->ss1.surface_qpitch = (slice_pitch / pitch + 3) / 4;

  ss->ss0.horizontal_alignment = 1;
  ss->ss0.vertical_alignment = 1;

  if (tiling == GPGPU_TILE_X) {
    ss->ss0.tile_mode = GEN8_TILEMODE_XMAJOR;
  } else if (tiling == GPGPU_TILE_Y) {
    ss->ss0.tile_mode = GEN8_TILEMODE_YMAJOR;
  } else
    assert(tiling == GPGPU_NO_TILE); // W mode is not supported now.

  ss->ss2.width = w - 1;
  ss->ss2.height = h - 1;
  ss->ss3.depth = depth - 1;

  ss->ss8.surface_base_addr_lo = (obj_bo->offset64 + obj_bo_offset) & 0xffffffff;
  ss->ss9.surface_base_addr_hi = ((obj_bo->offset64 + obj_bo_offset) >> 32) & 0xffffffff;

  ss->ss4.render_target_view_ext = depth - 1;
  ss->ss4.min_array_elt = 0;
  ss->ss3.surface_pitch = pitch - 1;

  ss->ss1.mem_obj_ctrl_state = gen_gpgpu_get_cache_ctrl(gpgpu);
  ss->ss7.shader_channel_select_red = I965_SURCHAN_SELECT_RED;
  ss->ss7.shader_channel_select_green = I965_SURCHAN_SELECT_GREEN;
  ss->ss7.shader_channel_select_blue = I965_SURCHAN_SELECT_BLUE;
  ss->ss7.shader_channel_select_alpha = I965_SURCHAN_SELECT_ALPHA;
  ss->ss0.render_cache_rw_mode = 1; /* XXX do we need to set it? */

  heap->binding_table[index] = offsetof(surface_heap_t, surface) +
                               index * surface_state_sz;
  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_RENDER,
                    I915_GEM_DOMAIN_RENDER,
                    obj_bo_offset,
                    gpgpu->aux.surface_heap_offset +
                      heap->binding_table[index] +
                      offsetof(gen8_surface_state_t, ss8),
                    obj_bo);

  assert(index < GEN_MAX_SURFACES);
}

static void
gen_gpgpu_bind_image(gen_gpgpu *gpgpu, uint32_t index, dri_bo *obj_bo, uint32_t obj_bo_offset,
                     uint32_t format, cl_mem_object_type type, uint32_t bpp, int32_t w, int32_t h,
                     int32_t depth, int32_t pitch, int32_t slice_pitch, int32_t tiling)
{
  if (IS_BROADWELL(gpgpu->device->device_id) || IS_CHERRYVIEW(gpgpu->device->device_id))
    return __gen_gpgpu_bind_image_gen8(gpgpu, index, obj_bo, obj_bo_offset, format, type, bpp,
                                       w, h, depth, pitch, slice_pitch, tiling);

  if (IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_bind_image_gen9(gpgpu, index, obj_bo, obj_bo_offset, format, type, bpp,
                                       w, h, depth, pitch, slice_pitch, tiling);

  if (IS_HASWELL(gpgpu->device->device_id))
    return __gen_gpgpu_bind_image_gen75(gpgpu, index, obj_bo, obj_bo_offset, format, type, bpp,
                                        w, h, depth, pitch, slice_pitch, tiling);

  if (IS_IVYBRIDGE(gpgpu->device->device_id))
    return __gen_gpgpu_bind_image_gen7(gpgpu, index, obj_bo, obj_bo_offset, format, type, bpp,
                                       w, h, depth, pitch, slice_pitch, tiling);

  assert(0);
}

static int __translate_wrap_mode(uint32_t cl_address_mode, int using_nearest)
{
  switch (cl_address_mode) {
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
__gen_gpgpu_insert_sampler_gen7(gen_gpgpu *gpgpu, uint32_t index, uint32_t clk_sampler)
{
  int using_nearest = 0;
  uint32_t wrap_mode;
  gen7_sampler_state_t *sampler;

  sampler = (gen7_sampler_state_t *)(gpgpu->aux.aux_bo->virtual + gpgpu->aux.sampler_state_offset) + index;
  memset(sampler, 0, sizeof(*sampler));
  assert((gpgpu->aux.aux_bo->offset + gpgpu->aux.sampler_border_color_state_offset) % 32 == 0);
  sampler->ss2.default_color_pointer = (gpgpu->aux.aux_bo->offset + gpgpu->aux.sampler_border_color_state_offset) >> 5;
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

  wrap_mode = __translate_wrap_mode(clk_sampler & __CLK_ADDRESS_MASK, using_nearest);
  sampler->ss3.s_wrap_mode = wrap_mode;
  /* XXX mesa i965 driver code point out that if the surface is a 1D surface, we may need
   * to set t_wrap_mode to GEN_TEXCOORDMODE_WRAP. */
  sampler->ss3.t_wrap_mode = wrap_mode;
  sampler->ss3.r_wrap_mode = wrap_mode;

  sampler->ss0.lod_preclamp = 1;       /* OpenGL mode */
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

  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_SAMPLER, 0,
                    gpgpu->aux.sampler_border_color_state_offset,
                    gpgpu->aux.sampler_state_offset +
                      index * sizeof(gen7_sampler_state_t) +
                      offsetof(gen7_sampler_state_t, ss2),
                    gpgpu->aux.aux_bo);
}

static void
__gen_gpgpu_insert_sampler_gen8(gen_gpgpu *gpgpu, uint32_t index, uint32_t clk_sampler)
{
  int using_nearest = 0;
  uint32_t wrap_mode;
  gen8_sampler_state_t *sampler;

  sampler = (gen8_sampler_state_t *)(gpgpu->aux.aux_bo->virtual + gpgpu->aux.sampler_state_offset) + index;
  memset(sampler, 0, sizeof(*sampler));
  assert((gpgpu->aux.aux_bo->offset + gpgpu->aux.sampler_border_color_state_offset) % 32 == 0);
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

  wrap_mode = __translate_wrap_mode(clk_sampler & __CLK_ADDRESS_MASK, using_nearest);
  sampler->ss3.s_wrap_mode = wrap_mode;
  /* XXX mesa i965 driver code point out that if the surface is a 1D surface, we may need
   * to set t_wrap_mode to GEN_TEXCOORDMODE_WRAP. */
  sampler->ss3.t_wrap_mode = wrap_mode;
  sampler->ss3.r_wrap_mode = wrap_mode;

  sampler->ss0.lod_preclamp = 1;       /* OpenGL mode */
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
gen_gpgpu_bind_sampler(gen_gpgpu *gpgpu, uint32_t *samplers, size_t sampler_sz)
{
  int index;
  assert(sampler_sz <= GEN_MAX_SAMPLERS);
  cl_uint device_id = gpgpu->device->device_id;

  for (index = 0; index < sampler_sz; index++) {
    if (IS_BROADWELL(device_id) || IS_CHERRYVIEW(device_id) || IS_GEN9(device_id)) {
      __gen_gpgpu_insert_sampler_gen8(gpgpu, index, samplers[index]);
      continue;
    }

    __gen_gpgpu_insert_sampler_gen7(gpgpu, index, samplers[index]);
  }
}

static void
gen_gpgpu_alloc_constant_buffer(gen_gpgpu *gpgpu, uint32_t size, uint8_t bti)
{
  gpgpu->mem.const_bo = drm_intel_bo_alloc(gpgpu->bufmgr, "CONSTANT_BUFFER", size, 64);
  if (gpgpu->mem.const_bo == NULL)
    return;

  gen_gpgpu_setup_bti(gpgpu, gpgpu->mem.const_bo, 0, size, bti, I965_SURFACEFORMAT_R32G32B32A32_UINT);
}

static void
__gen_gpgpu_build_idrt_gen7(gen_gpgpu *gpgpu)
{
  gen6_interface_descriptor_t *desc;

  desc = (gen6_interface_descriptor_t *)(gpgpu->aux.aux_bo->virtual + gpgpu->aux.idrt_offset);

  memset(desc, 0, sizeof(*desc));
  desc->desc0.kernel_start_pointer = (gpgpu->kernel_bo->offset) >> 6; /* reloc */
  desc->desc1.single_program_flow = 0;
  desc->desc1.floating_point_mode = 0; /* use IEEE-754 rule */
  desc->desc5.rounding_mode = 0;       /* round to nearest even */

  assert((gpgpu->aux.aux_bo->offset + gpgpu->aux.sampler_state_offset) % 32 == 0);
  desc->desc2.sampler_state_pointer = (gpgpu->aux.aux_bo->offset + gpgpu->aux.sampler_state_offset) >> 5;
  desc->desc3.binding_table_entry_count = 0; /* no prefetch */
  desc->desc3.binding_table_pointer = 0;
  desc->desc4.curbe_read_len = (gpgpu->thread.curbe_size) / 32;
  desc->desc4.curbe_read_offset = 0;

  /* Barriers / SLM are automatically handled on Gen7+ */
  size_t slm_sz = gpgpu->mem.local_mem_size;
  desc->desc5.group_threads_num = gpgpu->thread.barrier_slm_used ? gpgpu->thread.thread_num : 0;
  desc->desc5.barrier_enable = gpgpu->thread.barrier_slm_used;
  if (slm_sz <= 4 * KB)
    slm_sz = 4 * KB;
  else if (slm_sz <= 8 * KB)
    slm_sz = 8 * KB;
  else if (slm_sz <= 16 * KB)
    slm_sz = 16 * KB;
  else if (slm_sz <= 32 * KB)
    slm_sz = 32 * KB;
  else
    slm_sz = 64 * KB;
  slm_sz = slm_sz >> 12;
  desc->desc5.slm_sz = slm_sz;

  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_INSTRUCTION, 0,
                    0,
                    gpgpu->aux.idrt_offset + offsetof(gen6_interface_descriptor_t, desc0),
                    gpgpu->kernel_bo);

  dri_bo_emit_reloc(gpgpu->aux.aux_bo,
                    I915_GEM_DOMAIN_SAMPLER, 0,
                    gpgpu->aux.sampler_state_offset,
                    gpgpu->aux.idrt_offset + offsetof(gen6_interface_descriptor_t, desc2),
                    gpgpu->aux.aux_bo);
}

static void
__gen_gpgpu_build_idrt_gen8(gen_gpgpu *gpgpu)
{
  gen8_interface_descriptor_t *desc;

  desc = (gen8_interface_descriptor_t *)(gpgpu->aux.aux_bo->virtual + gpgpu->aux.idrt_offset);

  memset(desc, 0, sizeof(*desc));
  desc->desc0.kernel_start_pointer = 0; /* reloc */
  desc->desc2.single_program_flow = 0;
  desc->desc2.floating_point_mode = 0; /* use IEEE-754 rule */
  desc->desc6.rounding_mode = 0;       /* round to nearest even */

  assert((gpgpu->aux.aux_bo->offset + gpgpu->aux.sampler_state_offset) % 32 == 0);
  desc->desc3.sampler_state_pointer = gpgpu->aux.sampler_state_offset >> 5;
  desc->desc4.binding_table_entry_count = 0; /* no prefetch */
  desc->desc4.binding_table_pointer = 0;
  desc->desc5.curbe_read_len = (gpgpu->thread.curbe_size) / 32;
  desc->desc5.curbe_read_offset = 0;

  /* Barriers / SLM are automatically handled on Gen7+ */
  size_t slm_sz = gpgpu->mem.local_mem_size;
  /* group_threads_num should not be set to 0 even if the barrier is disabled per bspec */
  desc->desc6.group_threads_num = gpgpu->thread.thread_num;
  desc->desc6.barrier_enable = gpgpu->thread.barrier_slm_used;
  if (slm_sz == 0)
    slm_sz = 0;
  else if (slm_sz <= 4 * KB)
    slm_sz = 4 * KB;
  else if (slm_sz <= 8 * KB)
    slm_sz = 8 * KB;
  else if (slm_sz <= 16 * KB)
    slm_sz = 16 * KB;
  else if (slm_sz <= 32 * KB)
    slm_sz = 32 * KB;
  else
    slm_sz = 64 * KB;
  slm_sz = slm_sz >> 12;
  desc->desc6.slm_sz = slm_sz;
}

static void
__gen_gpgpu_build_idrt_gen9(gen_gpgpu *gpgpu)
{
  gen8_interface_descriptor_t *desc;

  desc = (gen8_interface_descriptor_t *)(gpgpu->aux.aux_bo->virtual + gpgpu->aux.idrt_offset);

  memset(desc, 0, sizeof(*desc));
  desc->desc0.kernel_start_pointer = 0; /* reloc */
  desc->desc2.single_program_flow = 0;
  desc->desc2.floating_point_mode = 0; /* use IEEE-754 rule */
  desc->desc6.rounding_mode = 0;       /* round to nearest even */

  assert((gpgpu->aux.aux_bo->offset + gpgpu->aux.sampler_state_offset) % 32 == 0);
  desc->desc3.sampler_state_pointer = gpgpu->aux.sampler_state_offset >> 5;
  desc->desc4.binding_table_entry_count = 0; /* no prefetch */
  desc->desc4.binding_table_pointer = 0;
  desc->desc5.curbe_read_len = (gpgpu->thread.curbe_size) / 32;
  desc->desc5.curbe_read_offset = 0;

  /* Barriers / SLM are automatically handled on Gen7+ */
  size_t slm_sz = gpgpu->mem.local_mem_size;
  /* group_threads_num should not be set to 0 even if the barrier is disabled per bspec */
  desc->desc6.group_threads_num = gpgpu->thread.thread_num;
  desc->desc6.barrier_enable = gpgpu->thread.barrier_slm_used;

  if (slm_sz == 0)
    slm_sz = 0;
  else if (slm_sz <= 1 * KB)
    slm_sz = 1;
  else if (slm_sz <= 2 * KB)
    slm_sz = 2;
  else if (slm_sz <= 4 * KB)
    slm_sz = 3;
  else if (slm_sz <= 8 * KB)
    slm_sz = 4;
  else if (slm_sz <= 16 * KB)
    slm_sz = 5;
  else if (slm_sz <= 32 * KB)
    slm_sz = 6;
  else
    slm_sz = 7;
  desc->desc6.slm_sz = slm_sz;
}

static void
gen_gpgpu_build_idrt(gen_gpgpu *gpgpu)
{
  if (IS_BROADWELL(gpgpu->device->device_id) || IS_CHERRYVIEW(gpgpu->device->device_id))
    return __gen_gpgpu_build_idrt_gen8(gpgpu);

  if (IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_build_idrt_gen9(gpgpu);

  return __gen_gpgpu_build_idrt_gen7(gpgpu);

  assert(0);
}

static void
__gen_gpgpu_upload_curbes_gen7(gen_gpgpu *gpgpu, const void *data, uint32_t size)
{
  unsigned char *curbe = NULL;
  uint32_t i, j;

  /* Upload the data first */
  assert(gpgpu->aux.aux_bo->virtual);
  curbe = (unsigned char *)(gpgpu->aux.aux_bo->virtual + gpgpu->aux.curbe_offset);
  memcpy(curbe, data, size);

  /* Now put all the relocations for our flat address space */
  for (i = 0; i < gpgpu->thread.thread_num; ++i)
    for (j = 0; j < gpgpu->mem.binded_n; ++j) {
      *(uint32_t *)(curbe + gpgpu->mem.binded_offset[j] + i * gpgpu->thread.curbe_size) =
        gpgpu->mem.binded_buf[j]->offset64 + gpgpu->mem.target_buf_offset[j];
      drm_intel_bo_emit_reloc(gpgpu->aux.aux_bo,
                              gpgpu->aux.curbe_offset + gpgpu->mem.binded_offset[j] + i * gpgpu->thread.curbe_size,
                              gpgpu->mem.binded_buf[j],
                              gpgpu->mem.target_buf_offset[j],
                              I915_GEM_DOMAIN_RENDER,
                              I915_GEM_DOMAIN_RENDER);
    }
}

static void
__gen_gpgpu_upload_curbes_gen8(gen_gpgpu *gpgpu, const void *data, uint32_t size)
{
  unsigned char *curbe = NULL;
  uint32_t i, j;

  /* Upload the data first */
  assert(gpgpu->aux.aux_bo->virtual);
  curbe = (unsigned char *)(gpgpu->aux.aux_bo->virtual + gpgpu->aux.curbe_offset);
  memcpy(curbe, data, size);

  /* Now put all the relocations for our flat address space */
  for (i = 0; i < gpgpu->thread.thread_num; ++i)
    for (j = 0; j < gpgpu->mem.binded_n; ++j) {
      *(size_t *)(curbe + gpgpu->mem.binded_offset[j] + i * gpgpu->thread.curbe_size) =
        gpgpu->mem.binded_buf[j]->offset64 + gpgpu->mem.target_buf_offset[j];
      drm_intel_bo_emit_reloc(gpgpu->aux.aux_bo,
                              gpgpu->aux.curbe_offset + gpgpu->mem.binded_offset[j] + i * gpgpu->thread.curbe_size,
                              gpgpu->mem.binded_buf[j],
                              gpgpu->mem.target_buf_offset[j],
                              I915_GEM_DOMAIN_RENDER,
                              I915_GEM_DOMAIN_RENDER);
    }
}

static void
gen_gpgpu_upload_curbes(gen_gpgpu *gpgpu, const void *data, uint32_t size)
{
  if (IS_BROADWELL(gpgpu->device->device_id) ||
      IS_CHERRYVIEW(gpgpu->device->device_id) || IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_upload_curbes_gen8(gpgpu, data, size);

  return __gen_gpgpu_upload_curbes_gen7(gpgpu, data, size);
}

static void
__gen_gpgpu_pipe_control_gen7(gen_gpgpu *gpgpu)
{
  gen6_pipe_control_t *pc = (gen6_pipe_control_t *)
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
__gen_gpgpu_pipe_control_gen75(gen_gpgpu *gpgpu)
{
  gen6_pipe_control_t *pc = (gen6_pipe_control_t *)
    intel_batchbuffer_alloc_space(gpgpu->batch, sizeof(gen6_pipe_control_t));
  memset(pc, 0, sizeof(*pc));
  pc->dw0.length = SIZEOF32(gen6_pipe_control_t) - 2;
  pc->dw0.instruction_subopcode = GEN7_PIPE_CONTROL_SUBOPCODE_3D_CONTROL;
  pc->dw0.instruction_opcode = GEN7_PIPE_CONTROL_OPCODE_3D_CONTROL;
  pc->dw0.instruction_pipeline = GEN7_PIPE_CONTROL_3D;
  pc->dw0.instruction_type = GEN7_PIPE_CONTROL_INSTRUCTION_GFX;
  pc->dw1.cs_stall = 1;
  pc->dw1.dc_flush_enable = 1;

  pc = (gen6_pipe_control_t *)
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
  ADVANCE_BATCH(gpgpu->batch);
}

static void
__gen_gpgpu_pipe_control_gen8(gen_gpgpu *gpgpu)
{
  gen8_pipe_control_t *pc = (gen8_pipe_control_t *)
    intel_batchbuffer_alloc_space(gpgpu->batch, sizeof(gen8_pipe_control_t));
  memset(pc, 0, sizeof(*pc));
  pc->dw0.length = SIZEOF32(gen8_pipe_control_t) - 2;
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
gen_gpgpu_pipe_control(gen_gpgpu *gpgpu)
{
  if (IS_BROADWELL(gpgpu->device->device_id) ||
      IS_CHERRYVIEW(gpgpu->device->device_id) || IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_pipe_control_gen8(gpgpu);

  if (IS_HASWELL(gpgpu->device->device_id))
    return __gen_gpgpu_pipe_control_gen75(gpgpu);

  if (IS_IVYBRIDGE(gpgpu->device->device_id))
    return __gen_gpgpu_pipe_control_gen7(gpgpu);

  assert(0);
  return;
}

static const uint32_t gpgpu_l3_config_reg1[] = {
  0x00080040, 0x02040040, 0x00800040, 0x01000038,
  0x02000030, 0x01000038, 0x00000038, 0x00000040,
  0x0A140091, 0x09100091, 0x08900091, 0x08900091,
  0x010000a1,
};

static const uint32_t gpgpu_l3_config_reg2[] = {
  0x00000000, 0x00000000, 0x00080410, 0x00080410,
  0x00040410, 0x00040420, 0x00080420, 0x00080020,
  0x00204080, 0x00244890, 0x00284490, 0x002444A0,
  0x00040810,
};

static void
__gen_gpgpu_set_L3_gen7(gen_gpgpu *gpgpu, uint32_t use_slm)
{
  BEGIN_BATCH(gpgpu->batch, 9);
  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_SQC_REG1_ADDRESS_OFFSET);
  OUT_BATCH(gpgpu->batch, 0x00A00000);

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

  gen_gpgpu_pipe_control(gpgpu);
}

static void
__gen_gpgpu_set_L3_baytrail(gen_gpgpu *gpgpu, uint32_t use_slm)
{
  BEGIN_BATCH(gpgpu->batch, 9);

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_SQC_REG1_ADDRESS_OFFSET);
  OUT_BATCH(gpgpu->batch, 0x00D30000); /* General credit : High credit = 26 : 6 */

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_CNTL_REG2_ADDRESS_OFFSET);
  if (use_slm)
    OUT_BATCH(gpgpu->batch, 0x01020021); /* {SLM=64, URB=96, DC=16, RO=16, Sum=192} */
  else
    OUT_BATCH(gpgpu->batch, 0x02040040); /* {SLM=0, URB=128, DC=32, RO=32, Sum=192} */

  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_CNTL_REG3_ADDRESS_OFFSET);
  OUT_BATCH(gpgpu->batch, 0x0); /* {I/S=0, Const=0, Tex=0} */

  ADVANCE_BATCH(gpgpu->batch);

  gen_gpgpu_pipe_control(gpgpu);
}

static void
__gen_gpgpu_set_L3_gen75(gen_gpgpu *gpgpu, uint32_t use_slm)
{
  /* still set L3 in batch buffer for fulsim. */
  if (gpgpu->atomic_test_result != 0) {
    BEGIN_BATCH(gpgpu->batch, 15);
    OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
    /* FIXME: KMD always disable the atomic in L3 for some reason.
       I checked the spec, and don't think we need that workaround now.
       Before I send a patch to kernel, let's just enable it here. */
    OUT_BATCH(gpgpu->batch, HSW_SCRATCH1_OFFSET);
    OUT_BATCH(gpgpu->batch, 0);                         /* enable atomic in L3 */
    OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
    OUT_BATCH(gpgpu->batch, HSW_ROW_CHICKEN3_HDC_OFFSET);
    OUT_BATCH(gpgpu->batch, (1 << 6ul) << 16); /* enable atomic in L3 */
  } else {
    BEGIN_BATCH(gpgpu->batch, 9);
  }
  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN7_L3_SQC_REG1_ADDRESS_OFFSET);
  OUT_BATCH(gpgpu->batch, 0x08800000);

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
  gen_gpgpu_pipe_control(gpgpu);
}

static void
__gen_gpgpu_set_L3_gen8(gen_gpgpu *gpgpu, uint32_t use_slm)
{
  BEGIN_BATCH(gpgpu->batch, 3);
  OUT_BATCH(gpgpu->batch, CMD_LOAD_REGISTER_IMM | 1); /* length - 2 */
  OUT_BATCH(gpgpu->batch, GEN8_L3_CNTL_REG_ADDRESS_OFFSET);
  // FIXME, this is a workaround for switch SLM enable and disable random hang
  if (use_slm)
    OUT_BATCH(gpgpu->batch, 0x60000121); /* {SLM=192, URB=128, Rest=384} */
  else
    OUT_BATCH(gpgpu->batch, 0x60000160); /* {SLM=0, URB=384, Rest=384, Sum=768} */

  //if(use_slm)
  //  gpgpu->batch->enable_slm = 1;
  gen_gpgpu_pipe_control(gpgpu);
}

static void
gen_gpgpu_set_L3(gen_gpgpu *gpgpu, uint32_t use_slm)
{
  if (IS_BROADWELL(gpgpu->device->device_id) ||
      IS_CHERRYVIEW(gpgpu->device->device_id) || IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_set_L3_gen8(gpgpu, use_slm);

  if (IS_HASWELL(gpgpu->device->device_id))
    return __gen_gpgpu_set_L3_gen75(gpgpu, use_slm);

  if (IS_BAYTRAIL_T(gpgpu->device->device_id))
    return __gen_gpgpu_set_L3_baytrail(gpgpu, use_slm);

  if (IS_IVYBRIDGE(gpgpu->device->device_id))
    return __gen_gpgpu_set_L3_gen7(gpgpu, use_slm);

  assert(0);
  return;
}

static void
__gen_gpgpu_select_pipeline_gen7(gen_gpgpu *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 1);
  OUT_BATCH(gpgpu->batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_GPGPU);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
__gen_gpgpu_select_pipeline_gen9(gen_gpgpu *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 1);
  OUT_BATCH(gpgpu->batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MASK | PIPELINE_SELECT_GPGPU);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
gen_gpgpu_select_pipeline(gen_gpgpu *gpgpu)
{
  if (IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_select_pipeline_gen9(gpgpu);

  return __gen_gpgpu_select_pipeline_gen7(gpgpu);
}

static void
__gen_gpgpu_set_base_address_gen7(gen_gpgpu *gpgpu)
{
  const uint32_t def_cc = gen_gpgpu_get_cache_ctrl(gpgpu); /* default Cache Control value */
  BEGIN_BATCH(gpgpu->batch, 10);
  OUT_BATCH(gpgpu->batch, CMD_STATE_BASE_ADDRESS | 8);
  /* 0, Gen State Mem Obj CC, Stateless Mem Obj CC, Stateless Access Write Back */
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 8) | (def_cc << 4) | (0 << 3) | BASE_ADDRESS_MODIFY); /* General State Base Addr   */
  /* 0, State Mem Obj CC */
  /* We use a state base address for the surface heap since IVB clamp the
   * binding table pointer at 11 bits. So, we cannot use pointers directly while
   * using the surface heap
   */
  assert(gpgpu->aux.surface_heap_offset % 4096 == 0);
  OUT_RELOC(gpgpu->batch, gpgpu->aux.aux_bo,
            I915_GEM_DOMAIN_INSTRUCTION,
            I915_GEM_DOMAIN_INSTRUCTION,
            gpgpu->aux.surface_heap_offset + (0 | (def_cc << 8) | (def_cc << 4) | (0 << 3) | BASE_ADDRESS_MODIFY));

  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Dynamic State Base Addr */

  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Indirect Obj Base Addr */
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 8) | BASE_ADDRESS_MODIFY); /* Instruction Base Addr  */
  OUT_BATCH(gpgpu->batch, 0 | BASE_ADDRESS_MODIFY);
  /* According to mesa i965 driver code, we must set the dynamic state access upper bound
   * to a valid bound value, otherwise, the border color pointer may be rejected and you
   * may get incorrect border color. This is a known hardware bug. */
  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0 | BASE_ADDRESS_MODIFY);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
__gen_gpgpu_set_base_address_gen8(gen_gpgpu *gpgpu)
{
  const uint32_t def_cc = gen_gpgpu_get_cache_ctrl(gpgpu); /* default Cache Control value */
  BEGIN_BATCH(gpgpu->batch, 16);
  OUT_BATCH(gpgpu->batch, CMD_STATE_BASE_ADDRESS | 14);
  /* 0, Gen State Mem Obj CC, Stateless Mem Obj CC, Stateless Access Write Back */
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | (0 << 1) | BASE_ADDRESS_MODIFY); /* General State Base Addr   */
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 16));
  /* 0, State Mem Obj CC */
  /* We use a state base address for the surface heap since IVB clamp the
     * binding table pointer at 11 bits. So, we cannot use pointers directly while
     * using the surface heap
     */
  assert(gpgpu->aux.surface_heap_offset % 4096 == 0);
  OUT_RELOC(gpgpu->batch, gpgpu->aux.aux_bo,
            I915_GEM_DOMAIN_SAMPLER,
            I915_GEM_DOMAIN_SAMPLER,
            gpgpu->aux.surface_heap_offset + (0 | (def_cc << 4) | (0 << 1) | BASE_ADDRESS_MODIFY));
  OUT_BATCH(gpgpu->batch, 0);
  OUT_RELOC(gpgpu->batch, gpgpu->aux.aux_bo,
            I915_GEM_DOMAIN_RENDER,
            I915_GEM_DOMAIN_RENDER,
            (0 | (def_cc << 4) | (0 << 1) | BASE_ADDRESS_MODIFY)); /* Dynamic State Base Addr */
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | BASE_ADDRESS_MODIFY); /* Indirect Obj Base Addr */
  OUT_BATCH(gpgpu->batch, 0);
  //OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | BASE_ADDRESS_MODIFY); /* Instruction Base Addr  */
  OUT_RELOC(gpgpu->batch, gpgpu->kernel_bo,
            I915_GEM_DOMAIN_INSTRUCTION,
            I915_GEM_DOMAIN_INSTRUCTION,
            0 + (0 | (def_cc << 4) | (0 << 1) | BASE_ADDRESS_MODIFY));
  OUT_BATCH(gpgpu->batch, 0);

  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  /* According to mesa i965 driver code, we must set the dynamic state access upper bound
     * to a valid bound value, otherwise, the border color pointer may be rejected and you
     * may get incorrect border color. This is a known hardware bug. */
  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
__gen_gpgpu_set_base_address_gen9(gen_gpgpu *gpgpu)
{
  const uint32_t def_cc = gen_gpgpu_get_cache_ctrl(gpgpu); /* default Cache Control value */
  BEGIN_BATCH(gpgpu->batch, 19);
  OUT_BATCH(gpgpu->batch, CMD_STATE_BASE_ADDRESS | 17);
  /* 0, Gen State Mem Obj CC, Stateless Mem Obj CC, Stateless Access Write Back */
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | (0 << 1) | BASE_ADDRESS_MODIFY); /* General State Base Addr   */
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 16));
  /* 0, State Mem Obj CC */
  /* We use a state base address for the surface heap since IVB clamp the
     * binding table pointer at 11 bits. So, we cannot use pointers directly while
     * using the surface heap
     */
  assert(gpgpu->aux.surface_heap_offset % 4096 == 0);
  OUT_RELOC(gpgpu->batch, gpgpu->aux.aux_bo,
            I915_GEM_DOMAIN_SAMPLER,
            I915_GEM_DOMAIN_SAMPLER,
            gpgpu->aux.surface_heap_offset + (0 | (def_cc << 4) | (0 << 1) | BASE_ADDRESS_MODIFY));
  OUT_BATCH(gpgpu->batch, 0);
  OUT_RELOC(gpgpu->batch, gpgpu->aux.aux_bo,
            I915_GEM_DOMAIN_RENDER,
            I915_GEM_DOMAIN_RENDER,
            (0 | (def_cc << 4) | (0 << 1) | BASE_ADDRESS_MODIFY)); /* Dynamic State Base Addr */
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | BASE_ADDRESS_MODIFY); /* Indirect Obj Base Addr */
  OUT_BATCH(gpgpu->batch, 0);
  //OUT_BATCH(gpgpu->batch, 0 | (def_cc << 4) | BASE_ADDRESS_MODIFY); /* Instruction Base Addr  */
  OUT_RELOC(gpgpu->batch, gpgpu->kernel_bo,
            I915_GEM_DOMAIN_INSTRUCTION,
            I915_GEM_DOMAIN_INSTRUCTION,
            0 + (0 | (def_cc << 4) | (0 << 1) | BASE_ADDRESS_MODIFY));
  OUT_BATCH(gpgpu->batch, 0);

  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  /* According to mesa i965 driver code, we must set the dynamic state access upper bound
     * to a valid bound value, otherwise, the border color pointer may be rejected and you
     * may get incorrect border color. This is a known hardware bug. */
  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0xfffff000 | BASE_ADDRESS_MODIFY);
  /* Bindless surface state base address */
  OUT_BATCH(gpgpu->batch, (def_cc << 4) | BASE_ADDRESS_MODIFY);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0xfffff000);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
gen_gpgpu_set_base_address(gen_gpgpu *gpgpu)
{
  if (IS_BROADWELL(gpgpu->device->device_id) || IS_CHERRYVIEW(gpgpu->device->device_id))
    return __gen_gpgpu_set_base_address_gen8(gpgpu);

  if (IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_set_base_address_gen9(gpgpu);

  return __gen_gpgpu_set_base_address_gen7(gpgpu);
}

static uint32_t
__gen_gpgpu_get_scratch_index_gen7(uint32_t size)
{
  return size / 1024 - 1;
}

static uint32_t
__gen_gpgpu_get_scratch_index_gen75(uint32_t size)
{
  //align in backend, if non pow2, must align when alloc scratch bo.
  assert((size & (size - 1)) == 0);
  size = size >> 11;
  uint32_t index = 0;
  while ((size >>= 1) > 0)
    index++; //get leading one

  return index;
}

static uint32_t
__gen_gpgpu_get_scratch_index_gen8(uint32_t size)
{
  //align in backend, if non pow2, must align when alloc scratch bo.
  assert((size & (size - 1)) == 0);
  size = size >> 10;
  uint32_t index = 0;
  while ((size >>= 1) > 0)
    index++; //get leading one

  return index;
}

static uint32_t
gen_gpgpu_get_scratch_index(gen_gpgpu *gpgpu, uint32_t size)
{
  if (IS_BROADWELL(gpgpu->device->device_id) ||
      IS_CHERRYVIEW(gpgpu->device->device_id) || IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_get_scratch_index_gen8(size);

  if (IS_HASWELL(gpgpu->device->device_id))
    return __gen_gpgpu_get_scratch_index_gen75(size);

  if (IS_IVYBRIDGE(gpgpu->device->device_id))
    return __gen_gpgpu_get_scratch_index_gen7(size);

  assert(0);
  return -1;
}

static cl_int
__gen_gpgpu_get_max_curbe_size(uint32_t device_id)
{
  if (IS_BAYTRAIL_T(device_id) ||
      IS_IVB_GT1(device_id))
    return 992;
  else
    return 2016;
}

static cl_int
__gen_gpgpu_get_curbe_size(gen_gpgpu *gpgpu)
{
  int curbe_size = gpgpu->thread.size_cs_entry * gpgpu->thread.num_cs_entries;
  int max_curbe_size = __gen_gpgpu_get_max_curbe_size(gpgpu->device->device_id);

  if (curbe_size > max_curbe_size) {
    CL_LOG_WARNING("warning, curbe size exceed limitation.");
    return max_curbe_size;
  } else
    return curbe_size;
}

static void
__gen_gpgpu_load_vfe_state_gen7(gen_gpgpu *gpgpu)
{
  int32_t scratch_index;
  BEGIN_BATCH(gpgpu->batch, 8);
  OUT_BATCH(gpgpu->batch, CMD_MEDIA_STATE_POINTERS | (8 - 2));

  if (gpgpu->thread.per_thread_scratch > 0) {
    scratch_index = gen_gpgpu_get_scratch_index(gpgpu, gpgpu->thread.per_thread_scratch);
    OUT_RELOC(gpgpu->batch, gpgpu->mem.scratch_bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              scratch_index);
  } else {
    OUT_BATCH(gpgpu->batch, 0);
  }
  /* max_thread | urb entries | (reset_gateway|bypass_gate_way | gpgpu_mode) */
  OUT_BATCH(gpgpu->batch, 0 | ((gpgpu->thread.max_thread_num - 1) << 16) | (0 << 8) | 0xc4);
  OUT_BATCH(gpgpu->batch, 0);
  /* curbe_size */
  OUT_BATCH(gpgpu->batch, __gen_gpgpu_get_curbe_size(gpgpu));
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
__gen_gpgpu_load_vfe_state_gen8(gen_gpgpu *gpgpu)
{
  int32_t scratch_index;
  BEGIN_BATCH(gpgpu->batch, 9);
  OUT_BATCH(gpgpu->batch, CMD_MEDIA_STATE_POINTERS | (9 - 2));

  if (gpgpu->thread.per_thread_scratch > 0) {
    scratch_index = gen_gpgpu_get_scratch_index(gpgpu, gpgpu->thread.per_thread_scratch);
    OUT_RELOC(gpgpu->batch, gpgpu->mem.scratch_bo,
              I915_GEM_DOMAIN_RENDER,
              I915_GEM_DOMAIN_RENDER,
              scratch_index);
  } else {
    OUT_BATCH(gpgpu->batch, 0);
  }
  OUT_BATCH(gpgpu->batch, 0);

  /* max_thread | urb entries | (reset_gateway|bypass_gate_way | gpgpu_mode) */
  OUT_BATCH(gpgpu->batch, 0 | ((gpgpu->thread.max_thread_num - 1) << 16) | (2 << 8) | 0xc0); //urb entries can't be 0
  OUT_BATCH(gpgpu->batch, 0);
  /* urb entries size | curbe_size */
  OUT_BATCH(gpgpu->batch, 2 << 16 | __gen_gpgpu_get_curbe_size(gpgpu));
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);

  ADVANCE_BATCH(gpgpu->batch);
}

static void
gen_gpgpu_load_vfe_state(gen_gpgpu *gpgpu)
{
  if (IS_BROADWELL(gpgpu->device->device_id) ||
      IS_CHERRYVIEW(gpgpu->device->device_id) || IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_load_vfe_state_gen8(gpgpu);

  return __gen_gpgpu_load_vfe_state_gen7(gpgpu);
}

static void
__gen_gpgpu_load_curbe_buffer_gen7(gen_gpgpu *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 4);
  OUT_BATCH(gpgpu->batch, CMD(2, 0, 1) | (4 - 2)); /* length-2 */
  OUT_BATCH(gpgpu->batch, 0);                      /* mbz */
  OUT_BATCH(gpgpu->batch, __gen_gpgpu_get_curbe_size(gpgpu) * 32);
  OUT_RELOC(gpgpu->batch, gpgpu->aux.aux_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, gpgpu->aux.curbe_offset);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
__gen_gpgpu_load_curbe_buffer_gen8(gen_gpgpu *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 4);
  OUT_BATCH(gpgpu->batch, CMD(2, 0, 1) | (4 - 2)); /* length-2 */
  OUT_BATCH(gpgpu->batch, 0);                      /* mbz */
  OUT_BATCH(gpgpu->batch, __gen_gpgpu_get_curbe_size(gpgpu) * 32);
  OUT_BATCH(gpgpu->batch, gpgpu->aux.curbe_offset);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
gen_gpgpu_load_curbe_buffer(gen_gpgpu *gpgpu)
{
  if (IS_BROADWELL(gpgpu->device->device_id) ||
      IS_CHERRYVIEW(gpgpu->device->device_id) || IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_load_curbe_buffer_gen8(gpgpu);

  return __gen_gpgpu_load_curbe_buffer_gen7(gpgpu);
}

static void
__gen_gpgpu_load_idrt_gen7(gen_gpgpu *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 4);
  OUT_BATCH(gpgpu->batch, CMD(2, 0, 2) | (4 - 2)); /* length-2 */
  OUT_BATCH(gpgpu->batch, 0);                      /* mbz */
  OUT_BATCH(gpgpu->batch, 1 << 5);
  OUT_RELOC(gpgpu->batch, gpgpu->aux.aux_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, gpgpu->aux.idrt_offset);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
__gen_gpgpu_load_idrt_gen8(gen_gpgpu *gpgpu)
{
  BEGIN_BATCH(gpgpu->batch, 4);
  OUT_BATCH(gpgpu->batch, CMD(2, 0, 2) | (4 - 2)); /* length-2 */
  OUT_BATCH(gpgpu->batch, 0);                      /* mbz */
  OUT_BATCH(gpgpu->batch, 1 << 5);
  OUT_BATCH(gpgpu->batch, gpgpu->aux.idrt_offset);
  ADVANCE_BATCH(gpgpu->batch);
}

static void
gen_gpgpu_load_idrt(gen_gpgpu *gpgpu)
{
  if (IS_BROADWELL(gpgpu->device->device_id) ||
      IS_CHERRYVIEW(gpgpu->device->device_id) || IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_load_idrt_gen8(gpgpu);

  return __gen_gpgpu_load_idrt_gen7(gpgpu);
}

/* Emit PIPE_CONTROLs to write the current GPU timestamp into a buffer. */
static void
gen_gpgpu_write_timestamp(gen_gpgpu *gpgpu, int idx)
{
  BEGIN_BATCH(gpgpu->batch, 5);
  OUT_BATCH(gpgpu->batch, CMD_PIPE_CONTROL | (5 - 2));
  OUT_BATCH(gpgpu->batch, GEN7_PIPE_CONTROL_WRITE_TIMESTAMP);
  OUT_RELOC(gpgpu->batch, gpgpu->mem.time_stamp_bo,
            I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
            GEN7_PIPE_CONTROL_GLOBAL_GTT_WRITE | idx * sizeof(uint64_t));
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, 0);
  ADVANCE_BATCH();
}

static void
gen_gpgpu_batch_start(gen_gpgpu *gpgpu)
{
  intel_batchbuffer_start_atomic(gpgpu->batch, 256);
  gen_gpgpu_pipe_control(gpgpu);
  gen_gpgpu_set_L3(gpgpu, gpgpu->thread.barrier_slm_used);
  gen_gpgpu_select_pipeline(gpgpu);
  gen_gpgpu_set_base_address(gpgpu);
  gen_gpgpu_load_vfe_state(gpgpu);
  gen_gpgpu_load_curbe_buffer(gpgpu);
  gen_gpgpu_load_idrt(gpgpu);

  /* Insert PIPE_CONTROL for time stamp of start*/
  if (gpgpu->mem.time_stamp_bo)
    gen_gpgpu_write_timestamp(gpgpu, 0);
}

static void
__gen_gpgpu_walker_gen7(gen_gpgpu *gpgpu, uint32_t simd_sz, uint32_t thread_n,
                        const size_t global_wk_off[3], const size_t global_dim_off[3],
                        const size_t global_wk_sz[3], const size_t local_wk_sz[3])
{
  const uint32_t global_wk_dim[3] = {
    global_wk_sz[0] / local_wk_sz[0],
    global_wk_sz[1] / local_wk_sz[1],
    global_wk_sz[2] / local_wk_sz[2]};
  uint32_t right_mask = ~0x0;
  size_t group_sz = local_wk_sz[0] * local_wk_sz[1] * local_wk_sz[2];

  assert(simd_sz == 8 || simd_sz == 16);

  uint32_t shift = (group_sz & (simd_sz - 1));
  shift = (shift == 0) ? simd_sz : shift;
  right_mask = (1 << shift) - 1;

  BEGIN_BATCH(gpgpu->batch, 11);
  OUT_BATCH(gpgpu->batch, CMD_GPGPU_WALKER | 9);
  OUT_BATCH(gpgpu->batch, 0); /* kernel index == 0 */
  assert(thread_n <= 64);
  if (simd_sz == 16)
    OUT_BATCH(gpgpu->batch, (1 << 30) | (thread_n - 1)); /* SIMD16 | thread max */
  else
    OUT_BATCH(gpgpu->batch, (0 << 30) | (thread_n - 1)); /* SIMD8  | thread max */
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[0]);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[1]);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[2]);
  OUT_BATCH(gpgpu->batch, right_mask);
  OUT_BATCH(gpgpu->batch, ~0x0); /* we always set height as 1, so set bottom mask as all 1*/
  ADVANCE_BATCH(gpgpu->batch);

  BEGIN_BATCH(gpgpu->batch, 2);
  OUT_BATCH(gpgpu->batch, CMD_MEDIA_STATE_FLUSH | 0);
  OUT_BATCH(gpgpu->batch, 0); /* kernel index == 0 */
  ADVANCE_BATCH(gpgpu->batch);

  if (IS_IVYBRIDGE(gpgpu->device->device_id))
    gen_gpgpu_pipe_control(gpgpu);
}

static void
__gen_gpgpu_walker_gen8(gen_gpgpu *gpgpu, uint32_t simd_sz, uint32_t thread_n,
                        const size_t global_wk_off[3], const size_t global_dim_off[3],
                        const size_t global_wk_sz[3], const size_t local_wk_sz[3])
{
  const uint32_t global_wk_dim[3] = {
    global_wk_sz[0] / local_wk_sz[0],
    global_wk_sz[1] / local_wk_sz[1],
    global_wk_sz[2] / local_wk_sz[2]};
  uint32_t right_mask = ~0x0;
  size_t group_sz = local_wk_sz[0] * local_wk_sz[1] * local_wk_sz[2];

  assert(simd_sz == 8 || simd_sz == 16);

  uint32_t shift = (group_sz & (simd_sz - 1));
  shift = (shift == 0) ? simd_sz : shift;
  right_mask = (1 << shift) - 1;

  BEGIN_BATCH(gpgpu->batch, 15);
  OUT_BATCH(gpgpu->batch, CMD_GPGPU_WALKER | 13);
  OUT_BATCH(gpgpu->batch, 0); /* kernel index == 0 */
  OUT_BATCH(gpgpu->batch, 0); /* Indirect Data Length */
  OUT_BATCH(gpgpu->batch, 0); /* Indirect Data Start Address */
  assert(thread_n <= 64);
  if (simd_sz == 16)
    OUT_BATCH(gpgpu->batch, (1 << 30) | (thread_n - 1)); /* SIMD16 | thread max */
  else
    OUT_BATCH(gpgpu->batch, (0 << 30) | (thread_n - 1)); /* SIMD8  | thread max */
  OUT_BATCH(gpgpu->batch, global_dim_off[0]);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[0] + global_dim_off[0]);
  OUT_BATCH(gpgpu->batch, global_dim_off[1]);
  OUT_BATCH(gpgpu->batch, 0);
  OUT_BATCH(gpgpu->batch, global_wk_dim[1] + global_dim_off[1]);
  OUT_BATCH(gpgpu->batch, global_dim_off[2]);
  OUT_BATCH(gpgpu->batch, global_wk_dim[2] + global_dim_off[2]);
  OUT_BATCH(gpgpu->batch, right_mask);
  OUT_BATCH(gpgpu->batch, ~0x0); /* we always set height as 1, so set bottom mask as all 1*/
  ADVANCE_BATCH(gpgpu->batch);

  BEGIN_BATCH(gpgpu->batch, 2);
  OUT_BATCH(gpgpu->batch, CMD_MEDIA_STATE_FLUSH | 0);
  OUT_BATCH(gpgpu->batch, 0); /* kernel index == 0 */
  ADVANCE_BATCH(gpgpu->batch);

  gen_gpgpu_pipe_control(gpgpu);
}

static void
gen_gpgpu_walker(gen_gpgpu *gpgpu, uint32_t simd_sz, uint32_t thread_n,
                 const size_t global_wk_off[3], const size_t global_dim_off[3],
                 const size_t global_wk_sz[3], const size_t local_wk_sz[3])
{
  if (IS_BROADWELL(gpgpu->device->device_id) ||
      IS_CHERRYVIEW(gpgpu->device->device_id) || IS_GEN9(gpgpu->device->device_id))
    return __gen_gpgpu_walker_gen8(gpgpu, simd_sz, thread_n, global_wk_off,
                                   global_dim_off, global_wk_sz, local_wk_sz);

  return __gen_gpgpu_walker_gen7(gpgpu, simd_sz, thread_n, global_wk_off,
                                 global_dim_off, global_wk_sz, local_wk_sz);
}

static void
__gen_gpgpu_post_action_gen7(gen_gpgpu *gpgpu, int32_t flush_mode)
{
  if (flush_mode)
    gen_gpgpu_pipe_control(gpgpu);
}

static void
__gen_gpgpu_post_action_gen75(gen_gpgpu *gpgpu, int32_t flush_mode)
{
  /* flush force for set L3 */
  gen_gpgpu_pipe_control(gpgpu);

  /* Restore L3 control to disable SLM mode,
     otherwise, may affect 3D pipeline */
  gen_gpgpu_set_L3(gpgpu, 0);
}

static void
gen_gpgpu_post_action(gen_gpgpu *gpgpu, int32_t flush_mode)
{
  if (IS_HASWELL(gpgpu->device->device_id))
    return __gen_gpgpu_post_action_gen75(gpgpu, flush_mode);

  return __gen_gpgpu_post_action_gen7(gpgpu, flush_mode);
}

static void
gen_gpgpu_batch_end(gen_gpgpu *gpgpu, int32_t flush_mode)
{
  /* Insert PIPE_CONTROL for time stamp of end*/
  if (gpgpu->mem.time_stamp_bo)
    gen_gpgpu_write_timestamp(gpgpu, 1);

  gen_gpgpu_post_action(gpgpu, flush_mode);
  intel_batchbuffer_end_atomic(gpgpu->batch);
}

/* Get the GPU execute time. */
LOCAL void
gen_gpgpu_event_get_exec_timestamp(void *gpgpu_ctx, int index, uint64_t *ret_ts)
{
  gen_gpgpu_exec_ctx *gpu_exec_ctx = gpgpu_ctx;
  uint64_t result = 0;
  /* We use last one's time */
  gen_gpgpu *gpgpu = gpu_exec_ctx->all_gpu[gpu_exec_ctx->gpu_num - 1];
  assert(gpgpu);

  assert(gpgpu->mem.time_stamp_bo);
  assert(index == 0 || index == 1);
  drm_intel_gem_bo_map_gtt(gpgpu->mem.time_stamp_bo);
  uint64_t *ptr = gpgpu->mem.time_stamp_bo->virtual;
  result = ptr[index];

  /* According to BSpec, the timestamp counter should be 36 bits,
     but comparing to the timestamp counter from IO control reading,
     we find the first 4 bits seems to be fake. In order to keep the
     timestamp counter conformable, we just skip the first 4 bits.
  */
  result = (result & 0x0FFFFFFFF) * 80; //convert to nanoseconds
  *ret_ts = result;

  drm_intel_gem_bo_unmap_gtt(gpgpu->mem.time_stamp_bo);
}

#define TIMESTAMP_ADDR 0x2358

/* IVB and HSW's result MUST shift in x86_64 system */
static uint64_t
__gen_gpgpu_read_ts_reg_gen7(drm_intel_bufmgr *bufmgr)
{
  uint64_t result = 0;
  drm_intel_reg_read(bufmgr, TIMESTAMP_ADDR, &result);
  /* In x86_64 system, the low 32bits of timestamp count are stored in the high 32 bits of
     result which got from drm_intel_reg_read, and 32-35 bits are lost; but match bspec in
     i386 system. It seems the kernel readq bug. So shift 32 bit in x86_64, and only remain
     32 bits data in i386.
  */
  struct utsname buf;
  uname(&buf);
  /* In some systems, the user space is 32 bit, but kernel is 64 bit, so can't use the
   * compiler's flag to determine the kernel'a architecture, use uname to get it. */
  /* x86_64 in linux, amd64 in bsd */
  if (strcmp(buf.machine, "x86_64") == 0 || strcmp(buf.machine, "amd64") == 0)
    return result >> 32;
  else
    return result & 0x0ffffffff;
}

/* baytrail's result should clear high 4 bits */
static uint64_t
__gen_gpgpu_read_ts_reg_baytrail(drm_intel_bufmgr *bufmgr)
{
  uint64_t result = 0;
  drm_intel_reg_read(bufmgr, TIMESTAMP_ADDR, &result);
  return result & 0x0ffffffff;
}

static uint64_t
gen_gpgpu_read_ts_reg(cl_device_id device, intel_driver_t *drv)
{
  if (IS_CHERRYVIEW(device->device_id) || IS_BAYTRAIL_T(device->device_id))
    return __gen_gpgpu_read_ts_reg_baytrail(drv->bufmgr);

  return __gen_gpgpu_read_ts_reg_gen7(drv->bufmgr);
}

LOCAL void
gen_gpgpu_event_get_gpu_cur_timestamp(cl_device_id device, intel_driver_t *drv, uint64_t *ret_ts)
{
  uint64_t result = 0;

  /* Get the ts that match the bspec */
  result = gen_gpgpu_read_ts_reg(device, drv);
  result *= 80;

  *ret_ts = result;
  return;
}
