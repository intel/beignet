/* OpenCL 1.1 Supported Data Types */
__kernel void compiler_data_types()
{
  // built-in scalar data types (section 6.1.1)
  bool b;
  b = true;
  b = false;
  char c;
  unsigned char uc;
  uchar uc_2;
  short s;
  unsigned short us;
  ushort us_2;
  int i;
  unsigned int ui;
  uint ui_2;
  long l;
  unsigned long ul;
  ulong ul_2;
  float f;
  half h;
  size_t sz;
  ptrdiff_t pt;
  intptr_t it;
  uintptr_t uit;
  
  // built-in vector data types (section 6.1.2)
  // supported values of $n$ are 2, 3, 4, 8, 16 for all vector data types
#define VEC(sz) char##sz c##sz;   \
                uchar##sz uc##sz; \
                short##sz s##sz;  \
                ushort##sz us##sz;\
                int##sz i##sz;    \
                uint##sz ui##sz;  \
                long##sz l##sz;   \
                ulong##sz ul##sz; \
                float##sz f##sz;
#if 1
   VEC(2);
   VEC(3);
   VEC(4);
   VEC(8);
   VEC(16);
#endif
   float16 f_16 = (float16)(1.0f);
   f_16.s0 += 1;
   f_16.s1 += 1;
   f_16.s2 += 1;
   f_16.s3 += 1;
   f_16.s4 += 1;
   f_16.s5 += 1;
   f_16.s6 += 1;
   f_16.s7 += 1;
   f_16.s8 += 1;
   f_16.s9 += 1;
   f_16.sa += 1;
   f_16.sb += 1;
   f_16.sc += 1;
   f_16.sd += 1;
   f_16.se += 1;
   f_16.sf += 1;
   f_16.sA += 1;
   f_16.sB += 1;
   f_16.sC += 1;
   f_16.sD += 1;
   f_16.sE += 1;
   f_16.sF += 1;
   float8 f_8;
   f_8 = f_16.lo;
   f_8 = f_16.hi;
   f_8 = f_16.odd;
   f_8 = f_16.even;
   uint4 u_4 = (uint4)(1);

   // Other built-in data types (section 6.1.3)
   image2d_t i2dt;
   image3d_t i3dt;
   sampler_t st;
   event_t et;
}
