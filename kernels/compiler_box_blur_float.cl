__kernel void compiler_box_blur_float(__global const float4 *src,
                                      __global float4 *dst,
                                      int w,
                                      int h,
                                      int chunk)
{
  const int x = get_global_id(0);
  int y = get_global_id(1)*chunk;
  const int yend = min(y+chunk, h); /* we process a tile in the image */

  /* Current line (left (1 pixel), center (4 pixels), right (1 pixel)) */
  const int left = max(x-1,0) + y*w;
  const int right = min(x+1,w-1) + y*w;
  int curr = x + y*w;
  float4 currPixel = src[left] + src[curr] + src[right];

  /* Top line (left (1 pixel), center (4 pixels), right (1 pixel)) */
  const int ytop = max(y-1,0);
  const int topLeft = max(x-1,0) + ytop*w;
  const int topRight = min(x+1,w-1) + ytop*w;
  const int top = x + ytop*w;
  float4 topPixel = src[topLeft] + src[top] + src[topRight];

  /* To guard bottom line */
  const int maxBottom = x + (h-1)*w;
  const int maxBottomLeft = max(x-1,0) + (h-1)*w;
  const int maxBottomRight = min(x+1,w-1) + (h-1)*w;

  /* We use a short 4 pixel sliding window */
  const int ybottom = min(y+1,h-1);
  int bottomLeft = max(x-1 + ybottom*w, ybottom*w);
  int bottomRight = min(x+1 + ybottom*w, ybottom*w+w-1);
  int bottom = x + ybottom*w;


  /* Top down sliding window */
  for (; y < yend; ++y, curr += w, bottom += w, bottomLeft += w, bottomRight += w) {
    const int center = min(bottom, maxBottom);
    const int left = min(bottomLeft, maxBottomLeft);
    const int right = min(bottomRight, maxBottomRight);
    const float4 bottomPixel = src[left] + src[center] + src[right];
    const float4 to = (bottomPixel + currPixel + topPixel) * (1.f/9.f);
    dst[curr] = to;
    topPixel = currPixel;
    currPixel = bottomPixel;
  }
}

