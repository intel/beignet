/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/**
 * \file sim_vector.h
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * c++ class helper for the simulator
 */

#ifndef __GBE_SIM_VECTOR_H__
#define __GBE_SIM_VECTOR_H__

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>
#include <cmath>

#define INLINE inline __attribute__((always_inline))

/*! Base structure for 1 / 4 / 8 / 16 / 32 floats */
template <uint32_t vectorNum, bool scalar = false>
struct genf { __m128 m[vectorNum]; };
/*! Base structure for 1 / 4 / 8 / 16 / 32 integers */
template <uint32_t vectorNum, bool scalar = false>
struct geni { __m128i m[vectorNum]; };
/*! Base structure for 1 / 4 / 8 / 16 / 32 booleans (m stands for "mask") */
template <uint32_t vectorNum, bool scalar = false>
struct genm { __m128 m[vectorNum]; };

/*! To cast through memory */
union CastType {
  INLINE CastType(uint32_t u0, uint32_t u1, uint32_t u2, uint32_t u3) {
    u[0] = u0; u[1] = u1; u[2] = u2; u[3] = u3;
  }
  INLINE CastType(float f0, float f1, float f2, float f3) {
    f[0] = f0; f[1] = f1; f[2] = f2; f[3] = f3;
  }
  __m128 v;
  __m128i vi;
  uint32_t u[4];
  float f[4];
};

typedef genf<1,true>  genf1; // contains 3 clobbered values
typedef genf<1,false> genf4;
typedef genf<2,false> genf8;
typedef genf<4,false> genf16;
typedef genf<8,false> genf32;
typedef geni<1,true>  geni1; // contains 3 clobbered values
typedef geni<1,false> geni4;
typedef geni<2,false> geni8;
typedef geni<4,false> geni16;
typedef geni<8,false> geni32;
typedef genm<1,true>  genm1; // contains 3 clobbered values
typedef genm<1,false> genm4;
typedef genm<2,false> genm8;
typedef genm<4,false> genm16;
typedef genm<8,false> genm32;

static INLINE uint32_t elemNum(genf1 x)  { return 1; }
static INLINE uint32_t elemNum(genf4 x)  { return 4; }
static INLINE uint32_t elemNum(genf8 x)  { return 8; }
static INLINE uint32_t elemNum(genf16 x) { return 16; }
static INLINE uint32_t elemNum(genf32 x) { return 32; }
static INLINE uint32_t elemNum(geni1 x)  { return 1; }
static INLINE uint32_t elemNum(geni4 x)  { return 4; }
static INLINE uint32_t elemNum(geni8 x)  { return 8; }
static INLINE uint32_t elemNum(geni16 x) { return 16; }
static INLINE uint32_t elemNum(geni32 x) { return 32; }

template<size_t i0, size_t i1, size_t i2, size_t i3>
INLINE const __m128 shuffle(const __m128& b) {
  return _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(b), _MM_SHUFFLE(i3, i2, i1, i0)));
}

template<size_t i> INLINE
__m128 expand(const __m128& b) { 
  return shuffle<i, i, i, i>(b);
}

template<size_t index_0, size_t index_1, size_t index_2, size_t index_3>
INLINE const __m128i shuffle(const __m128i& a) {
  return _mm_shuffle_epi32(a, _MM_SHUFFLE(index_3, index_2, index_1, index_0));
}

template<size_t index>
INLINE const __m128i expand(const __m128i& b) {
  return shuffle<index, index, index, index>(b);
}

/* Build an integer mask from the mask vectors */
template <uint32_t vectorNum>
INLINE uint32_t mask(const genm<vectorNum> v) {
  uint32_t m = _mm_movemask_ps(v.m[0]);
  for (uint32_t i = 1; i < vectorNum; ++i)
    m |= _mm_movemask_ps(v.m[i]) << (4*i);
  return m;
}
INLINE uint32_t mask(const genm1 &v) { return _mm_movemask_ps(v.m[0]) & 1; }

#define ID(X) X
#define VEC_OP(DST_TYPE, SRC_TYPE, NAME, INTRINSIC_NAME, FN, FN0, FN1)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE<vectorNum> &dst,\
                 const SRC_TYPE<vectorNum> &v0,\
                 const SRC_TYPE<vectorNum> &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = FN(INTRINSIC_NAME(FN0(v0.m[i]), FN1(v1.m[i])));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE<vectorNum> &dst,\
                 const SRC_TYPE<vectorNum> &v0,\
                 const SRC_TYPE##1 &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = FN(INTRINSIC_NAME(FN0(v0.m[i]), FN1(expand<0>(v1.m[0]))));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE<vectorNum> &dst,\
                 const SRC_TYPE##1 &v0,\
                 const SRC_TYPE<vectorNum> &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = FN(INTRINSIC_NAME(FN0(expand<0>(v0.m[0])), FN1(v1.m[i])));\
}

VEC_OP(genf, genf, ADD, _mm_add_ps, ID, ID, ID);
VEC_OP(genf, genf, SUB, _mm_sub_ps, ID, ID, ID);
VEC_OP(genf, genf, MUL, _mm_mul_ps, ID, ID, ID);
VEC_OP(genf, genf, DIV, _mm_div_ps, ID, ID, ID);
VEC_OP(genm, genf, EQ, _mm_cmpeq_ps, ID, ID, ID);
VEC_OP(genm, genf, NE, _mm_cmpneq_ps, ID, ID, ID);
VEC_OP(genm, genf, LT, _mm_cmplt_ps, ID, ID, ID);
VEC_OP(genm, genf, LE, _mm_cmple_ps, ID, ID, ID);
VEC_OP(genm, genf, GT, _mm_cmpgt_ps, ID, ID, ID);
VEC_OP(genm, genf, GE, _mm_cmpge_ps, ID, ID, ID);
VEC_OP(geni, geni, ADD, _mm_add_epi32, ID, ID, ID);
VEC_OP(geni, geni, SUB, _mm_sub_epi32, ID, ID, ID);
VEC_OP(genm, geni, EQ, _mm_cmpeq_epi32, ID, ID, ID);
VEC_OP(genm, geni, SLT, _mm_cmplt_epi32, ID, ID, ID);
VEC_OP(genm, geni, SGT, _mm_cmpgt_epi32, ID, ID, ID);
VEC_OP(geni, geni, OR, _mm_or_ps, _mm_castps_si128, _mm_castsi128_ps, _mm_castsi128_ps);
VEC_OP(geni, geni, XOR, _mm_xor_ps, _mm_castps_si128, _mm_castsi128_ps, _mm_castsi128_ps);
VEC_OP(geni, geni, AND, _mm_and_ps, _mm_castps_si128, _mm_castsi128_ps, _mm_castsi128_ps);
VEC_OP(genm, genf, SLT, _mm_cmplt_ps, ID, ID, ID);
VEC_OP(genm, genf, SLE, _mm_cmple_ps, ID, ID, ID);
VEC_OP(genm, genf, SGT, _mm_cmpgt_ps, ID, ID, ID);
VEC_OP(genm, genf, SGE, _mm_cmpge_ps, ID, ID, ID);

#undef VEC_OP

#define ICMP_VEC_OP(DST_TYPE, SRC_TYPE, NAME, INTRINSIC_NAME, FN, FN0, FN1)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE<vectorNum> &dst,\
                 const SRC_TYPE<vectorNum> &v0,\
                 const SRC_TYPE<vectorNum> &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = FN(INTRINSIC_NAME(FN1(v1.m[i]), FN0(v0.m[i])));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE<vectorNum> &dst,\
                 const SRC_TYPE<vectorNum> &v0,\
                 const SRC_TYPE##1 &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = FN(INTRINSIC_NAME(FN1(expand<0>(v1.m[0])), FN0(v0.m[i])));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE<vectorNum> &dst,\
                 const SRC_TYPE##1 &v0,\
                 const SRC_TYPE<vectorNum> &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = FN(INTRINSIC_NAME(FN1(v1.m[i]), FN0(expand<0>(v0.m[0]))));\
}
ICMP_VEC_OP(genm, geni, SGE, _mm_cmplt_epi32, ID, ID, ID);
ICMP_VEC_OP(genm, geni, SLE, _mm_cmpgt_epi32, ID, ID, ID);
#undef ICMP_VEC_OP

static const CastType alltrue(0xffffffff,0xffffffff,0xffffffff,0xffffffff);

template <uint32_t vectorNum>
INLINE void NE(genm<vectorNum> &dst, const geni<vectorNum> &v0, const geni<vectorNum> &v1) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_castps_si128(_mm_xor_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(v0.m[i], v1.m[i])),alltrue.v));
}
template <uint32_t vectorNum>
INLINE void NE(genm<vectorNum> &dst, const geni<vectorNum> &v0, const geni1 &v1) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_castps_si128(_mm_xor_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(v0.m[i], v1.m[0])),alltrue.v));
}
template <uint32_t vectorNum>
INLINE void NE(genm<vectorNum> &dst, const geni1 &v0, const geni<vectorNum> &v1) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_castps_si128(_mm_xor_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(v0.m[0], v1.m[i])),alltrue.v));
}

#define SCALAR_OP(TYPE, NAME, INTRINSIC_NAME)\
INLINE void NAME(TYPE &dst, const TYPE &v0, const TYPE &v1) {\
  dst.m[0] = INTRINSIC_NAME(v0.m[0], v1.m[0]);\
}
SCALAR_OP(genf1, ADD, _mm_add_ss);
SCALAR_OP(genf1, SUB, _mm_sub_ss);
SCALAR_OP(genf1, MUL, _mm_mul_ss);
SCALAR_OP(genf1, DIV, _mm_div_ss);
SCALAR_OP(geni1, ADD, _mm_add_epi32);
SCALAR_OP(geni1, SUB, _mm_sub_epi32);
#undef SCALAR_OP

/* load from contiguous floats / integers */
template <uint32_t vectorNum>
INLINE void LOAD(genf<vectorNum> &dst, const char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_loadu_ps((const float*) ptr + 4*i);
}
INLINE void LOAD(genf1 &dst, const char *ptr) {
  dst.m[0] = _mm_load_ss((const float*) ptr);
}
template <uint32_t vectorNum>
INLINE void LOAD(geni<vectorNum> &dst, const char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_castps_si128(_mm_loadu_ps((const float*) ptr + 4*i));
}
INLINE void LOAD(geni1 &dst, const char *ptr) {
  dst.m[0] = _mm_castps_si128(_mm_load_ss((const float*) ptr));
}

/* store to contiguous floats / integers */
template <uint32_t vectorNum>
INLINE void STORE(const genf<vectorNum> &src, char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    _mm_storeu_ps((float*) ptr + 4*i, src.m[i]);
}
INLINE void STORE(genf1 &src, char *ptr) {
  _mm_store_ss((float*) ptr, src.m[0]);
}
template <uint32_t vectorNum>
INLINE void STORE(const geni<vectorNum> &src, char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
     _mm_storeu_ps((float*) ptr + 4*i, _mm_castsi128_ps(src.m[i]));
}
INLINE void STORE(const geni1 &src, char *ptr) {
  _mm_store_ss((float*) ptr, _mm_castsi128_ps(src.m[0]));
}

/* Load immediates */
template <uint32_t vectorNum>
INLINE void LOADI(genf<vectorNum> &dst, float f) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_load1_ps(&f);
}
INLINE void LOADI(genf1 &dst, float f) { dst.m[0] = _mm_load_ss(&f); }
template <uint32_t vectorNum>
INLINE void LOADI(geni<vectorNum> &dst, uint32_t u) {
  union { float f; uint32_t u; } cast;
  cast.u = u;
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_castps_si128(_mm_load1_ps(&cast.f));
}
INLINE void LOADI(geni1 &dst, uint32_t u) {
  union { float f; uint32_t u; } cast;
  cast.u = u;
  dst.m[0] = _mm_castps_si128(_mm_load_ss(&cast.f));
}

/* Scatter */
#define SCATTER_OP(TYPE, FN)\
template <uint32_t vectorNum>\
INLINE void SCATTER(const TYPE<vectorNum> &value,\
                    const geni<vectorNum> &offset,\
                    char *base_address) {\
  for (uint32_t i = 0; i < vectorNum; ++i) {\
    const int v0 = _mm_extract_epi32(FN(value.m[i]), 0);\
    const int v1 = _mm_extract_epi32(FN(value.m[i]), 1);\
    const int v2 = _mm_extract_epi32(FN(value.m[i]), 2);\
    const int v3 = _mm_extract_epi32(FN(value.m[i]), 3);\
    const int o0 = _mm_extract_epi32(offset.m[i], 0);\
    const int o1 = _mm_extract_epi32(offset.m[i], 1);\
    const int o2 = _mm_extract_epi32(offset.m[i], 2);\
    const int o3 = _mm_extract_epi32(offset.m[i], 3);\
    *(int*)(base_address + o0) = v0;\
    *(int*)(base_address + o1) = v1;\
    *(int*)(base_address + o2) = v2;\
    *(int*)(base_address + o3) = v3;\
  }\
}\
INLINE void SCATTER(const TYPE##1 &value, const geni1 &offset, char *base_address) {\
  const int v0 = _mm_extract_epi32(FN(value.m[0]), 0);\
  const int o0 = _mm_extract_epi32(offset.m[0], 0);\
  *(int*)(base_address + o0) = v0;\
}
SCATTER_OP(genf, _mm_castps_si128)
SCATTER_OP(geni, ID)
#undef SCATTER_OP

/* Gather */
#define GATHER_OP(TYPE, FN)\
template <uint32_t vectorNum>\
INLINE void GATHER(TYPE<vectorNum> &dst,\
                   const geni<vectorNum> &offset,\
                   char *base_address) {\
  for (uint32_t i = 0; i < vectorNum; ++i) {\
    const int o0 = _mm_extract_epi32(offset.m[i], 0);\
    const int o1 = _mm_extract_epi32(offset.m[i], 1);\
    const int o2 = _mm_extract_epi32(offset.m[i], 2);\
    const int o3 = _mm_extract_epi32(offset.m[i], 3);\
    const int v0 = *(int*)(base_address + o0);\
    const int v1 = *(int*)(base_address + o1);\
    const int v2 = *(int*)(base_address + o2);\
    const int v3 = *(int*)(base_address + o3);\
    _mm_insert_epi32(FN(dst.m[i]), v0, 0);\
    _mm_insert_epi32(FN(dst.m[i]), v1, 1);\
    _mm_insert_epi32(FN(dst.m[i]), v2, 2);\
    _mm_insert_epi32(FN(dst.m[i]), v3, 3);\
  }\
}\
INLINE void GATHER(TYPE##1 &dst, const geni1 &offset, char *base_address) {\
    const int o0 = _mm_extract_epi32(offset.m[0], 0);\
    const int v0 = *(int*)(base_address + o0);\
    _mm_insert_epi32(FN(dst.m[0]), v0, 0);\
}
GATHER_OP(genf, _mm_castps_si128)
GATHER_OP(geni, ID)
#undef GATHER_OP

#undef ID
#undef INLINE

#endif /* __GBE_SIM_VECTOR_H__ */

