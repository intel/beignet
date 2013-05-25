inline float3 unpack_fp3(uint u) {
  float3 u3;
  u3.x = (float) (u & 0xff); u >>= 8;
  u3.y = (float) (u & 0xff); u >>= 8;
  u3.z = (float) (u & 0xff);
  return u3;
}

inline uint pack_fp3(float3 u3) {
  uint u;
  u = (((uint) u3.x)) | (((uint) u3.y) << 8) | (((uint) u3.z) << 16);
  return u;
}

#define HFILTER3(C0, C1, C2, C3, CURR, LEFT, RIGHT)\
  float3 C0, C1, C2, C3;\
  do {\
    const uint4 from = vload4(CURR, src);\
    const float3 from0 = unpack_fp3(from.x);\
    const float3 from1 = unpack_fp3(from.y);\
    const float3 from2 = unpack_fp3(from.z);\
    const float3 from3 = unpack_fp3(from.w);\
    const float3 l = unpack_fp3(src[LEFT]);\
    const float3 r = unpack_fp3(src[RIGHT]);\
    C0 = (l+from0+from1);\
    C1 = (from0+from1+from2);\
    C2 = (from1+from2+from3);\
    C3 = (from2+from3+r);\
  } while(0)

__kernel void compiler_box_blur(__global const uint *src,
                                __global uint *dst,
                                int w,
                                int h,
                                int chunk)
{
  const int x = get_global_id(0);
  int y = get_global_id(1)*chunk;
  const int yend = min(y + chunk, h); /* we process a tile in the image */

  /* Current line (left (1 pixel), center (4 pixels), right (1 pixel)) */
  const int left = max(4*x-1, 0) + y*w;
  const int right = min(4*x+4, w-1) + y*w;
  int curr = x + y*(w>>2);
  HFILTER3(curr0, curr1, curr2, curr3, curr, left, right);

  /* Top line (left (1 pixel), center (4 pixels), right (1 pixel)) */
  const int ytop = max(y-1,0);
  const int topLeft = max(4*x-1, 0) + ytop*w;
  const int topRight = min(4*x+4, w-1) + ytop*w;
  const int top = x + ytop*(w>>2);
  HFILTER3(top0, top1, top2, top3, top, topLeft, topRight);

  /* To guard bottom line */
  const int maxBottom = x + (h-1)*(w>>2);
  const int maxBottomLeft = max(4*x-1,0) + (h-1)*w;
  const int maxBottomRight = min(4*x+4,w-1) + (h-1)*w;

  /* We use a short 3 pixel sliding window */
  const int ybottom = min(y+1,h-1);
  int bottomLeft = max(4*x-1, 0) + ybottom*w;
  int bottomRight = min(4*x+4, w-1) + ybottom*w;
  int bottom = x + ybottom*(w>>2);

  /* Top down sliding window */
  for (; y < yend; ++y, curr += (w>>2), bottom += (w>>2), bottomLeft += w, bottomRight += w) {
    const int center = min(bottom, maxBottom);
    const int left = min(bottomLeft, maxBottomLeft);
    const int right = min(bottomRight, maxBottomRight);
    HFILTER3(bottom0, bottom1, bottom2, bottom3, center, left, right);
    const float3 to0 = (top0+curr0+bottom0)*(1.f/9.f);
    const float3 to1 = (top1+curr1+bottom1)*(1.f/9.f);
    const float3 to2 = (top2+curr2+bottom2)*(1.f/9.f);
    const float3 to3 = (top3+curr3+bottom3)*(1.f/9.f);
    const uint4 to = (uint4)(pack_fp3(to0),pack_fp3(to1),pack_fp3(to2),pack_fp3(to3));
    vstore4(to, curr, dst);
    top0 = curr0; top1 = curr1; top2 = curr2; top3 = curr3;
    curr0 = bottom0; curr1 = bottom1; curr2 = bottom2; curr3 = bottom3;
  }
}
