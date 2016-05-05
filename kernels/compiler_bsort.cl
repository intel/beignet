#define UP 0
#define DOWN -1

/* Sort elements in a vector */
#define SORT_VECTOR(input, dir)                                   \
   comp = (input < shuffle(input, mask1)) ^ dir;                  \
   input = shuffle(input, as_uint4(comp + add1));                 \
   comp = (input < shuffle(input, mask2)) ^ dir;                  \
   input = shuffle(input, as_uint4(comp * 2 + add2));             \
   comp = (input < shuffle(input, mask3)) ^ dir;                  \
   input = shuffle(input, as_uint4(comp + add3));                 \

/* Sort elements between two vectors */
#define SWAP_VECTORS(input1, input2, dir)                         \
   temp = input1;                                                 \
   comp = ((input1 < input2) ^ dir) * 4 + add4;                   \
   input1 = shuffle2(input1, input2, as_uint4(comp));             \
   input2 = shuffle2(input2, temp, as_uint4(comp));               \

__kernel void compiler_bsort(__global float4 *data) {

   float4 input1, input2, temp;
   int4 comp;

   uint4 mask1 = (uint4)(1, 0, 3, 2);
   uint4 mask2 = (uint4)(2, 3, 0, 1);
   uint4 mask3 = (uint4)(3, 2, 1, 0);

   int4 add1 = (int4)(1, 1, 3, 3);
   int4 add2 = (int4)(2, 3, 2, 3);
   int4 add3 = (int4)(1, 2, 2, 3);
   int4 add4 = (int4)(4, 5, 6, 7);

   input1 = data[0];
   input2 = data[1];

   SORT_VECTOR(input1, UP)
   SORT_VECTOR(input2, DOWN)

   SWAP_VECTORS(input1, input2, UP)

   SORT_VECTOR(input1, UP)
   SORT_VECTOR(input2, UP)

   data[0] = input1;
   data[1] = input2;
}
