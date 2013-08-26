/*****************************************************************
 * The following functions are copied from i965 driver, commit
 * id 292368570a13501dfa95b1b0dd70966caf6ffc6b. Need to keep consistant
 * with the dri driver installed on current system.
 *****************************************************************/
static bool
_intel_region_flink(struct intel_region *region, uint32_t *name)
{
   if (region->name == 0) {
      if (drm_intel_bo_flink(region->bo, &region->name))
         return false;
   }

   *name = region->name;

   return true;
}

#define _DBG(...)
static void
_intel_region_release(struct intel_region **region_handle)
{
   struct intel_region *region = *region_handle;

   if (region == NULL) {
      _DBG("%s NULL\n", __FUNCTION__);
      return;
   }

   _DBG("%s %p %d\n", __FUNCTION__, region, region->refcount - 1);

   ASSERT(region->refcount > 0);
   region->refcount--;

   if (region->refcount == 0) {
      drm_intel_bo_unreference(region->bo);

      free(region);
   }
   *region_handle = NULL;
}

static void
_intel_region_reference(struct intel_region **dst, struct intel_region *src)
{
   _DBG("%s: %p(%d) -> %p(%d)\n", __FUNCTION__,
        *dst, *dst ? (*dst)->refcount : 0, src, src ? src->refcount : 0);

   if (src != *dst) {
      if (*dst)
         _intel_region_release(dst);

      if (src)
         src->refcount++;
      *dst = src;
   }
}

/**
 * This function computes masks that may be used to select the bits of the X
 * and Y coordinates that indicate the offset within a tile.  If the region is
 * untiled, the masks are set to 0.
 */
static void
_intel_region_get_tile_masks(struct intel_region *region,
                             uint32_t *mask_x, uint32_t *mask_y,
                             bool map_stencil_as_y_tiled)
{
   int cpp = region->cpp;
   uint32_t tiling = region->tiling;

   if (map_stencil_as_y_tiled)
      tiling = I915_TILING_Y;

   switch (tiling) {
   default:
      assert(false);
   case I915_TILING_NONE:
      *mask_x = *mask_y = 0;
      break;
   case I915_TILING_X:
      *mask_x = 512 / cpp - 1;
      *mask_y = 7;
      break;
   case I915_TILING_Y:
      *mask_x = 128 / cpp - 1;
      *mask_y = 31;
      break;
   }
}

/**
 * Compute the offset (in bytes) from the start of the region to the given x
 * and y coordinate.  For tiled regions, caller must ensure that x and y are
 * multiples of the tile size.
 */
static uint32_t
_intel_region_get_aligned_offset(struct intel_region *region, uint32_t x,
                                 uint32_t y, bool map_stencil_as_y_tiled)
{
   int cpp = region->cpp;
   uint32_t pitch = region->pitch;
   uint32_t tiling = region->tiling;

   if (map_stencil_as_y_tiled) {
      tiling = I915_TILING_Y;

      /* When mapping a W-tiled stencil buffer as Y-tiled, each 64-high W-tile
       * gets transformed into a 32-high Y-tile.  Accordingly, the pitch of
       * the resulting region is twice the pitch of the original region, since
       * each row in the Y-tiled view corresponds to two rows in the actual
       * W-tiled surface.  So we need to correct the pitch before computing
       * the offsets.
       */
      pitch *= 2;
   }

   switch (tiling) {
   default:
      assert(false);
   case I915_TILING_NONE:
      return y * pitch + x * cpp;
   case I915_TILING_X:
      assert((x % (512 / cpp)) == 0);
      assert((y % 8) == 0);
      return y * pitch + x / (512 / cpp) * 4096;
   case I915_TILING_Y:
      assert((x % (128 / cpp)) == 0);
      assert((y % 32) == 0);
      return y * pitch + x / (128 / cpp) * 4096;
   }
}

static void
_intel_miptree_get_image_offset(struct intel_mipmap_tree *mt,
                                GLuint level, GLuint slice,
                                GLuint *x, GLuint *y)
{
   assert(slice < mt->level[level].depth);

   *x = mt->level[level].slice[slice].x_offset;
   *y = mt->level[level].slice[slice].y_offset;
}
