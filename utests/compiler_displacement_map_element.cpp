#include "utest_helper.hpp"

typedef unsigned int uint;
constexpr int W = 16, H = 16;
constexpr int SIZE = W * H;
uint in_1[SIZE];
uint disp_map[SIZE];
uint out_1[SIZE];

uint cpu(const int cx, const int cy, const uint *in, const uint *disp_map, int w, int h) {
  uint c = disp_map[cy * w + cx];
  int x_pos = cx + c;
  int y_pos = cy + c;
  if(0 <= x_pos && x_pos < w && 0 <= y_pos && y_pos < h)
    return in[y_pos * w + x_pos];
  else
    return 0;
}

void test() {
  OCL_MAP_BUFFER(2);
  for(int y=0; y<H; y++)
    for(int x=0; x<W; x++) {
      uint out = ((uint*)buf_data[2]) [y * W + x];
      uint wish = cpu(x, y, in_1, disp_map, W, H);
      if(out != wish)
        printf("XXX %d %d %x %x\n", x, y, out, wish);
      OCL_ASSERT(out == wish);
    }
  OCL_UNMAP_BUFFER(2);
}

void displacement_map_element(void) {
  int i, pass;

  OCL_CREATE_KERNEL("compiler_displacement_map_element");
  OCL_CREATE_BUFFER(buf[0], 0, SIZE * sizeof(uint), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, SIZE * sizeof(uint), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, SIZE * sizeof(uint), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(W), &W);
  OCL_SET_ARG(3, sizeof(H), &H);
  OCL_SET_ARG(4, sizeof(cl_mem), &buf[2]);
  globals[0] = W;
  globals[1] = H;
  locals[0] = 16;
  locals[1] = 16;

  for (pass = 0; pass < 8; pass ++) {
    OCL_MAP_BUFFER(0);
    OCL_MAP_BUFFER(1);
    for (i = 0; i < SIZE; i ++) {
      in_1[i] = ((uint*)buf_data[0])[i] = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
      disp_map[i] = ((uint*)buf_data[1])[i] = rand() & 3;
    }
    OCL_UNMAP_BUFFER(0);
    OCL_UNMAP_BUFFER(1);
    OCL_NDRANGE(2);
    test();
  }
}

MAKE_UTEST_FROM_FUNCTION(displacement_map_element);
