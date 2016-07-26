//#define BENCHMARK_NATIVE 1
//#define BENCHMARK_INTERNAL_FAST 2

/* benchmark pow performance */
kernel void bench_math_pow(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_powr(result, pwr); /* calls native */
#else
    result = pow(result, pwr); /* calls internal slow */
#endif
  }
  dst[get_global_id(0)] = result;
}

/* benchmark exp2 performance, exp2 is native */
kernel void bench_math_exp2(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
    result = exp2(result) * 0.1f;

  dst[get_global_id(0)] = result;
}

/* benchmark exp performance */
/* calls internal fast (native) if (x > -0x1.6p1 && x < 0x1.6p1) */
kernel void bench_math_exp(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_exp((float)-0x1.6p1 - result * 0.1f); /* calls native */
#elif defined(BENCHMARK_INTERNAL_FAST)
    result = exp((float)-0x1.6p1 + result * 0.1f); /* calls internal fast */
#else
    result = exp((float)-0x1.6p1 - result * 0.1f); /* calls internal slow */
#endif
  }

  dst[get_global_id(0)] = result;
}

/* benchmark exp10 performance */
/* calls internal fast (native) if (x < -0x1.4p+5) || (x > +0x1.4p+5)  */
kernel void bench_math_exp10(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_exp10((float)0x1.4p+5 + result * 0.1f); /* calls native */
#elif defined(BENCHMARK_INTERNAL_FAST)
    result = exp10((float)-0x1.4p+5 - result * 0.1f); /* calls internal fast */
#else
    result = exp10((float)-0x1.2p+5 - result * 0.1f); /* calls internal slow */
#endif
  }

  dst[get_global_id(0)] = result;
}

/* benchmark log2 performance */
/* calls internal fast (native) if (x > 0x1.1p0)  */
kernel void bench_math_log2(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_log2((float)0x1.1p0 + result * 0.0001f); /* calls native */
#elif defined(BENCHMARK_INTERNAL_FAST)
    result = log2((float)0x1.1p0 + result * 0.0001f); /* calls internal fast */
#else
    result = log2((float)0x1.1p0 - result * 0.0001f); /* calls internal slow */
#endif
  }

  dst[get_global_id(0)] = result;
}

/* benchmark log performance */
/* calls internal fast (native) if (x > 0x1.1p0)  */
kernel void bench_math_log(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_log((float)0x1.1p0 + result * 0.0001f); /* calls native */
#elif defined(BENCHMARK_INTERNAL_FAST)
    result = log((float)0x1.1p0 + result * 0.0001f); /* calls internal fast */
#else
    result = log((float)0x1.1p0 - result * 0.0001f); /* calls internal slow */
#endif
  }

  dst[get_global_id(0)] = result;
}

/* benchmark log10 performance */
/* calls internal fast (native) if (x > 0x1.1p0)  */
kernel void bench_math_log10(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_log10((float)0x1.1p0 + result * 0.0001f); /* calls native */
#elif defined(BENCHMARK_INTERNAL_FAST)
    result = log10((float)0x1.1p0 + result * 0.0001f); /* calls internal fast */
#else
    result = log10((float)0x1.1p0 - result * 0.0001f); /* calls internal slow */
#endif
  }

  dst[get_global_id(0)] = result;
}

/* benchmark sqrt performance */
kernel void bench_math_sqrt(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
    result = sqrt(result) + sqrt(pwr + result);

  dst[get_global_id(0)] = result;
}

/* benchmark sin performance */
kernel void bench_math_sin(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_sin(result); /* calls native */
#else
    result = sin(result);	/* calls internal, random complexity */
    //result = sin(0.1f + result); /* calls internal, (1) no reduction */
    //result = sin(2.f + result); /* calls internal, (2) fast reduction */
    //result = sin(4001 + result); /* calls internal, (3) slow reduction */
    result *= 0x1p-16;
#endif
  }

  dst[get_global_id(0)] = result;
}

/* benchmark cos performance */
kernel void bench_math_cos(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_cos(result); /* calls native */
#else
    result = cos(result);	/* calls internal, random complexity */
    //result = cos(0.1f + result); /* calls internal, (1) no reduction */
    //result = cos(2.f + result); /* calls internal, (2) fast reduction */
    //result = cos(4001.f + result); /* calls internal, (3) slow reduction */
    result *= 0x1p-16;
#endif
  }
  dst[get_global_id(0)] = result;
}

/* benchmark native tan performance */
kernel void bench_math_tan(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
  {
#if defined(BENCHMARK_NATIVE)
    result = native_tan(result); /* calls native */
#else
    result = tan(result); /* calls internal slow */
#endif
  }

  dst[get_global_id(0)] = result;
}

/* benchmark asin performance */
kernel void bench_math_asin(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
    result = asin(pwr - 1);

  dst[get_global_id(0)] = result;
}

/* benchmark acos performance */
kernel void bench_math_acos(
  global float *src,
  global float *dst,
  float pwr,
  uint loop)
{
  float result = src[get_global_id(0)];

  for(; loop > 0; loop--)
    result = acos(pwr - 1);

  dst[get_global_id(0)] = result;
}
