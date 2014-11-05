#!/usr/bin/python
from utest_generator import *
import os,sys

#base_input_values = [80, -80, 3.14, -3.14, -0.5, 0.5, 1, -1, 0.0,6,-6,1500.24,-1500.24]
#extend_input_values = [FLT_MAX_POSI,FLT_MIN_NEGA,FLT_MIN_POSI,FLT_MAX_NEGA,80, -80, 3.14, -3.14, -0.5, 0.5, 1, -1, 0.0,6,-6,1500.24,-1500.24]

#func:
#    gpufuncName 
#    cpuFuncName
#    fileName: 'builtin_'+name
#    inputtype: a 2-D list because there're more than one input data
#    outputtype: a list
#    values
#    ulp

# reduce pi*x limitation to [-pi,pi]
reduce1='''
static float reduce1( float x )
{
  SF fx, fy;
  fx.f = fy.f = x;
  int n;

  fy.spliter.exponent = fx.spliter.exponent - 1;
  n = (int)fy.f;

  fx.f = fx.f - 2.0 * n;

  // reduce to [-1.0, 1.0]
  fx.f = (fx.f < -1)?(fx.f + 2.0):((fx.f > 1)?(fx.f - 2.0):fx.f);

  return fx.f;
}
'''
# define fuction: cospi
cospi='''
static float cospi(float x){
  float r = x;
  if ( x > 1 || x < -1) r = reduce1(x);

  // reduce to [0.0, 1.0]
  if (r < 0)
    r = fabs(r);

  if (r >= 0 && r <= 0.25)
    return  cosf(r * M_PI);
  else if (r > 0.25 && r <= 0.5)
    return  sinf((0.5 - r) * M_PI);
  else if (r > 0.5 && r <= 0.75)
    return sinf(-(r-0.5) * M_PI);
  else if (r > 0.75 && r <= 1.0){
    return -cosf((1 -  r) * M_PI);}

  // Error return
  return 0xffffffff;
}
'''
# define function: sinpi
sinpi='''
static float sinpi(float x){
  float r = x;
  if ( x > 1 || x < -1) r = reduce1(x);

  // reduce to [-0.5, 0.5]
  if (r < -0.5)
    r = -1 - r;
  else if (r > 0.5)
    r = 1 - r;

  if (r > 0.25 && r <= 0.5)
    return  cosf((0.5 - r) * M_PI);
  else if (r >= 0 && r <= 0.25)
    return  sinf(r * M_PI);
  else if (r >= -0.25 && r < 0)
    return -sinf(r * -M_PI);
  else if (r >= -0.5 && r < -0.25){
    return -cosf((0.5 + r) * M_PI);}

  // Error return
  return 0xffffffff;
}
'''

base_input_values = [ 0, 1, 3.14, 5.15, 6.01, 7.89]
base_input_values1 = [ 1, 3.14, 5.15, 6.01, 7.89]
def main():
  ##### gentype acos(gentype)
  acos_input_values = base_input_values
  acos_input_type = ['float','float2','float4','float8','float16']
  acos_output_type = ['float','float2','float4','float8','float16']
  acosUtests = func('acos','acos',[acos_input_type],acos_output_type,[acos_input_values],'4 * FLT_ULP')
  
  ##### gentype acosh(gentype)
  acosh_input_values = base_input_values
  acosh_input_type = ['float','float2','float4','float8','float16']
  acosh_output_type = ['float','float2','float4','float8','float16']
  acoshUtests = func('acosh','acosh',[acosh_input_type],acosh_output_type,[acosh_input_values],'4 * FLT_ULP')
  
  ##### gentype acospi(gentype x)
  acospi_input_values = base_input_values
  acospi_input_type = ['float','float2','float4','float8','float16']
  acospi_output_type = ['float','float2','float4','float8','float16']
  acospi_cpu_func='''
static float acospi(float x){
  return acos(x)/M_PI;
} '''
  acospiUtests = func('acospi','acospi',[acospi_input_type],acospi_output_type,[acospi_input_values],'4 * FLT_ULP',acospi_cpu_func)
  
  ##### gentype asin(gentype)
  asin_input_values = base_input_values
  asin_input_type = ['float','float2','float4','float8','float16']
  asin_output_type = ['float','float2','float4','float8','float16']
  asinUtests = func('asin','asin',[asin_input_type],asin_output_type,[asin_input_values],'4 * FLT_ULP')
  
  ##### gentype asinh(gentype)
  asinh_input_values = base_input_values
  asinh_input_type = ['float','float2','float4','float8','float16']
  asinh_output_type = ['float','float2','float4','float8','float16']
  asinhUtests = func('asinh','asinh',[asinh_input_type],asinh_output_type,[asinh_input_values],'4 * FLT_ULP')
  
  ##### gentype asinpi(gentype x)
  asinpi_input_values = base_input_values
  asinpi_input_type = ['float','float2','float4','float8','float16']
  asinpi_output_type = ['float','float2','float4','float8','float16']
  asinpi_cpu_func='''
static float asinpi(float x){
  return asin(x)/M_PI;
} '''
  asinpiUtests = func('asinpi','asinpi',[asinpi_input_type],asinpi_output_type,[asinpi_input_values],'4 * FLT_ULP',asinpi_cpu_func)
  
  ##### gentype atan(gentype y_over_x)
  atan_input_values = base_input_values
  atan_input_type = ['float','float2','float4','float8','float16']
  atan_output_type = ['float','float2','float4','float8','float16']
  atanUtests = func('atan','atan',[atan_input_type],atan_output_type,[atan_input_values],'5 * FLT_ULP')
  
  ##### gentype atan2(gentype y, gentype x)
  atan2_base_values = base_input_values
  atan2_input_values1 = []
  atan2_input_values2 = []
  atan2_input_values1,atan2_input_values2=gene2ValuesLoop(atan2_input_values1,atan2_input_values2,atan2_base_values)
  atan2_input_type1 = ['float','float2','float4','float8','float16']
  atan2_input_type2 = ['float','float2','float4','float8','float16']
  atan2_output_type = ['float','float2','float4','float8','float16']
  atan2Utests = func('atan2','atan2',[atan2_input_type1,atan2_input_type2],atan2_output_type,[atan2_input_values1,atan2_input_values2],'6 * FLT_ULP')
  
  ##### gentype atanh(gentype)
  atanh_input_values = base_input_values
  atanh_input_type = ['float','float2','float4','float8','float16']
  atanh_output_type = ['float','float2','float4','float8','float16']
  atanhUtests = func('atanh','atanh',[atanh_input_type],atanh_output_type,[atanh_input_values],'5 * FLT_ULP')
  
  ##### gentype atanpi(gentype x)
  atanpi_input_values = base_input_values
  atanpi_input_type = ['float','float2','float4','float8','float16']
  atanpi_output_type = ['float','float2','float4','float8','float16']
  atanpi_cpu_func='''
static float atanpi(float x){
  return atan(x)/M_PI;
} '''
  atanpiUtests = func('atanpi','atanpi',[atanpi_input_type],atanpi_output_type,[atanpi_input_values],'4 * FLT_ULP',atanpi_cpu_func)
  
#  ##### gentype atan2pi(gentype y, gentype x)
#  atan2pi_base_values = base_input_values
#  atan2pi_input_values1 = []
#  atan2pi_input_values2 = []
#  atan2pi_input_values1,atan2pi_input_values2=gene2ValuesLoop(atan2pi_input_values1,atan2pi_input_values2,atan2pi_base_values)
#  atan2pi_input_type1 = ['float','float2','float4','float8','float16']
#  atan2pi_input_type2 = ['float','float2','float4','float8','float16']
#  atan2pi_output_type = ['float','float2','float4','float8','float16']
#  atan2pi_cpu_func='''
#static float atan2pi(float y, float x){
#  return atan2(y,x)/M_PI;
#} '''
#  atan2piUtests = func('atan2pi','atan2pi',[atan2pi_input_type1,atan2pi_input_type2],atan2pi_output_type,[atan2pi_input_values1,atan2pi_input_values2],'6 * FLT_ULP',atan2pi_cpu_func)
  
  ##### gentype cbrt(gentype)
  cbrt_input_values = base_input_values
  cbrt_input_type = ['float','float2','float4','float8','float16']
  cbrt_output_type = ['float','float2','float4','float8','float16']
  cbrtUtests = func('cbrt','cbrt',[cbrt_input_type],cbrt_output_type,[cbrt_input_values],'4 * FLT_ULP')
  
  ##### gentype ceil(gentype)
  ceil_input_values = base_input_values
  ceil_input_type = ['float','float2','float4','float8','float16']
  ceil_output_type = ['float','float2','float4','float8','float16']
  ceilUtests = func('ceil','ceil',[ceil_input_type],ceil_output_type,[ceil_input_values],'0 * FLT_ULP')
  
  ##### gentype copysign(gentype x, gentype y)
  copysign_base_values = base_input_values
  copysign_input_values1 = []
  copysign_input_values2 = []
  copysign_input_values1,copysign_input_values2=gene2ValuesLoop(copysign_input_values1,copysign_input_values2,copysign_base_values)
  copysign_input_type1 = ['float','float2','float4','float8','float16']
  copysign_input_type2 = ['float','float2','float4','float8','float16']
  copysign_output_type = ['float','float2','float4','float8','float16']
  copysignUtests = func('copysign','copysign',[copysign_input_type1,copysign_input_type2],copysign_output_type,[copysign_input_values1,copysign_input_values2],'0 * FLT_ULP')
  
  ##### gentype cos(gentype)
  cos_input_values = base_input_values
  cos_input_type = ['float','float2','float4','float8','float16']
  cos_output_type = ['float','float2','float4','float8','float16']
  cosUtests = func('cos','cos',[cos_input_type],cos_output_type,[cos_input_values],'4 * FLT_ULP')
  
  ##### gentype cosh(gentype)
  cosh_input_values = base_input_values
  cosh_input_type = ['float','float2','float4','float8','float16']
  cosh_output_type = ['float','float2','float4','float8','float16']
  coshUtests = func('cosh','cosh',[cosh_input_type],cosh_output_type,[cosh_input_values],'4 * FLT_ULP')
  
  ##### gentype cospi(gentype x)
  cospi_input_values = base_input_values
  cospi_input_type = ['float','float2','float4','float8','float16']
  cospi_output_type = ['float','float2','float4','float8','float16']
  cospi_cpu_func=reduce1+cospi
  cospiUtests = func('cospi','cospi',[cospi_input_type],cospi_output_type,[cospi_input_values],'2 * FLT_ULP',cospi_cpu_func)
  
  ##### gentype erf(gentype)
  erf_input_values = base_input_values
  erf_input_type = ['float','float2','float4','float8','float16']
  erf_output_type = ['float','float2','float4','float8','float16']
  erfUtests = func('erf','erf',[erf_input_type],erf_output_type,[erf_input_values],'16 * FLT_ULP')

  ##### gentype erfc(gentype)
  erfc_input_values = base_input_values
  erfc_input_type = ['float','float2','float4','float8','float16']
  erfc_output_type = ['float','float2','float4','float8','float16']
  erfcUtests = func('erfc','erfc',[erfc_input_type],erfc_output_type,[erfc_input_values],'16 * FLT_ULP')
  
  ##### gentype exp(gentype x)
  exp_input_values = base_input_values
  exp_input_type = ['float','float2','float4','float8','float16']
  exp_output_type = ['float','float2','float4','float8','float16']
  expUtests = func('exp','exp',[exp_input_type],exp_output_type,[exp_input_values],'4 * FLT_ULP')
  
  ##### gentype exp2(gentype)
  exp2_input_values = base_input_values
  exp2_input_type = ['float','float2','float4','float8','float16']
  exp2_output_type = ['float','float2','float4','float8','float16']
  exp2Utests = func('exp2','exp2',[exp2_input_type],exp2_output_type,[exp2_input_values],'4 * FLT_ULP')
  
  ##### gentype exp10(gentype)
  exp10_input_values = base_input_values
  exp10_input_type = ['float','float2','float4','float8','float16']
  exp10_output_type = ['float','float2','float4','float8','float16']
  exp10Utests = func('exp10','exp10',[exp10_input_type],exp10_output_type,[exp10_input_values],'4 * FLT_ULP')
  
  ##### gentype expm1(gentype x)
  expm1_input_values = base_input_values
  expm1_input_type = ['float','float2','float4','float8','float16']
  expm1_output_type = ['float','float2','float4','float8','float16']
  expm1Utests = func('expm1','expm1',[expm1_input_type],expm1_output_type,[expm1_input_values],'4 * FLT_ULP')
  
  ##### gentype fabs(gentype)
  fabs_input_values = base_input_values
  fabs_input_type = ['float','float2','float4','float8','float16']
  fabs_output_type = ['float','float2','float4','float8','float16']
  fabsUtests = func('fabs','fabs',[fabs_input_type],fabs_output_type,[fabs_input_values],'0 * FLT_ULP')
  
  ##### gentype fdim(gentype x, gentype y)
  fdim_base_values = base_input_values
  fdim_input_values1 = []
  fdim_input_values2 = []
  fdim_input_values1,fdim_input_values2=gene2ValuesLoop(fdim_input_values1,fdim_input_values2,fdim_base_values)
  fdim_input_type1 = ['float','float2','float4','float8','float16']
  fdim_input_type2 = ['float','float2','float4','float8','float16']
  fdim_output_type = ['float','float2','float4','float8','float16']
  fdimUtests = func('fdim','fdim',[fdim_input_type1,fdim_input_type2],fdim_output_type,[fdim_input_values1,fdim_input_values2],'0 * FLT_ULP')
  
  ##### gentype floor(gentype)
  floor_input_values = base_input_values
  floor_input_type = ['float','float2','float4','float8','float16']
  floor_output_type = ['float','float2','float4','float8','float16']
  floorUtests = func('floor','floor',[floor_input_type],floor_output_type,[floor_input_values],'0 * FLT_ULP')
  
  ##### gentype fmax(gentype x, gentype y)
  fmax_base_values = base_input_values
  fmax_input_values1 = []
  fmax_input_values2 = []
  fmax_input_values1,fmax_input_values2=gene2ValuesLoop(fmax_input_values1,fmax_input_values2,fmax_base_values)
  fmax_input_type1 = ['float','float2','float4','float8','float16']
  fmax_input_type2 = ['float','float2','float4','float8','float16']
  fmax_output_type = ['float','float2','float4','float8','float16']
  fmaxUtests = func('fmax','fmax',[fmax_input_type1,fmax_input_type2],fmax_output_type,[fmax_input_values1,fmax_input_values2],'0 * FLT_ULP')
  
  ##### gentypef fmax(gentypef x, float y)
#  fmax_gentypef_base_values = base_input_values
#  fmax_gentypef_input_values1 = []
#  fmax_gentypef_input_values2 = []
#  fmax_gentypef_input_values2,fmax_gentypef_input_values1=gene2ValuesLoop(fmax_gentypef_input_values1,fmax_gentypef_input_values2,fmax_gentypef_base_values)
#  fmax_gentypef_input_type1 = ['float','float2','float4','float8','float16']
#  fmax_gentypef_input_type2 = ['float','float','float','float','float']
#  fmax_gentypef_output_type = ['float','float2','float4','float8','float16']
#  ##### gentypef fmax(gentypef x, float y)
#  fmax_gentypefUtests = func('gentypef_fmax','gentypef_fmax',[fmax_gentypef_input_type1,fmax_gentypef_input_type2],fmax_gentypef_output_type,[fmax_gentypef_input_values1,fmax_gentypef_input_values2],'0 * FLT_ULP')
  
  ##### gentype fmin(gentype x, gentype y)
  fmin_base_values = base_input_values
  fmin_input_values1 = []
  fmin_input_values2 = []
  fmin_input_values1,fmin_input_values2=gene2ValuesLoop(fmin_input_values1,fmin_input_values2,fmin_base_values)
  fmin_input_type1 = ['float','float2','float4','float8','float16']
  fmin_input_type2 = ['float','float2','float4','float8','float16']
  fmin_output_type = ['float','float2','float4','float8','float16']
  fminUtests = func('fmin','fmin',[fmin_input_type1,fmin_input_type2],fmin_output_type,[fmin_input_values1,fmin_input_values2],'0 * FLT_ULP')
  
#  ##### gentypef fmin(gentypef x, float y)
#  fmin_gentypef_base_values = base_input_values
#  fmin_gentypef_input_values1 = []
#  fmin_gentypef_input_values2 = []
#  fmin_gentypef_input_values2,fmin_gentypef_input_values1=gene2ValuesLoop(fmin_gentypef_input_values1,fmin_gentypef_input_values2,fmin_gentypef_base_values)
#  fmin_gentypef_input_type1 = ['float','float2','float4','float8','float16']
#  fmin_gentypef_input_type2 = ['float','float','float','float','float']
#  fmin_gentypef_output_type = ['float','float2','float4','float8','float16']
#  ##### gentypef fmin(gentypef x, float y)
#  fmin_gentypefUtests = func('gentypef_fmin','gentypef_fmin',[fmin_gentypef_input_type1,fmin_gentypef_input_type2],fmin_gentypef_output_type,[fmin_gentypef_input_values1,fmin_gentypef_input_values2],'0 * FLT_ULP')
#  
  ##### gentype fmod(gentype x, gentype y)
  fmod_base_values = base_input_values
  fmod_input_values1 = []
  fmod_input_values2 = []
  fmod_input_values1,fmod_input_values2=gene2ValuesLoop(fmod_input_values1,fmod_input_values2,fmod_base_values)
  fmod_input_type1 = ['float','float2','float4','float8','float16']
  fmod_input_type2 = ['float','float2','float4','float8','float16']
  fmod_output_type = ['float','float2','float4','float8','float16']
  fmodUtests = func('fmod','fmod',[fmod_input_type1,fmod_input_type2],fmod_output_type,[fmod_input_values1,fmod_input_values2],'0 * FLT_ULP')
  
  ##### gentype hypot(gentype x, gentype y)
  hypot_base_values = base_input_values
  hypot_input_values1 = []
  hypot_input_values2 = []
  hypot_input_values1,hypot_input_values2=gene2ValuesLoop(hypot_input_values1,hypot_input_values2,hypot_base_values)
  hypot_input_type1 = ['float','float2','float4','float8','float16']
  hypot_input_type2 = ['float','float2','float4','float8','float16']
  hypot_output_type = ['float','float2','float4','float8','float16']
  hypotUtests = func('hypot','hypot',[hypot_input_type1,hypot_input_type2],hypot_output_type,[hypot_input_values1,hypot_input_values2],'4 * FLT_ULP')
  
  ##### intn ilogb(floartn x)
  ilogb_input_values = base_input_values
  ilogb_input_type = ['float','float2','float4','float8','float16']
  ilogb_output_type = ['int','int2','int4','int8','int16']
  ilogbUtests = func('ilogb','ilogb',[ilogb_input_type],ilogb_output_type,[ilogb_input_values],'0 * INT_ULP')

  ##### gentype lgamma(gentype x)
  lgamma_input_values = base_input_values
  lgamma_input_type = ['float','float2','float4','float8','float16']
  lgamma_output_type = ['float','float2','float4','float8','float16']
  lgammaUtests = func('lgamma','lgamma',[lgamma_input_type],lgamma_output_type,[lgamma_input_values],'4 * FLT_ULP')

  ##### gentype log(gentype)
  log_input_values = base_input_values
  log_input_type = ['float','float2','float4','float8','float16']
  log_output_type = ['float','float2','float4','float8','float16']
  logUtests = func('log','log',[log_input_type],log_output_type,[log_input_values],'4 * FLT_ULP')
  
  ##### gentype log2(gentype)
  log2_input_values = base_input_values
  log2_input_type = ['float','float2','float4','float8','float16']
  log2_output_type = ['float','float2','float4','float8','float16']
  log2Utests = func('log2','log2',[log2_input_type],log2_output_type,[log2_input_values],'4 * FLT_ULP')
  
  ##### gentype log10(gentype)
  log10_input_values = base_input_values
  log10_input_type = ['float','float2','float4','float8','float16']
  log10_output_type = ['float','float2','float4','float8','float16']
  log10Utests = func('log10','log10',[log10_input_type],log10_output_type,[log10_input_values],'4 * FLT_ULP')
  
  ##### gentype log1p(gentype x)
  log1p_input_values = base_input_values
  log1p_input_type = ['float','float2','float4','float8','float16']
  log1p_output_type = ['float','float2','float4','float8','float16']
  log1pUtests = func('log1p','log1p',[log1p_input_type],log1p_output_type,[log1p_input_values],'4 * FLT_ULP')
  
  ##### gentype logb(gentype x)
  logb_input_values = base_input_values
  logb_input_type = ['float','float2','float4','float8','float16']
  logb_output_type = ['float','float2','float4','float8','float16']
  logbUtests = func('logb','logb',[logb_input_type],logb_output_type,[logb_input_values],'0 * FLT_ULP')
  
  ##### gentype maxmag(gentype x, gentype y)
  maxmag_base_values = base_input_values
  maxmag_input_values1 = []
  maxmag_input_values2 = []
  maxmag_input_values1,maxmag_input_values2=gene2ValuesLoop(maxmag_input_values1,maxmag_input_values2,maxmag_base_values)
  maxmag_input_type1 = ['float','float2','float4','float8','float16']
  maxmag_input_type2 = ['float','float2','float4','float8','float16']
  maxmag_output_type = ['float','float2','float4','float8','float16']
  maxmag_cpu_func='''
static float maxmag(float x, float y){
  if(fabs(x) > fabs(y))
    return x;
  else if (fabs(x) < fabs(y))
    return y;
  else
    return fmax(x,y);
} '''
  maxmagUtests = func('maxmag','maxmag',[maxmag_input_type1,maxmag_input_type2],maxmag_output_type,[maxmag_input_values1,maxmag_input_values2],'0 * FLT_ULP',maxmag_cpu_func)
  
  ##### gentype minmag(gentype x, gentype y)
  minmag_base_values = base_input_values
  minmag_input_values1 = []
  minmag_input_values2 = []
  minmag_input_values1,minmag_input_values2=gene2ValuesLoop(minmag_input_values1,minmag_input_values2,minmag_base_values)
  minmag_input_type1 = ['float','float2','float4','float8','float16']
  minmag_input_type2 = ['float','float2','float4','float8','float16']
  minmag_output_type = ['float','float2','float4','float8','float16']
  minmag_cpu_func='''
static float minmag(float x, float y){
  if(fabs(x) < fabs(y))
    return x;
  else if (fabs(x) > fabs(y))
    return y;
  else
    return fmin(x,y);
} '''
  minmagUtests = func('minmag','minmag',[minmag_input_type1,minmag_input_type2],minmag_output_type,[minmag_input_values1,minmag_input_values2],'0 * FLT_ULP',minmag_cpu_func)
  
#  ##### floatn nan(uintn nancode)
#  nan_input_values = base_input_values
#  nan_input_type = ['uint','uint2','uint4','uint8','uint16']
#  nan_output_type = ['float','float2','float4','float8','float16']
#  nanUtests = func('nan','nan',[nan_input_type],nan_output_type,[nan_input_values],'0 * FLT_ULP')
  
  ##### gentype nextafter(gentype x, gentype y)
  nextafter_base_values = base_input_values
  nextafter_input_values1 = []
  nextafter_input_values2 = []
  nextafter_input_values1,nextafter_input_values2=gene2ValuesLoop(nextafter_input_values1,nextafter_input_values2,nextafter_base_values)
  nextafter_input_type1 = ['float','float2','float4','float8','float16']
  nextafter_input_type2 = ['float','float2','float4','float8','float16']
  nextafter_output_type = ['float','float2','float4','float8','float16']
  nextafterUtests = func('nextafter','nextafterf',[nextafter_input_type1,nextafter_input_type2],nextafter_output_type,[nextafter_input_values1,nextafter_input_values2],'0 * FLT_ULP')
  
  ##### gentype pow(gentype x, gentype y)
  pow_base_values = base_input_values1
  pow_input_values1 = []
  pow_input_values2 = []
  pow_input_values1,pow_input_values2=gene2ValuesLoop(pow_input_values1,pow_input_values2,pow_base_values)
  pow_input_type1 = ['float','float2','float4','float8','float16']
  pow_input_type2 = ['float','float2','float4','float8','float16']
  pow_output_type = ['float','float2','float4','float8','float16']
  powUtests = func('pow','powf',[pow_input_type1,pow_input_type2],pow_output_type,[pow_input_values1,pow_input_values2],'16 * FLT_ULP')
  
  ##### floatn pown(floatn x, intn y)
  pown_input_values1 = [FLT_MAX_POSI,FLT_MIN_NEGA,FLT_MIN_POSI,FLT_MAX_NEGA,80, -80, 3.14, -3.14, 0.5, 1, 0.0,1500.24,-1500.24]
  pown_input_values2 = [-1,-2,-3,4,5,6,7,8,10,12,14,16,12]
  pown_input_type1 = ['float','float2','float4','float8','float16']
  pown_input_type2 = ['int','int2','int4','int8','int16']
  pown_output_type = ['float','float2','float4','float8','float16']
  pown_cpu_func='''
static float pown(float x, int y){
    return pow(x,y);
} '''
  pownUtests = func('pown','pown',[pown_input_type1,pown_input_type2],pown_output_type,[pown_input_values1,pown_input_values2],'16 * FLT_ULP', pown_cpu_func)
  
  ##### gentype powr(gentype x, gentype y)
  powr_input_values1 = [80, -80, 3.14, -3.14, 0.5, 1, -1, 0.0,6,1500.24,-1500.24]
  powr_input_values2 = [5,6,7,8,10,11,12,13,14,0,12]
  powr_input_type1 = ['float','float2','float4','float8','float16']
  powr_input_type2 = ['float','float2','float4','float8','float16']
  powr_output_type = ['float','float2','float4','float8','float16']
  powr_cpu_func='''
static float powr(float x, int y){
    return powf(x,y);
} '''
  powrUtests = func('powr','powr',[powr_input_type1,powr_input_type2],powr_output_type,[powr_input_values1,powr_input_values2],'16 * FLT_ULP', powr_cpu_func)
  
  ##### gentype remainder(gentype x, gentype y)
  remainder_base_values = base_input_values
  remainder_input_values1 = []
  remainder_input_values2 = []
  remainder_input_values1,remainder_input_values2=gene2ValuesLoop(remainder_input_values1,remainder_input_values2,remainder_base_values)
  remainder_input_type1 = ['float','float2','float4','float8','float16']
  remainder_input_type2 = ['float','float2','float4','float8','float16']
  remainder_output_type = ['float','float2','float4','float8','float16']
  remainderUtests = func('remainder','remainder',[remainder_input_type1,remainder_input_type2],remainder_output_type,[remainder_input_values1,remainder_input_values2],'0 * FLT_ULP')
  
  ##### gentype rint(gentype x)
  rint_input_values = base_input_values
  rint_input_type = ['float','float2','float4','float8','float16']
  rint_output_type = ['float','float2','float4','float8','float16']
  rintUtests = func('rint','rint',[rint_input_type],rint_output_type,[rint_input_values],'0 * FLT_ULP')
  
  ##### floatn rootn(floatn x, intn y)
  rootn_input_values1 = [0.0, 0.0012,  0.5, 1, 3.14, 12345]
  rootn_input_values2 = [-1, 1, -20, 20, -123, 456]
  rootn_input_type1 = ['float','float2','float4','float8','float16']
  rootn_input_type2 = ['int','int2','int4','int8','int16']
  rootn_output_type = ['float','float2','float4','float8','float16']
  rootn_cpu_func='''
static float rootn(float x, int y){
    return pow(x,1.0/y);
} '''
  rootnUtests = func('rootn','rootn',[rootn_input_type1,rootn_input_type2],rootn_output_type,[rootn_input_values1,rootn_input_values2],'4 * FLT_ULP',rootn_cpu_func)
  
  ##### gentype round(gentype x)
  round_input_values = base_input_values
  round_input_type = ['float','float2','float4','float8','float16']
  round_output_type = ['float','float2','float4','float8','float16']
  roundUtests = func('round','round',[round_input_type],round_output_type,[round_input_values],'0 * FLT_ULP')
  
  ##### gentype rsqrt(gentype)
  rsqrt_input_values = base_input_values
  rsqrt_input_type = ['float','float2','float4','float8','float16']
  rsqrt_output_type = ['float','float2','float4','float8','float16']
  rsqrt_cpu_func='''
static float rsqrt(float x)
{ return 1/sqrt(x);} '''
  rsqrtUtests = func('rsqrt','rsqrt',[rsqrt_input_type],rsqrt_output_type,[rsqrt_input_values],'4 * FLT_ULP', rsqrt_cpu_func)

 
  ##### gentype sin(gentype)
  sin_input_values = base_input_values
  sin_input_type = ['float','float2','float4','float8','float16']
  sin_output_type = ['float','float2','float4','float8','float16']
  sinUtests = func('sin','sin',[sin_input_type],sin_output_type,[sin_input_values],'4 * FLT_ULP')
  
#  ##### gentype sincos(gentype)
#  sincos_input_values1 = [FLT_MAX_POSI,FLT_MIN_NEGA,FLT_MIN_POSI,FLT_MAX_NEGA,80, -80, 3.14, -3.14, -0.5, 0.5, 1, -1, 0.0,6,-6,1500.24,-1500.24]
#  sincos_input_values2 = []
#  sincos_input_type1 = ['float','float2','float4','float8','float16']
#  sincos_input_type2 = ['float','float2','float4','float8','float16']
#  sincos_output_type = ['float','float2','float4','float8','float16']
#  ###### gentype sincos(gentype)
#  #  sincosUtests = func('sincos','sincos',[sincos_input_type1,sincos_input_type2],sincos_output_type,[sincos_input_values1,sincos_input_values2],'4 * FLT_ULP')
  
  ##### gentype sinh(gentype)
  sinh_input_values = base_input_values
  sinh_input_type = ['float','float2','float4','float8','float16']
  sinh_output_type = ['float','float2','float4','float8','float16']
  sinhUtests = func('sinh','sinh',[sinh_input_type],sinh_output_type,[sinh_input_values],'4 * FLT_ULP')
  
  ##### gentype sinpi(gentype x)
  sinpi_input_values = [0, 1, 3.14, -0.88, -0.12, -0.5, 0.5, -0.49, 0.49, 0.51, -0.51, -0.1, 0.1]
  sinpi_input_type = ['float','float2','float4','float8','float16']
  sinpi_output_type = ['float','float2','float4','float8','float16']
  sinpi_cpu_func=reduce1+sinpi
  sinpiUtests = func('sinpi','sinpi',[sinpi_input_type],sinpi_output_type,[sinpi_input_values],'4 * FLT_ULP',sinpi_cpu_func)
  
  ##### gentype sqrt(gentype)
  sqrt_input_values = base_input_values
  sqrt_input_type = ['float','float2','float4','float8','float16']
  sqrt_output_type = ['float','float2','float4','float8','float16']
  sqrtUtests = func('sqrt','sqrt',[sqrt_input_type],sqrt_output_type,[sqrt_input_values],'4 * FLT_ULP')
  
  ##### gentype tan(gentype)
  tan_input_values = base_input_values
  tan_input_type = ['float','float2','float4','float8','float16']
  tan_output_type = ['float','float2','float4','float8','float16']
  tanUtests = func('tan','tan',[tan_input_type],tan_output_type,[tan_input_values],'5 * FLT_ULP')
  
  ##### gentype tanh(gentype)
  tanh_input_values = base_input_values
  tanh_input_type = ['float','float2','float4','float8','float16']
  tanh_output_type = ['float','float2','float4','float8','float16']
  tanhUtests = func('tanh','tanh',[tanh_input_type],tanh_output_type,[tanh_input_values],'5 * FLT_ULP')
  
  ##### gentype tanpi(gentype x)
  tanpi_input_values = [ 0, 3.14, 5.15, 6.01, 7.89]
  tanpi_input_type = ['float','float2','float4','float8','float16']
  tanpi_output_type = ['float','float2','float4','float8','float16']
  tanpi_cpu_func=reduce1+sinpi+cospi+'''
static float tanpi(float x){
  return sinpi(x)/cospi(x);
}
'''
  tanpiUtests = func('tanpi','tanpi',[tanpi_input_type],tanpi_output_type,[tanpi_input_values],'400 * FLT_ULP',tanpi_cpu_func)
  
  ##### gentype trunc(gentype)
  trunc_input_values = base_input_values
  trunc_input_type = ['float','float2','float4','float8','float16']
  trunc_output_type = ['float','float2','float4','float8','float16']
  truncUtests = func('trunc','trunc',[trunc_input_type],trunc_output_type,[trunc_input_values],'0 * FLT_ULP')

if __name__ == "__main__":
  main()
