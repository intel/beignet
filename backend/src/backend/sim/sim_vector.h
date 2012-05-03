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
#define ID(X) (X)
#define PS2SI(X) _mm_castps_si128(X)
#define SI2PS(X) _mm_castsi128_ps(X)

/* Some extra SSE functions */
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

/*! Base structure for scalar double word (32 bits) */
union scalar_dw {
  INLINE scalar_dw(void) {}
  INLINE scalar_dw(uint32_t u) { this->u = u; }
  INLINE scalar_dw(int32_t s) { this->s = s; }
  INLINE scalar_dw(float f) { this->f = f; }
  uint32_t u; int32_t s; float f;
};

/*! Base structure for scalar word (16 bits) */
union scalar_w {
  INLINE scalar_w(void) {}
  INLINE scalar_w(uint16_t u) { this->u = u; }
  INLINE scalar_w(int16_t s) { this->s = s; }
  INLINE float toFloat(void) const {
    union {uint16_t u[2]; float f;} x;
    x.u[0] = u;
    x.u[1] = 0;
    return x.f;
  }
  uint16_t u; int16_t s;
};

/*! Base structure for scalar mask */
union scalar_m { uint32_t u; int32_t s; float f; };

/*! Base structure for vectors 4 / 8 / 16 / 32 double words */
template <uint32_t vectorNum>
struct simd_dw {
  INLINE simd_dw(void) {}
  INLINE simd_dw(const scalar_dw &s) {
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&s.f);
  }
  simd_dw &operator= (const scalar_dw &s) {
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&s.f);
    return *this;
  }
  __m128 m[vectorNum];
};

/*! Base structure for vectors 4 / 8 / 16 / 32 words. We do not store 8 shorts
 *  but only 4. This makes everything much simpler even if it is clearly slower
 */
template <uint32_t vectorNum>
struct simd_w {
  INLINE simd_w(void) {}
  INLINE simd_w(const scalar_w &s) {
    const float f = s.toFloat();
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&f);
  }
  simd_w &operator= (const scalar_w &s) {
    const float f = s.toFloat();
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&f);
    return *this;
  }
  __m128 m[vectorNum];
};

/*! Base structure for 4 / 8 / 16 / 32 booleans (m stands for "mask") */
template <uint32_t vectorNum>
struct simd_m {
  INLINE simd_m(void) {}
  INLINE simd_m(scalar_m s) {
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&s.f);
  }
  __m128 m[vectorNum];
};

/*! Select instruction on vectors */
template <uint32_t vectorNum>
INLINE void select(simd_dw<vectorNum> &dst,
                   const simd_dw<vectorNum> &src0,
                   const simd_dw<vectorNum> &src1,
                   const simd_m<vectorNum> &mask)
{
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_blendv_ps(src0.m[i], src1.m[i], mask.m[i]);
}
template <uint32_t vectorNum>
INLINE void select(simd_m<vectorNum> &dst,
                   const simd_m<vectorNum> &src0,
                   const simd_m<vectorNum> &src1,
                   const simd_m<vectorNum> &mask)
{
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_blendv_ps(src0.m[i], src1.m[i], mask.m[i]);
}

/*! To cast through memory 32 bits values in sse registers */
union cast_dw {
  INLINE cast_dw(uint32_t u0, uint32_t u1, uint32_t u2, uint32_t u3) {
    u[0] = u0; u[1] = u1; u[2] = u2; u[3] = u3;
  }
  INLINE cast_dw(int32_t s0, int32_t s1, int32_t s2, int32_t s3) {
    s[0] = s0; s[1] = s1; s[2] = s2; s[3] = s3;
  }
  INLINE cast_dw(float f0, float f1, float f2, float f3) {
    f[0] = f0; f[1] = f1; f[2] = f2; f[3] = f3;
  }
  INLINE cast_dw(const __m128 &v) : v(v) {}
  INLINE cast_dw(const __m128i &vi) : vi(vi) {}
  INLINE cast_dw(void) {}
  __m128 v;
  __m128i vi;
  uint32_t u[4];
  int32_t s[4];
  float f[4];
};
static const cast_dw allTrue(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

/*! To cast through memory 16 bits values in sse registers */
union cast_w {
  INLINE cast_w(int16_t s0, int16_t s1, int16_t s2, int16_t s3) {
    s[0].v = s0; s[1].v = s1; s[2].v = s2; s[3].v = s3;
    s[0].pad = s[1].pad = s[2].pad = s[3].pad = 0;
  }
  INLINE cast_w(uint16_t u0, uint16_t u1, uint16_t u2, uint16_t u3) {
    u[0].v = u0; u[1].v = u1; u[2].v = u2; u[3].v = u3;
    u[0].pad = u[1].pad = u[2].pad = u[3].pad = 0;
  }
  INLINE cast_w(const __m128 &v) : v(v) {}
  INLINE cast_w(const __m128i &vi) : vi(vi) {}
  INLINE cast_w(void) {}
  __m128 v;
  __m128i vi;
  struct { uint16_t v; uint16_t pad; } u[4];
  struct {  int16_t v;  int16_t pad; } s[4];
};

/*! Make a mask true */
template <uint32_t vectorNum>
INLINE void allTrueMask(simd_m<vectorNum> &x) {
  for (uint32_t i = 0; i < vectorNum; ++i) x.m[i] = allTrue.v;
}

/* Some convenient typedefs */
typedef scalar_dw  simd1dw;
typedef simd_dw<1> simd4dw;
typedef simd_dw<2> simd8dw;
typedef simd_dw<4> simd16dw;
typedef simd_dw<8> simd32dw;
typedef scalar_w   simd1w;
typedef simd_w<1>  simd4w;
typedef simd_w<2>  simd8w;
typedef simd_w<4>  simd16w;
typedef simd_w<8>  simd32w;
typedef scalar_m   simd1m;
typedef simd_m<1>  simd4m;
typedef simd_m<2>  simd8m;
typedef simd_m<4>  simd16m;
typedef simd_m<8>  simd32m;

//////////////////////////////////////////////////////////////////////////////
// Vector instructions
//////////////////////////////////////////////////////////////////////////////
/* Simple function to get the number of element per vector */
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_dw<vectorNum> &x) {
  return 4*vectorNum;
}
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_m<vectorNum> &x) {
  return 4*vectorNum;
}
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_w<vectorNum> &x) {
  return 4*vectorNum;
}

/* Build an integer mask from the mask vectors */
template <uint32_t vectorNum>
INLINE uint32_t mask(const simd_m<vectorNum> v) {
  uint32_t m = _mm_movemask_ps(v.m[0]);
  for (uint32_t i = 1; i < vectorNum; ++i)
    m |= (_mm_movemask_ps(v.m[i]) << (4*i));
  return m;
}

/* MOV instruction */
template <uint32_t vectorNum>
INLINE void MOV_S32(simd_dw<vectorNum> &dst, const simd_dw<vectorNum> &v) {
  for (uint32_t i = 0; i < vectorNum; ++i) dst.m[i] = v.m[i];
}
template <uint32_t vectorNum>
INLINE void MOV_S32(simd_dw<vectorNum> &dst, const scalar_dw &x) {
  const __m128 v = _mm_load1_ps(&x.f);
  for (uint32_t i = 0; i < vectorNum; ++i) dst.m[i] = v;
}
template <uint32_t vectorNum>
INLINE void MOV_S16(simd_w<vectorNum> &dst, const simd_w<vectorNum> &v) {
  for (uint32_t i = 0; i < vectorNum; ++i) dst.m[i] = v.m[i];
}
template <uint32_t vectorNum>
INLINE void MOV_S16(simd_w<vectorNum> &dst, const scalar_w &x) {
  const float f = x.toFloat();
  const __m128 v = _mm_load1_ps(&f);
  for (uint32_t i = 0; i < vectorNum; ++i) dst.m[i] = v;
}

/* Vector instructions that use sse* */
#define VEC_OP(DST_TYPE, SRC_TYPE, SCALAR_TYPE, NAME, INTRINSIC_NAME, FN, FN0, FN1)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = FN(INTRINSIC_NAME(FN0(v0.m[i]), FN1(v1.m[i])));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SCALAR_TYPE &v1) {\
  NAME(dst, v0, SRC_TYPE(v1));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SCALAR_TYPE &v0, const SRC_TYPE &v1) {\
  NAME(dst, SRC_TYPE(v0), v1);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SCALAR_TYPE &v0, const SCALAR_TYPE &v1) {\
  NAME(dst, SRC_TYPE(v0), SRC_TYPE(v1));\
}
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, ADD_F, _mm_add_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, SUB_F, _mm_sub_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, MUL_F, _mm_mul_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, DIV_F, _mm_div_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, EQ_F, _mm_cmpeq_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, NE_F, _mm_cmpneq_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, LT_F, _mm_cmplt_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, LE_F, _mm_cmple_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, GT_F, _mm_cmpgt_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, GE_F, _mm_cmpge_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, ADD_S32, _mm_add_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, SUB_S32, _mm_sub_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, EQ_S32, _mm_cmpeq_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, LT_S32, _mm_cmplt_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, scalar_dw, GT_S32, _mm_cmpgt_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, AND_S32, _mm_and_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, OR_S32, _mm_or_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, XOR_S32, _mm_xor_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_w<vectorNum>,  scalar_w,  EQ_S16, _mm_cmpeq_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w, ADD_S16, _mm_add_epi16, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w, SUB_S16, _mm_sub_epi16, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w, AND_S16, _mm_and_ps, ID, ID, ID);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w, OR_S16, _mm_or_ps, ID, ID, ID);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w, XOR_S16, _mm_xor_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_m<vectorNum>,  scalar_m, AND_M,   _mm_and_ps, ID, ID, ID);
#undef VEC_OP

/* Vector integer operations that we can get by switching argument order */
#define VEC_OP(DST_TYPE, SRC_TYPE, NAME, INTRINSIC_NAME, FN, FN0, FN1)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = _mm_xor_ps(FN(INTRINSIC_NAME(FN1(v0.m[i]), FN0(v1.m[i]))), allTrue.v);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const scalar_dw &v1) {\
  NAME(dst, v0, simd_dw<vectorNum>(v1));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const SRC_TYPE &v1) {\
  NAME(dst, simd_dw<vectorNum>(v0), v1);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const scalar_dw &v1) {\
  NAME(dst, simd_dw<vectorNum>(v0), simd_dw<vectorNum>(v1));\
}
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, GE_S32, _mm_cmplt_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, LE_S32, _mm_cmpgt_epi32, SI2PS, PS2SI, PS2SI);
#undef VEC_OP

/* Vector binary integer operations that require C */
#define VEC_OP(DST_TYPE, SRC_TYPE, SCALAR_TYPE, CAST_TYPE, NAME, OP, FIELD)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i) {\
    CAST_TYPE c0(v0.m[i]), c1(v1.m[i]), d;\
    for (uint32_t j = 0; j < 4; ++j)\
      d.FIELD = c0.FIELD OP c1.FIELD;\
    dst.m[i] = d.v;\
  }\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SCALAR_TYPE &v1) {\
  NAME(dst, v0, SRC_TYPE(v1));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SCALAR_TYPE &v0, const SRC_TYPE &v1) {\
  NAME(dst, SRC_TYPE(v0), v1);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SCALAR_TYPE &v0, const SCALAR_TYPE &v1) {\
  NAME(dst, SRC_TYPE(v0), SRC_TYPE(v1));\
}
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, MUL_S32, *, s[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, DIV_S32, /, s[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, REM_S32, %, s[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, MUL_U32, *, u[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, DIV_U32, /, u[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, REM_U32, %, u[j]);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  MUL_S16, *, s[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  DIV_S16, /, s[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  REM_S16, %, s[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  MUL_U16, *, u[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  DIV_U16, /, u[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  REM_U16, %, u[j].v);
#undef VEC_OP

/* Vector compare vectors that require C */
#define VEC_OP(DST_TYPE, SRC_TYPE, SCALAR_TYPE, CAST_TYPE, NAME, OP, FIELD)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i) {\
    CAST_TYPE c0(v0.m[i]), c1(v1.m[i]);\
    cast_dw d;\
    for (uint32_t j = 0; j < 4; ++j)\
      d.u[j] = (c0.FIELD OP c1.FIELD) ? ~0u : 0u;\
    dst.m[i] = d.v;\
  }\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SCALAR_TYPE &v1) {\
  NAME(dst, v0, SRC_TYPE(v1));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SCALAR_TYPE &v0, const SRC_TYPE &v1) {\
  NAME(dst, SRC_TYPE(v0), v1);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SCALAR_TYPE &v0, const SCALAR_TYPE &v1) {\
  NAME(dst, SRC_TYPE(v0), SRC_TYPE(v1));\
}
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, LE_U32, <=, u[j]);
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, LT_U32, <, u[j]);
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, GE_U32, >=, u[j]);
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, GT_U32, >, u[j]);
VEC_OP(simd_m<vectorNum>, simd_w<vectorNum>,  scalar_w,  cast_w,  LE_U16, <=, u[j].v);
VEC_OP(simd_m<vectorNum>, simd_w<vectorNum>,  scalar_w,  cast_w,  LT_U16, <, u[j].v);
VEC_OP(simd_m<vectorNum>, simd_w<vectorNum>,  scalar_w,  cast_w,  GE_U16, >=, u[j].v);
VEC_OP(simd_m<vectorNum>, simd_w<vectorNum>,  scalar_w,  cast_w,  GT_U16, >, u[j].v);
VEC_OP(simd_m<vectorNum>, simd_w<vectorNum>,  scalar_w,  cast_w,  LE_S16, <=, s[j].v);
VEC_OP(simd_m<vectorNum>, simd_w<vectorNum>,  scalar_w,  cast_w,  LT_S16, <, s[j].v);
VEC_OP(simd_m<vectorNum>, simd_w<vectorNum>,  scalar_w,  cast_w,  GE_S16, >=, s[j].v);
VEC_OP(simd_m<vectorNum>, simd_w<vectorNum>,  scalar_w,  cast_w,  GT_S16, >, s[j].v);
#undef VEC_OP

/* Get NE from EQ */
template <uint32_t vectorNum>
INLINE void NE_S32(simd_m<vectorNum> &dst,
                   const simd_dw<vectorNum> &v0,
                   const simd_dw<vectorNum> &v1)
{
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_xor_ps(allTrue.v, SI2PS(_mm_cmpeq_epi32(PS2SI(v0.m[i]), PS2SI(v1.m[i]))));
}
template <uint32_t vectorNum>
INLINE void NE_S32(simd_m<vectorNum> &dst,
                   const simd_dw<vectorNum> &v0,
                   const scalar_dw &v1)
{
  NE_S32(dst, v0, simd_dw<vectorNum>(v1));
}
template <uint32_t vectorNum>
INLINE void NE_S32(simd_m<vectorNum> &dst,
                   const scalar_dw &v0,
                   const simd_dw<vectorNum> &v1)
{
  NE_S32(dst, simd_dw<vectorNum>(v0), v1);
}
template <uint32_t vectorNum>
INLINE void NE_S16(simd_m<vectorNum> &dst,
                   const simd_w<vectorNum> &v0,
                   const simd_w<vectorNum> &v1)
{
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_xor_ps(allTrue.v, SI2PS(_mm_cmpeq_epi32(PS2SI(v0.m[i]), PS2SI(v1.m[i]))));
}
template <uint32_t vectorNum>
INLINE void NE_S16(simd_m<vectorNum> &dst,
                   const simd_w<vectorNum> &v0,
                   const scalar_w &v1)
{
  NE_S16(dst, v0, simd_w<vectorNum>(v1));
}
template <uint32_t vectorNum>
INLINE void NE_S16(simd_m<vectorNum> &dst,
                   const scalar_w &v0,
                   const simd_w<vectorNum> &v1)
{
  NE_S16(dst, simd_w<vectorNum>(v0), v1);
}
template <uint32_t vectorNum>
INLINE void NE_S16(simd_m<vectorNum> &dst,
                   const scalar_w &v0,
                   const scalar_w &v1)
{
  NE_S16(dst, simd_w<vectorNum>(v0), simd_w<vectorNum>(v1));
}

/* Load from contiguous double words */
template <uint32_t vectorNum>
INLINE void LOAD(simd_dw<vectorNum> &dst, const char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_loadu_ps((const float*) ptr + 4*i);
}

/* Store to contiguous double words */
template <uint32_t vectorNum>
INLINE void STORE(const simd_dw<vectorNum> &src, char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    _mm_storeu_ps((float*) ptr + 4*i, src.m[i]);
}

/* Load from contiguous words */
template <uint32_t vectorNum>
INLINE void LOAD(simd_w<vectorNum> &dst, const char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const uint16_t u0 = *((uint16_t*) ptr + 4*i + 0);
    const uint16_t u1 = *((uint16_t*) ptr + 4*i + 1);
    const uint16_t u2 = *((uint16_t*) ptr + 4*i + 2);
    const uint16_t u3 = *((uint16_t*) ptr + 4*i + 3);
    const cast_w w(u0,u1,u2,u3);
    dst.m[i] = w.v;
  }
}

/* Store to contiguous words */
template <uint32_t vectorNum>
INLINE void STORE(const simd_w<vectorNum> &src, char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const cast_w w(src.m[i]);
    *((uint16_t*) ptr + 4*i + 0) = w.u[0].v;
    *((uint16_t*) ptr + 4*i + 1) = w.u[1].v;
    *((uint16_t*) ptr + 4*i + 2) = w.u[2].v;
    *((uint16_t*) ptr + 4*i + 3) = w.u[3].v;
  }
}

/* Load immediates */
template <uint32_t vectorNum>
INLINE void LOADI(simd_dw<vectorNum> &dst, uint32_t u) {
  union { uint32_t u; float f; } cast;
  cast.u = u;
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_load1_ps(&cast.f);
}

/* Scatter */
template <uint32_t vectorNum>
INLINE void SCATTER(const simd_dw<vectorNum> &offset,
                    const simd_dw<vectorNum> &value,
                    char *base_address) {
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const int v0 = _mm_extract_epi32(PS2SI(value.m[i]), 0);
    const int v1 = _mm_extract_epi32(PS2SI(value.m[i]), 1);
    const int v2 = _mm_extract_epi32(PS2SI(value.m[i]), 2);
    const int v3 = _mm_extract_epi32(PS2SI(value.m[i]), 3);
    const int o0 = _mm_extract_epi32(PS2SI(offset.m[i]), 0);
    const int o1 = _mm_extract_epi32(PS2SI(offset.m[i]), 1);
    const int o2 = _mm_extract_epi32(PS2SI(offset.m[i]), 2);
    const int o3 = _mm_extract_epi32(PS2SI(offset.m[i]), 3);
    *(int*)(base_address + o0) = v0;
    *(int*)(base_address + o1) = v1;
    *(int*)(base_address + o2) = v2;
    *(int*)(base_address + o3) = v3;
  }
}
template <uint32_t vectorNum>
INLINE void SCATTER(const simd_dw<vectorNum> &offset,
                    const scalar_dw &value,
                    char *base_address) {
  SCATTER(offset, simd_dw<vectorNum>(value), base_address);
}
template <uint32_t vectorNum>
INLINE void SCATTER(const scalar_dw &offset,
                    const simd_dw<vectorNum> &value,
                    char *base_address) {
  SCATTER(simd_dw<vectorNum>(offset), value, base_address);
}

/* Masked scatter will only store unmasked lanes */
template <uint32_t vectorNum>
INLINE void MASKED_SCATTER(const simd_dw<vectorNum> &offset,
                           const simd_dw<vectorNum> &value,
                           char *base_address,
                           uint32_t mask)
{
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const int v0 = _mm_extract_epi32(PS2SI(value.m[i]), 0);
    const int v1 = _mm_extract_epi32(PS2SI(value.m[i]), 1);
    const int v2 = _mm_extract_epi32(PS2SI(value.m[i]), 2);
    const int v3 = _mm_extract_epi32(PS2SI(value.m[i]), 3);
    const int o0 = _mm_extract_epi32(PS2SI(offset.m[i]), 0);
    const int o1 = _mm_extract_epi32(PS2SI(offset.m[i]), 1);
    const int o2 = _mm_extract_epi32(PS2SI(offset.m[i]), 2);
    const int o3 = _mm_extract_epi32(PS2SI(offset.m[i]), 3);
    if (mask & 1) *(int*)(base_address + o0) = v0;
    if (mask & 2) *(int*)(base_address + o1) = v1;
    if (mask & 4) *(int*)(base_address + o2) = v2;
    if (mask & 8) *(int*)(base_address + o3) = v3;
    mask = mask >> 4;
  }
}
template <uint32_t vectorNum>
INLINE void MASKED_SCATTER(const simd_dw<vectorNum> &offset,
                           const scalar_dw &value,
                           char *base_address,
                           uint32_t mask)
{
  MASKED_SCATTER(offset, simd_dw<vectorNum>(value), base_address, mask);
}
template <uint32_t vectorNum>
INLINE void MASKED_SCATTER(const scalar_dw &offset,
                           const simd_dw<vectorNum> &value,
                           char *base_address,
                           uint32_t mask)
{
  MASKED_SCATTER(simd_dw<vectorNum>(offset), value, base_address, mask);
}

/* Gather */
template <uint32_t vectorNum>
INLINE void GATHER(simd_dw<vectorNum> &dst,
                   const simd_dw<vectorNum> &offset,
                   const char *base_address) {
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const int o0 = _mm_extract_epi32(PS2SI(offset.m[i]) , 0);
    const int o1 = _mm_extract_epi32(PS2SI(offset.m[i]), 1);
    const int o2 = _mm_extract_epi32(PS2SI(offset.m[i]), 2);
    const int o3 = _mm_extract_epi32(PS2SI(offset.m[i]), 3);
    const int v0 = *(const int*)(base_address + o0);
    const int v1 = *(const int*)(base_address + o1);
    const int v2 = *(const int*)(base_address + o2);
    const int v3 = *(const int*)(base_address + o3);
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v0, 0));
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v1, 1));
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v2, 2));
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v3, 3));
  }
}
template <uint32_t vectorNum>
INLINE void GATHER(simd_dw<vectorNum> &dst,
                   const scalar_dw &offset,
                   const char *base_address) {
  GATHER(dst, simd_dw<vectorNum>(offset), base_address);
}

/* Masked gather will only load activated lanes */
template <uint32_t vectorNum>
INLINE void MASKED_GATHER(simd_dw<vectorNum> &dst,
                          const simd_dw<vectorNum> &offset,
                          const char *base_address,
                          uint32_t mask)
{
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const int o0 = _mm_extract_epi32(PS2SI(offset.m[i]) , 0);
    const int o1 = _mm_extract_epi32(PS2SI(offset.m[i]), 1);
    const int o2 = _mm_extract_epi32(PS2SI(offset.m[i]), 2);
    const int o3 = _mm_extract_epi32(PS2SI(offset.m[i]), 3);
    const int v0 = *(const int*)(base_address + o0);
    const int v1 = *(const int*)(base_address + o1);
    const int v2 = *(const int*)(base_address + o2);
    const int v3 = *(const int*)(base_address + o3);
    if (mask & 1) dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v0, 0));
    if (mask & 2) dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v1, 1));
    if (mask & 4) dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v2, 2));
    if (mask & 8) dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v3, 3));
    mask = mask >> 4;
  }
}
template <uint32_t vectorNum>
INLINE void MASKED_GATHER(simd_dw<vectorNum> &dst,
                          const scalar_dw &offset,
                          const char *base_address,
                          uint32_t mask)
{
  MASKED_GATHER(dst, simd_dw<vectorNum>(offset), base_address, mask);
}

//////////////////////////////////////////////////////////////////////////////
// Scalar instructions
//////////////////////////////////////////////////////////////////////////////
INLINE uint32_t elemNum(const scalar_dw &x) { return 1; }
INLINE uint32_t elemNum(const scalar_w &x) { return 1; }
INLINE uint32_t elemNum(const scalar_m &x) { return 1; }
INLINE uint32_t mask(const scalar_m &v) { return v.u ? 1 : 0; }

// 32 bit floating points
INLINE void ADD_F(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.f = v0.f + v1.f; }
INLINE void SUB_F(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.f = v0.f - v1.f; }
INLINE void MUL_F(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.f = v0.f * v1.f; }
INLINE void DIV_F(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.f = v0.f / v1.f; }
INLINE void EQ_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f == v1.f ? ~0 : 0); }
INLINE void NE_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f != v1.f ? ~0 : 0); }
INLINE void LE_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f <= v1.f ? ~0 : 0); }
INLINE void LT_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f < v1.f ? ~0 : 0); }
INLINE void GE_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f >= v1.f ? ~0 : 0); }
INLINE void GT_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f > v1.f ? ~0 : 0); }

// 32 bit integers
INLINE void ADD_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s + v1.s; }
INLINE void SUB_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s - v1.s; }
INLINE void MUL_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s * v1.s; }
INLINE void DIV_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s / v1.s; }
INLINE void REM_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s % v1.s; }
INLINE void MUL_U32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.u = v0.u * v1.u; }
INLINE void DIV_U32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.u = v0.u / v1.u; }
INLINE void REM_U32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.u = v0.u % v1.u; }
INLINE void EQ_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s == v1.s ? ~0 : 0); }
INLINE void NE_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s != v1.s ? ~0 : 0); }
INLINE void LE_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s <= v1.s ? ~0 : 0); }
INLINE void LT_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s < v1.s ? ~0 : 0); }
INLINE void GE_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s >= v1.s ? ~0 : 0); }
INLINE void GT_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s > v1.s ? ~0 : 0); }
INLINE void XOR_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s ^ v1.s; }
INLINE void OR_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s | v1.s; }
INLINE void AND_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s & v1.s; }
INLINE void LE_U32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.u <= v1.u ? ~0 : 0); }
INLINE void LT_U32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.u < v1.u ? ~0 : 0); }
INLINE void GE_U32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.u >= v1.u ? ~0 : 0); }
INLINE void GT_U32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.u > v1.u ? ~0 : 0); }
INLINE void LOAD(scalar_dw &dst, const char *ptr) { dst.u = *(const uint32_t *) ptr; }
INLINE void STORE(scalar_dw src, char *ptr) { *(uint32_t *) ptr = src.u; }
INLINE void LOADI(scalar_dw &dst, uint32_t u) { dst.u = u; }
INLINE void SCATTER(scalar_dw offset, scalar_dw value, char *base) { *(uint32_t*)(base + offset.u) = value.u; }
INLINE void GATHER(scalar_dw &dst, scalar_dw offset, const char *base) { dst.u = *(const uint32_t*)(base + offset.u); }

// 16 bit floating points
INLINE void ADD_U16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.u = v0.u + v1.u; }
INLINE void SUB_U16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.u = v0.u - v1.u; }
INLINE void ADD_S16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.s = v0.s + v1.s; }
INLINE void SUB_S16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.s = v0.s - v1.s; }
INLINE void MUL_S16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.s = v0.s * v1.s; }
INLINE void DIV_S16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.s = v0.s / v1.s; }
INLINE void REM_S16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.s = v0.s % v1.s; }
INLINE void MUL_U16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.u = v0.u * v1.u; }
INLINE void DIV_U16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.u = v0.u / v1.u; }
INLINE void REM_U16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.u = v0.u % v1.u; }
INLINE void EQ_S16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.s == v1.s ? ~0 : 0); }
INLINE void NE_S16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.s != v1.s ? ~0 : 0); }
INLINE void LE_S16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.s <= v1.s ? ~0 : 0); }
INLINE void LT_S16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.s < v1.s ? ~0 : 0); }
INLINE void GE_S16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.s >= v1.s ? ~0 : 0); }
INLINE void GT_S16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.s > v1.s ? ~0 : 0); }
INLINE void XOR_S16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.s = v0.s ^ v1.s; }
INLINE void OR_S16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.s = v0.s | v1.s; }
INLINE void AND_S16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.s = v0.s & v1.s; }
INLINE void LE_U16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.u <= v1.u ? ~0 : 0); }
INLINE void LT_U16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.u < v1.u ? ~0 : 0); }
INLINE void GE_U16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.u >= v1.u ? ~0 : 0); }
INLINE void GT_U16(scalar_m &dst, scalar_w v0, scalar_w v1) { dst.u = (v0.u > v1.u ? ~0 : 0); }
INLINE void LOAD(scalar_w &dst, const char *ptr) { dst.u = *(const uint16_t *) ptr; }
INLINE void STORE(scalar_w src, char *ptr) { *(uint16_t *) ptr = src.u; }
INLINE void LOADI(scalar_w &dst, uint16_t u) { dst.u = u; }
INLINE void SCATTER(scalar_w offset, scalar_w value, char *base) { *(uint16_t*)(base + offset.u) = value.u; }
INLINE void GATHER(scalar_w &dst, scalar_w offset, const char *base) { dst.u = *(const uint16_t*)(base + offset.u); }

//////////////////////////////////////////////////////////////////////////////
// Identical instructions are forwarded
//////////////////////////////////////////////////////////////////////////////

// Forward identical 32 bit instructions
#define NOV_U32 MOV_S32
#define NOV_F MOV_S32
#define ADD_U32 ADD_S32
#define SUB_U32 SUB_S32
#define XOR_U32 XOR_S32
#define OR_U32 OR_S32
#define AND_U32 AND_S32
#define EQ_U32 EQ_S32
#define NE_U32 NE_S32

// Forward identical 16 bit instructions
#define NOV_U16 MOV_S16
#define ADD_U16 ADD_S16
#define SUB_U16 SUB_S16
#define AND_U16 AND_S16
#define XOR_U16 XOR_S16
#define OR_U16 OR_S16
#define AND_U16 AND_S16
#define EQ_U16 EQ_S16
#define NE_U16 NE_S16

#undef PS2SI
#undef SI2PS
#undef ID

//////////////////////////////////////////////////////////////////////////////
// Goto implementation which is directly inspired by BDW goto and by this
// article "Whole function vectorization" (CGO 2011)
//////////////////////////////////////////////////////////////////////////////

/*! Update the UIP vector according for the lanes alive in mask */
template <uint32_t vectorNum>
void updateUIP(simd_w<vectorNum> &uipVec, const simd_m<vectorNum> mask, uint16_t uip) {
  union { float f; uint32_t u; } x;
  x.u = uip;
  __m128 v = _mm_load1_ps(&x.f);
  for (uint32_t i = 0; i < vectorNum; ++i)
    uipVec.m[i] = _mm_blendv_ps(uipVec.m[i], v, mask.m[i]);
}

/*! Update the UIP vector according for the lanes alive in mask */
template <uint32_t vectorNum>
void updateUIPC(simd_w<vectorNum> &uipVec,
                const simd_m<vectorNum> mask,
                const simd_m<vectorNum> cond,
                uint16_t uip) {
  union { float f; uint32_t u; } x;
  x.u = uip;
  __m128 v = _mm_load1_ps(&x.f);
  for (uint32_t i = 0; i < vectorNum; ++i)
    uipVec.m[i] = _mm_blendv_ps(uipVec.m[i], v, _mm_and_ps(cond.m[i], mask.m[i]));
}

/*! Update the execution mask based on block IP and UIP values */
template <uint32_t vectorNum>
void updateMask(simd_m<vectorNum> &mask, const simd_w<vectorNum> &uipVec, uint16_t ip) {
  const simd_w<vectorNum> ipv(ip);
  LE_U16(mask, uipVec, ipv);
}

/*! Jump to the block JIP */
#define SIM_FWD_BRA(UIPVEC, EMASK, JIP, UIP) \
  do { \
    updateUIP(UIPVEC, EMASK, UIP); \
    goto label##JIP; \
  } while (0)

/*! Based on the condition jump to block JIP */
#define SIM_FWD_BRA_C(UIPVEC, EMASK, COND, JIP, UIP) \
  do { \
    updateUIPC(UIPVEC, EMASK, COND, UIP); \
    typeof(COND) jumpCond; \
    scalar_w jipScalar(uint16_t(JIP)); \
    LT_U16(jumpCond, UIPVEC, uint16_t(JIP)); \
    uint32_t jumpMask = mask(jumpCond); \
    if (!jumpMask) goto label##JIP; \
  } while (0)

/*! Backward jump is always taken */
#define SIM_BWD_BRA(UIPVEC, EMASK, JIP) \
  do { \
    updateUIP(UIPVEC, EMASK, JIP); \
    goto label##JIP; \
  } while (0)

/*! Conditional backward jump is taken if the condition is non-null */
#define SIM_BWD_BRA_C(UIPVEC, EMASK, COND, JIP) \
  do { \
    updateUIPC(UIPVEC, EMASK, COND, JIP); \
    typeof(COND) JUMP_MASK; \
    AND_M(JUMP_MASK, COND, EMASK); \
    if (mask(JUMP_MASK) != 0) goto label##JIP; \
  } while (0)

/*! JOIN: reactivates lanes */
#define SIM_JOIN(UIPVEC, MASK, IP) \
  do { \
    updateMask(MASK, UIPVEC, IP); \
    movedMask = mask(MASK); \
  } while (0)

/*! JOIN_JUMP: ractivate lanes and jump to JIP if none is activated */
#define SIM_JOIN_JUMP(UIPVEC, EMASK, IP, JIP) \
  do { \
    SIM_JOIN(UIPVEC, EMASK, IP); \
    const uint32_t execMask = mask(EMASK); \
    if (execMask == 0) goto label##JIP; \
  } while (0)

/* Macro to apply masking on destinations (from zero to four destinations) */
#define MASKED0(OP, ...) \
  do { \
    OP(__VA_ARGS__); \
  } while (0)

#define MASKED1(OP, ARG0, ...) \
  do { \
    typeof(ARG0) ARG0##__; \
    OP(ARG0##__, __VA_ARGS__); \
    select(ARG0, ARG0, ARG0##__, emask); \
  } while (0)

#define MASKED2(OP, ARG0, ARG1, ...) \
  do { \
    typeof(ARG0) ARG0##__; \
    typeof(ARG1) ARG1##__; \
    OP(ARG0##__, ARG1##__, __VA_ARGS__); \
    select(ARG0, ARG0, ARG0##__, emask); \
    select(ARG1, ARG1, ARG1##__, emask); \
  } while (0)

#define MASKED3(OP, ARG0, ARG1, ARG2, ...) \
  do { \
    typeof(ARG0) ARG0##__; \
    typeof(ARG1) ARG1##__; \
    typeof(ARG2) ARG2##__; \
    OP(ARG0##__, ARG1##__, ARG2##__, __VA_ARGS__); \
    select(ARG0, ARG0, ARG0##__, emask); \
    select(ARG1, ARG1, ARG1##__, emask); \
    select(ARG2, ARG2, ARG2##__, emask); \
  } while (0)

#define MASKED4(OP, ARG0, ARG1, ARG2, ARG3, ...) \
  do { \
    typeof(ARG0) ARG0##__; \
    typeof(ARG1) ARG1##__; \
    typeof(ARG2) ARG2##__; \
    typeof(ARG3) ARG3##__; \
    OP(ARG0##__, ARG1##__, ARG2##__, ARG3##__, __VA_ARGS__); \
    select(ARG0, ARG0, ARG0##__, emask); \
    select(ARG1, ARG1, ARG1##__, emask); \
    select(ARG2, ARG2, ARG2##__, emask); \
    select(ARG3, ARG3, ARG3##__, emask); \
  } while (0)

#undef INLINE

#endif /* __GBE_SIM_VECTOR_H__ */

