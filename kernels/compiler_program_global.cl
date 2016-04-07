struct config{
  int s0;
  global short *s1;
};

global int i = 5;
global int bb = 4;
global int *global p;

/* array */
global int ba[12];

/* short/long data type */
global short s;
global short s2;
global long l;

/* pointer in constant AS to global */
global int * constant px =&i;

/* constant pointer relocation */
constant int x = 2;
constant int y =1;
constant int *constant z[2] = {&x, &y};

/* structure with pointer field */
global struct config c[2] = {{1, &s}, {2, &s2} };


global int a = 1;
global int b = 2;
global int * constant gArr[2]= {&a, &b};

global int a_var[1] = {0};
global int *p_var = a_var;

__kernel void compiler_program_global0(const global int *src, int dynamic) {
  size_t gid = get_global_id(0);
  /* global read/write */
  p = &i;
  *p += 1;

  /* pointer in struct memory access */
  *c[gid&1].s1 += 2;

  s = 2;
  l = 3;

  /* constant AS pointer (points to global) memory access */
  *px += *z[dynamic];

  p = &bb;
  /* array */
  if (gid < 11)
    ba[gid] = src[gid];
}

__kernel void compiler_program_global1(global int *dst, int dynamic) {
  size_t gid = get_global_id(0);
//  static global sg;

  dst[11] = i;
  dst[12] = *p;
  dst[13] = s;
  dst[14] = l;
  if (p_var == a_var)
    dst[15] = *gArr[dynamic];

  if (gid < 11)
    dst[gid] = ba[gid];
}

__kernel void nouse(int dynamic) {
  c[0].s1 = &s2;
  p_var = a+dynamic;
}

