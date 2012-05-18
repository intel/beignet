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
  uint32_t u;
  int32_t s;
  float f;
  char data[4];
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
  uint16_t u;
  int16_t s;
  char data[2];
};

/*! Base structure for scalar byte (8 bits) */
union scalar_b {
  INLINE scalar_b(void) {}
  INLINE scalar_b(uint8_t u) { this->u = u; }
  INLINE scalar_b(int8_t s) { this->s = s; }
  INLINE float toFloat(void) const {
    union {uint8_t u[4]; float f;} x;
    x.u[0] = u;
    x.u[1] = 0;
    x.u[2] = 0;
    x.u[3] = 0;
    return x.f;
  }
  uint8_t u;
  int8_t s;
  char data[1];
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

/*! Base structure for vectors 4 / 8 / 16 / 32 bytes. We do not store 16 bytes
 *  but only 4. This makes everything much simpler even if it is clearly slower
 */
template <uint32_t vectorNum>
struct simd_b {
  INLINE simd_b(void) {}
  INLINE simd_b(const scalar_b &s) {
    const float f = s.toFloat();
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&f);
  }
  simd_b &operator= (const scalar_b &s) {
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
template <uint32_t vectorNum, template <uint32_t> class T>
INLINE void select(T<vectorNum> &dst,
                   const T<vectorNum> &src0,
                   const T<vectorNum> &src1,
                   const simd_m<vectorNum> &mask)
{
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_blendv_ps(src0.m[i], src1.m[i], mask.m[i]);
}

/*! To cast 32 bits values in sse registers through memory */
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
  char data[16];
  uint32_t u[4];
  int32_t s[4];
  float f[4];
};
static const cast_dw allTrue(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

/*! To cast 16 bits values in sse registers through memory */
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
  char data[16];
  struct { uint16_t v; uint16_t pad; } u[4];
  struct {  int16_t v;  int16_t pad; } s[4];
};

/*! To cast 8 bits values in sse registers through memory */
union cast_b {
  INLINE cast_b(int8_t s0, int8_t s1, int8_t s2, int8_t s3) {
    s[0].v = s0; s[1].v = s1; s[2].v = s2; s[3].v = s3;
    for (uint32_t i = 0; i < 3; ++i)
      s[0].pad[i] = s[1].pad[i] = s[2].pad[i] = s[3].pad[i] = 0;
  }
  INLINE cast_b(uint8_t u0, uint8_t u1, uint8_t u2, uint8_t u3) {
    u[0].v = u0; u[1].v = u1; u[2].v = u2; u[3].v = u3;
    for (uint32_t i = 0; i < 3; ++i)
      s[0].pad[i] = s[1].pad[i] = s[2].pad[i] = s[3].pad[i] = 0;
  }
  INLINE cast_b(const __m128 &v) : v(v) {}
  INLINE cast_b(const __m128i &vi) : vi(vi) {}
  INLINE cast_b(void) {}
  __m128 v;
  __m128i vi;
  char data[16];
  struct { uint8_t v; uint8_t pad[3]; } u[4];
  struct {  int8_t v;  int8_t pad[3]; } s[4];
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
typedef scalar_b   simd1b;
typedef simd_b<1>  simd4b;
typedef simd_b<2>  simd8b;
typedef simd_b<4>  simd16b;
typedef simd_b<8>  simd32b;
typedef scalar_m   simd1m;
typedef simd_m<1>  simd4m;
typedef simd_m<2>  simd8m;
typedef simd_m<4>  simd16m;
typedef simd_m<8>  simd32m;

/* Meta-programming crap to get the vector and the scalar types from the regular
 * base C types
 */
template <uint32_t vectorNum, typename T> struct SimTypeTrait {};

#define DECL_TYPE_TRAIT(CTYPE, VECTOR_TYPE, SCALAR_TYPE, CAST_TYPE) \
template <uint32_t vectorNum> \
struct SimTypeTrait<vectorNum, CTYPE> { \
  typedef VECTOR_TYPE Vector; \
  typedef SCALAR_TYPE Scalar; \
  typedef CAST_TYPE Cast; \
};
DECL_TYPE_TRAIT(uint8_t, simd_b<vectorNum>, scalar_b, cast_b)
DECL_TYPE_TRAIT(int8_t, simd_b<vectorNum>, scalar_b, cast_b)
DECL_TYPE_TRAIT(uint16_t, simd_w<vectorNum>, scalar_w, cast_w)
DECL_TYPE_TRAIT(int16_t, simd_w<vectorNum>, scalar_w, cast_w)
DECL_TYPE_TRAIT(uint32_t, simd_dw<vectorNum>, scalar_dw, cast_dw)
DECL_TYPE_TRAIT(int32_t, simd_dw<vectorNum>, scalar_dw, cast_dw)
DECL_TYPE_TRAIT(float, simd_dw<vectorNum>, scalar_dw, cast_dw)
#undef DECL_TYPE_TRAIT

//////////////////////////////////////////////////////////////////////////////
// Vector instructions
//////////////////////////////////////////////////////////////////////////////
/* Simple function to get the number of element per vector */
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_dw<vectorNum> &x) {
  return 4*vectorNum;
}
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_w<vectorNum> &x) {
  return 4*vectorNum;
}
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_b<vectorNum> &x) {
  return 4*vectorNum;
}
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_m<vectorNum> &x) {
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
template <uint32_t vectorNum>
INLINE void MOV_S8(simd_b<vectorNum> &dst, const simd_b<vectorNum> &v) {
  for (uint32_t i = 0; i < vectorNum; ++i) dst.m[i] = v.m[i];
}
template <uint32_t vectorNum>
INLINE void MOV_S8(simd_b<vectorNum> &dst, const scalar_b &x) {
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
VEC_OP(simd_m<vectorNum>,  simd_b<vectorNum>,  scalar_b,  EQ_S8, _mm_cmpeq_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b, ADD_S8, _mm_add_epi8, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b, SUB_S8, _mm_sub_epi8, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b, AND_S8, _mm_and_ps, ID, ID, ID);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b, OR_S8, _mm_or_ps, ID, ID, ID);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b, XOR_S8, _mm_xor_ps, ID, ID, ID);
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
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, SHL_U32, <<, s[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, MUL_S32, *, s[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, DIV_S32, /, s[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, REM_S32, %, s[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, MUL_U32, *, u[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, DIV_U32, /, u[j]);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, scalar_dw, cast_dw, REM_U32, %, u[j]);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  SHL_U16, <<, s[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  MUL_S16, *, s[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  DIV_S16, /, s[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  REM_S16, %, s[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  MUL_U16, *, u[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  DIV_U16, /, u[j].v);
VEC_OP(simd_w<vectorNum>,  simd_w<vectorNum>,  scalar_w,  cast_w,  REM_U16, %, u[j].v);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b,  cast_b,  SHL_U8, <<, s[j].v);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b,  cast_b,  MUL_S8, *, s[j].v);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b,  cast_b,  DIV_S8, /, s[j].v);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b,  cast_b,  REM_S8, %, s[j].v);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b,  cast_b,  MUL_U8, *, u[j].v);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b,  cast_b,  DIV_U8, /, u[j].v);
VEC_OP(simd_b<vectorNum>,  simd_b<vectorNum>,  scalar_b,  cast_b,  REM_U8, %, u[j].v);
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
VEC_OP(simd_m<vectorNum>, simd_b<vectorNum>,  scalar_b,  cast_b,  LE_U8, <=, u[j].v);
VEC_OP(simd_m<vectorNum>, simd_b<vectorNum>,  scalar_b,  cast_b,  LT_U8, <, u[j].v);
VEC_OP(simd_m<vectorNum>, simd_b<vectorNum>,  scalar_b,  cast_b,  GE_U8, >=, u[j].v);
VEC_OP(simd_m<vectorNum>, simd_b<vectorNum>,  scalar_b,  cast_b,  GT_U8, >, u[j].v);
VEC_OP(simd_m<vectorNum>, simd_b<vectorNum>,  scalar_b,  cast_b,  LE_S8, <=, s[j].v);
VEC_OP(simd_m<vectorNum>, simd_b<vectorNum>,  scalar_b,  cast_b,  LT_S8, <, s[j].v);
VEC_OP(simd_m<vectorNum>, simd_b<vectorNum>,  scalar_b,  cast_b,  GE_S8, >=, s[j].v);
VEC_OP(simd_m<vectorNum>, simd_b<vectorNum>,  scalar_b,  cast_b,  GT_S8, >, s[j].v);
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
template <uint32_t vectorNum>
INLINE void NE_S8(simd_m<vectorNum> &dst,
                  const simd_b<vectorNum> &v0,
                  const simd_b<vectorNum> &v1)
{
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_xor_ps(allTrue.v, SI2PS(_mm_cmpeq_epi32(PS2SI(v0.m[i]), PS2SI(v1.m[i]))));
}
template <uint32_t vectorNum>
INLINE void NE_S8(simd_m<vectorNum> &dst,
                  const simd_b<vectorNum> &v0,
                  const scalar_b &v1)
{
  NE_S8(dst, v0, simd_b<vectorNum>(v1));
}
template <uint32_t vectorNum>
INLINE void NE_S8(simd_m<vectorNum> &dst,
                  const scalar_b &v0,
                  const simd_b<vectorNum> &v1)
{
  NE_S8(dst, simd_b<vectorNum>(v0), v1);
}
template <uint32_t vectorNum>
INLINE void NE_S8(simd_m<vectorNum> &dst,
                  const scalar_b &v0,
                  const scalar_b &v1)
{
  NE_S8(dst, simd_b<vectorNum>(v0), simd_b<vectorNum>(v1));
}


template <typename DstCType,
          typename SrcCType,
          uint32_t vectorNum,
          template <uint32_t> class DstType,
          template <uint32_t> class SrcType>
INLINE void CVT(DstType<vectorNum> &dst, const SrcType<vectorNum> &src)
{
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const typename SimTypeTrait<vectorNum, SrcCType>::Cast srcCast(src.m[i]);
    const DstCType x0 = (DstCType) *(const SrcCType*) (srcCast.data + 0);
    const DstCType x1 = (DstCType) *(const SrcCType*) (srcCast.data + 4);
    const DstCType x2 = (DstCType) *(const SrcCType*) (srcCast.data + 8);
    const DstCType x3 = (DstCType) *(const SrcCType*) (srcCast.data + 12);
    const typename SimTypeTrait<vectorNum, DstCType>::Cast dstCast(x0, x1, x2, x3);
    dst.m[i] = dstCast.v;
  }
}

template <typename DstCType,
          typename SrcCType,
          uint32_t vectorNum,
          template <uint32_t> class DstType,
          class SrcType>
INLINE void CVT(DstType<vectorNum> &dst, const SrcType &src)
{
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const SrcCType from = *((SrcCType *) src.data);
    const DstCType x = (DstCType) from;
    const typename SimTypeTrait<vectorNum, DstCType>::Cast dstCast(x,x,x,x);
    dst.m[i] = dstCast.v;
  }
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

/* Load from contiguous bytes */
template <uint32_t vectorNum>
INLINE void LOAD(simd_b<vectorNum> &dst, const char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const uint8_t u0 = *((uint8_t*) ptr + 4*i + 0);
    const uint8_t u1 = *((uint8_t*) ptr + 4*i + 1);
    const uint8_t u2 = *((uint8_t*) ptr + 4*i + 2);
    const uint8_t u3 = *((uint8_t*) ptr + 4*i + 3);
    const cast_b w(u0,u1,u2,u3);
    dst.m[i] = w.v;
  }
}

/* Store to contiguous bytes */
template <uint32_t vectorNum>
INLINE void STORE(const simd_b<vectorNum> &src, char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const cast_b w(src.m[i]);
    *((uint8_t*) ptr + 4*i + 0) = w.u[0].v;
    *((uint8_t*) ptr + 4*i + 1) = w.u[1].v;
    *((uint8_t*) ptr + 4*i + 2) = w.u[2].v;
    *((uint8_t*) ptr + 4*i + 3) = w.u[3].v;
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
template <uint32_t vectorNum>
INLINE void LOADI(simd_w<vectorNum> &dst, uint16_t u) {
  union { uint32_t u; float f; } cast;
  cast.u = u;
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_load1_ps(&cast.f);
}
template <uint32_t vectorNum>
INLINE void LOADI(simd_b<vectorNum> &dst, uint8_t u) {
  union { uint32_t u; float f; } cast;
  cast.u = u;
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_load1_ps(&cast.f);
}

/* Scatter for bytes, shorts and integers */
#define DECL_SCATTER(VECTOR_TYPE, SCALAR_TYPE, CTYPE, MASK) \
template <uint32_t vectorNum> \
INLINE void SCATTER(const simd_dw<vectorNum> &address, \
                    const VECTOR_TYPE<vectorNum> &value, \
                    char *base_address, \
                    uint32_t offset = 0) \
{ \
  for (uint32_t i = 0; i < vectorNum; ++i) { \
    const uint32_t v0 = _mm_extract_epi32(PS2SI(value.m[i]), 0) & MASK; \
    const uint32_t v1 = _mm_extract_epi32(PS2SI(value.m[i]), 1) & MASK; \
    const uint32_t v2 = _mm_extract_epi32(PS2SI(value.m[i]), 2) & MASK; \
    const uint32_t v3 = _mm_extract_epi32(PS2SI(value.m[i]), 3) & MASK; \
    const uint32_t o0 = _mm_extract_epi32(PS2SI(address.m[i]), 0) + offset; \
    const uint32_t o1 = _mm_extract_epi32(PS2SI(address.m[i]), 1) + offset; \
    const uint32_t o2 = _mm_extract_epi32(PS2SI(address.m[i]), 2) + offset; \
    const uint32_t o3 = _mm_extract_epi32(PS2SI(address.m[i]), 3) + offset; \
    *(CTYPE *)(base_address + o0) = v0; \
    *(CTYPE *)(base_address + o1) = v1; \
    *(CTYPE *)(base_address + o2) = v2; \
    *(CTYPE *)(base_address + o3) = v3; \
  } \
} \
template <uint32_t vectorNum> \
INLINE void SCATTER(const simd_dw<vectorNum> &address, \
                    const SCALAR_TYPE &value, \
                    char *base_address, \
                    uint32_t offset = 0) \
{ \
  SCATTER(address, VECTOR_TYPE<vectorNum>(value), base_address, offset); \
} \
template <uint32_t vectorNum> \
INLINE void SCATTER(const scalar_dw &address, \
                    const VECTOR_TYPE<vectorNum> &value, \
                    char *base_address, \
                    uint32_t offset = 0) \
{ \
  SCATTER(simd_dw<vectorNum>(address), value, base_address, offset); \
}
DECL_SCATTER(simd_dw, scalar_dw, uint32_t, 0xffffffff)
DECL_SCATTER(simd_w,  scalar_w,  uint16_t, 0xffff)
DECL_SCATTER(simd_b,  scalar_b,  uint8_t,  0xff)
#undef DECL_SCATTER

template <typename T, typename U, typename V, uint32_t vectorNum>
INLINE void SCATTER2(const T &address,
                     const U &value0,
                     const V &value1,
                     char *base_address)
{
  SCATTER(address, value0, base_address, 0);
  SCATTER(address, value1, base_address, 4);
}
template <typename T, typename U, typename V, typename W, uint32_t vectorNum>
INLINE void SCATTER3(const T &address,
                     const U &value0,
                     const V &value1,
                     const W &value2,
                     char *base_address)
{
  SCATTER(address, value0, base_address, 0);
  SCATTER(address, value1, base_address, 4);
  SCATTER(address, value2, base_address, 8);
}
template <typename T, typename U, typename V, typename W, typename X, uint32_t vectorNum>
INLINE void SCATTER4(const T &address,
                     const U &value0,
                     const V &value1,
                     const W &value2,
                     const X &value3,
                     char *base_address)
{
  SCATTER(address, value0, base_address, 0);
  SCATTER(address, value1, base_address, 4);
  SCATTER(address, value2, base_address, 8);
  SCATTER(address, value3, base_address, 12);
}

/* Masked scatter will only store unmasked lanes */
#define DECL_MASKED_SCATTER(VECTOR_TYPE, SCALAR_TYPE, CTYPE, MASK) \
template <uint32_t vectorNum> \
INLINE void MASKED_SCATTER(const simd_dw<vectorNum> &address, \
                           const VECTOR_TYPE<vectorNum> &value, \
                           char *base_address, \
                           uint32_t mask, \
                           uint32_t offset = 0) \
{ \
  for (uint32_t i = 0; i < vectorNum; ++i) { \
    const uint32_t v0 = _mm_extract_epi32(PS2SI(value.m[i]), 0) & MASK; \
    const uint32_t v1 = _mm_extract_epi32(PS2SI(value.m[i]), 1) & MASK; \
    const uint32_t v2 = _mm_extract_epi32(PS2SI(value.m[i]), 2) & MASK; \
    const uint32_t v3 = _mm_extract_epi32(PS2SI(value.m[i]), 3) & MASK; \
    const uint32_t o0 = _mm_extract_epi32(PS2SI(address.m[i]), 0) + offset; \
    const uint32_t o1 = _mm_extract_epi32(PS2SI(address.m[i]), 1) + offset; \
    const uint32_t o2 = _mm_extract_epi32(PS2SI(address.m[i]), 2) + offset; \
    const uint32_t o3 = _mm_extract_epi32(PS2SI(address.m[i]), 3) + offset; \
    if (mask & 1) *(CTYPE *)(base_address + o0) = v0; \
    if (mask & 2) *(CTYPE *)(base_address + o1) = v1; \
    if (mask & 4) *(CTYPE *)(base_address + o2) = v2; \
    if (mask & 8) *(CTYPE *)(base_address + o3) = v3; \
    mask = mask >> 4; \
  } \
} \
template <uint32_t vectorNum> \
INLINE void MASKED_SCATTER(const simd_dw<vectorNum> &address, \
                           const SCALAR_TYPE &value, \
                           char *base_address, \
                           uint32_t mask, \
                           uint32_t offset = 0) \
{ \
  MASKED_SCATTER(address, VECTOR_TYPE<vectorNum>(value), base_address, mask, offset); \
} \
template <uint32_t vectorNum> \
INLINE void MASKED_SCATTER(const scalar_dw &address, \
                           const VECTOR_TYPE<vectorNum> &value, \
                           char *base_address, \
                           uint32_t mask, \
                           uint32_t offset = 0) \
{ \
  MASKED_SCATTER(simd_dw<vectorNum>(address), value, base_address, mask, offset); \
}
DECL_MASKED_SCATTER(simd_dw, scalar_dw, uint32_t, 0xffffffff)
DECL_MASKED_SCATTER(simd_w,  scalar_w,  uint16_t, 0xffff)
DECL_MASKED_SCATTER(simd_b,  scalar_b,  uint8_t,  0xff)
#undef DECL_MASKED_SCATTER

template <typename T, typename U, typename V>
INLINE void MASKED_SCATTER2(const T &address,
                            const U &value0,
                            const V &value1,
                            char *base_address,
                            uint32_t mask)
{
  MASKED_SCATTER(address, value0, base_address, mask, 0);
  MASKED_SCATTER(address, value1, base_address, mask, 4);
}
template <typename T, typename U, typename V, typename W>
INLINE void MASKED_SCATTER3(const T &address,
                            const U &value0,
                            const V &value1,
                            const W &value2,
                            char *base_address,
                            uint32_t mask)
{
  MASKED_SCATTER(address, value0, base_address, mask, 0);
  MASKED_SCATTER(address, value1, base_address, mask, 4);
  MASKED_SCATTER(address, value2, base_address, mask, 8);
}
template <typename T, typename U, typename V, typename W, typename X>
INLINE void MASKED_SCATTER4(const T &address,
                            const U &value0,
                            const V &value1,
                            const W &value2,
                            const X &value3,
                            char *base_address,
                            uint32_t mask)
{
  MASKED_SCATTER(address, value0, base_address, mask, 0);
  MASKED_SCATTER(address, value1, base_address, mask, 4);
  MASKED_SCATTER(address, value2, base_address, mask, 8);
  MASKED_SCATTER(address, value3, base_address, mask, 12);
}

/* Gather */
#define DECL_GATHER(VECTOR_TYPE, SCALAR_TYPE, CTYPE) \
template <uint32_t vectorNum> \
INLINE void GATHER(VECTOR_TYPE<vectorNum> &dst, \
                   const simd_dw<vectorNum> &address, \
                   const char *base_address, \
                   uint32_t offset = 0) \
{ \
  for (uint32_t i = 0; i < vectorNum; ++i) { \
    const uint32_t o0 = _mm_extract_epi32(PS2SI(address.m[i]), 0) + offset; \
    const uint32_t o1 = _mm_extract_epi32(PS2SI(address.m[i]), 1) + offset; \
    const uint32_t o2 = _mm_extract_epi32(PS2SI(address.m[i]), 2) + offset; \
    const uint32_t o3 = _mm_extract_epi32(PS2SI(address.m[i]), 3) + offset; \
    const CTYPE v0 = *(const CTYPE *)(base_address + o0); \
    const CTYPE v1 = *(const CTYPE *)(base_address + o1); \
    const CTYPE v2 = *(const CTYPE *)(base_address + o2); \
    const CTYPE v3 = *(const CTYPE *)(base_address + o3); \
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v0, 0)); \
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v1, 1)); \
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v2, 2)); \
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v3, 3)); \
  } \
} \
template <uint32_t vectorNum> \
INLINE void GATHER(VECTOR_TYPE<vectorNum> &dst, \
                   const scalar_dw &address, \
                   const char *base_address, \
                   uint32_t offset = 0) \
{ \
  GATHER(dst, VECTOR_TYPE<vectorNum>(address), base_address, offset); \
}
DECL_GATHER(simd_dw, scalar_dw, uint32_t)
DECL_GATHER(simd_w,  scalar_w, uint16_t)
DECL_GATHER(simd_b,  scalar_b, uint8_t)
#undef DECL_GATHER

template <typename T, typename U, typename V>
INLINE void GATHER2(U &value0,
                    V &value1,
                    const T &address,
                    char *base_address)
{
  GATHER(value0, address, base_address, 0);
  GATHER(value1, address, base_address, 4);
}
template <typename T, typename U, typename V, typename W>
INLINE void GATHER3(U &value0,
                    V &value1,
                    W &value2,
                    const T &address,
                    char *base_address)
{
  GATHER(value0, address, base_address, 0);
  GATHER(value1, address, base_address, 4);
  GATHER(value2, address, base_address, 8);
}
template <typename T, typename U, typename V, typename W, typename X>
INLINE void GATHER4(U &value0,
                    V &value1,
                    W &value2,
                    X &value3,
                    const T &address,
                    char *base_address)
{
  GATHER(value0, address, base_address, 0);
  GATHER(value1, address, base_address, 4);
  GATHER(value2, address, base_address, 8);
  GATHER(value3, address, base_address, 12);
}
#include <cstdio>
/* Masked gather will only load activated lanes */
#define DECL_MASKED_GATHER(VECTOR_TYPE, SCALAR_TYPE, CTYPE) \
template <uint32_t vectorNum> \
INLINE void MASKED_GATHER(VECTOR_TYPE<vectorNum> &dst, \
                          const simd_dw<vectorNum> &address, \
                          const char *base_address, \
                          uint32_t mask, \
                          uint32_t offset = 0) \
{ \
  for (uint32_t i = 0; i < vectorNum; ++i) { \
    printf("%i\n", i);\
    const uint32_t o0 = _mm_extract_epi32(PS2SI(address.m[i]), 0) + offset; \
    const uint32_t o1 = _mm_extract_epi32(PS2SI(address.m[i]), 1) + offset; \
    const uint32_t o2 = _mm_extract_epi32(PS2SI(address.m[i]), 2) + offset; \
    const uint32_t o3 = _mm_extract_epi32(PS2SI(address.m[i]), 3) + offset; \
    if (mask & 1) { \
      const CTYPE v0 = *(const CTYPE *)(base_address + o0); \
      dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v0, 0)); \
    } \
    if (mask & 2) { \
      const CTYPE v1 = *(const CTYPE *)(base_address + o1); \
      dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v1, 1)); \
    } \
    if (mask & 4) { \
      const CTYPE v2 = *(const CTYPE *)(base_address + o2); \
      dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v2, 2)); \
    } \
    if (mask & 8) { \
      const CTYPE v3 = *(const CTYPE *)(base_address + o3); \
      dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v3, 3)); \
    } \
    mask = mask >> 4; \
  } \
} \
template <uint32_t vectorNum> \
INLINE void MASKED_GATHER(VECTOR_TYPE<vectorNum> &dst, \
                          const scalar_dw &address, \
                          const char *base_address, \
                          uint32_t mask, \
                          uint32_t offset = 0) \
{ \
  MASKED_GATHER(dst, simd_dw<vectorNum>(address), base_address, mask, offset); \
}
DECL_MASKED_GATHER(simd_dw, scalar_dw, uint32_t)
DECL_MASKED_GATHER(simd_w, scalar_w, uint16_t)
DECL_MASKED_GATHER(simd_b, scalar_b, uint8_t)
#undef DECL_MASKED_GATHER

template <typename T, typename U, typename V>
INLINE void MASKED_GATHER2(U &value0,
                           V &value1,
                           const T &address,
                           char *base_address,
                           uint32_t mask)
{
  MASKED_GATHER(value0, address, base_address, mask, 0);
  MASKED_GATHER(value1, address, base_address, mask, 4);
}
template <typename T, typename U, typename V, typename W>
INLINE void MASKED_GATHER3(U &value0,
                           V &value1,
                           W &value2,
                           const T &address,
                           char *base_address,
                           uint32_t mask)
{
  MASKED_GATHER(value0, address, base_address, mask, 0);
  MASKED_GATHER(value1, address, base_address, mask, 4);
  MASKED_GATHER(value2, address, base_address, mask, 8);
}
template <typename T, typename U, typename V, typename W, typename X>
INLINE void MASKED_GATHER4(U &value0,
                           V &value1,
                           W &value2,
                           X &value3,
                           const T &address,
                           char *base_address,
                           uint32_t mask)
{
  MASKED_GATHER(value0, address, base_address, mask, 0);
  MASKED_GATHER(value1, address, base_address, mask, 4);
  MASKED_GATHER(value2, address, base_address, mask, 8);
  MASKED_GATHER(value3, address, base_address, mask, 12);
}

//////////////////////////////////////////////////////////////////////////////
// Scalar instructions
//////////////////////////////////////////////////////////////////////////////
INLINE uint32_t elemNum(const scalar_dw &x) { return 1; }
INLINE uint32_t elemNum(const scalar_w &x) { return 1; }
INLINE uint32_t elemNum(const scalar_b &x) { return 1; }
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
INLINE void SHL_U32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s << v1.s; }
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

// 16 bits scalar
INLINE void SHL_U16(scalar_w &dst, scalar_w v0, scalar_w v1) { dst.u = v0.u << v1.u; }
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
INLINE void SCATTER(scalar_dw offset, scalar_w value, char *base) { *(uint16_t*)(base + offset.u) = value.u; }
INLINE void GATHER(scalar_w &dst, scalar_dw offset, const char *base) { dst.u = *(const uint16_t*)(base + offset.u); }

// 8 bits scalars
INLINE void SHL_U8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.u = v0.u << v1.u; }
INLINE void ADD_U8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.u = v0.u + v1.u; }
INLINE void SUB_U8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.u = v0.u - v1.u; }
INLINE void ADD_S8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.s = v0.s + v1.s; }
INLINE void SUB_S8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.s = v0.s - v1.s; }
INLINE void MUL_S8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.s = v0.s * v1.s; }
INLINE void DIV_S8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.s = v0.s / v1.s; }
INLINE void REM_S8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.s = v0.s % v1.s; }
INLINE void MUL_U8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.u = v0.u * v1.u; }
INLINE void DIV_U8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.u = v0.u / v1.u; }
INLINE void REM_U8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.u = v0.u % v1.u; }
INLINE void EQ_S8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.s == v1.s ? ~0 : 0); }
INLINE void NE_S8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.s != v1.s ? ~0 : 0); }
INLINE void LE_S8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.s <= v1.s ? ~0 : 0); }
INLINE void LT_S8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.s < v1.s ? ~0 : 0); }
INLINE void GE_S8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.s >= v1.s ? ~0 : 0); }
INLINE void GT_S8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.s > v1.s ? ~0 : 0); }
INLINE void XOR_S8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.s = v0.s ^ v1.s; }
INLINE void OR_S8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.s = v0.s | v1.s; }
INLINE void AND_S8(scalar_b &dst, scalar_b v0, scalar_b v1) { dst.s = v0.s & v1.s; }
INLINE void LE_U8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.u <= v1.u ? ~0 : 0); }
INLINE void LT_U8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.u < v1.u ? ~0 : 0); }
INLINE void GE_U8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.u >= v1.u ? ~0 : 0); }
INLINE void GT_U8(scalar_m &dst, scalar_b v0, scalar_b v1) { dst.u = (v0.u > v1.u ? ~0 : 0); }
INLINE void LOAD(scalar_b &dst, const char *ptr) { dst.u = *(const uint8_t *) ptr; }
INLINE void STORE(scalar_b src, char *ptr) { *(uint8_t *) ptr = src.u; }
INLINE void LOADI(scalar_b &dst, uint8_t u) { dst.u = u; }
INLINE void SCATTER(scalar_dw offset, scalar_b value, char *base) { *(uint8_t*)(base + offset.u) = value.u; }
INLINE void GATHER(scalar_b &dst, scalar_dw offset, const char *base) { dst.u = *(const uint8_t*)(base + offset.u); }

//////////////////////////////////////////////////////////////////////////////
// Identical instructions are forwarded
//////////////////////////////////////////////////////////////////////////////

// Forward identical 32 bit instructions
#define MOV_U32 MOV_S32
#define SHL_S32 SHL_U32
#define MOV_F MOV_S32
#define ADD_U32 ADD_S32
#define SUB_U32 SUB_S32
#define XOR_U32 XOR_S32
#define OR_U32 OR_S32
#define AND_U32 AND_S32
#define EQ_U32 EQ_S32
#define NE_U32 NE_S32

// Forward identical 16 bit instructions
#define MOV_U16 MOV_S16
#define SHL_S16 SHL_U16
#define ADD_U16 ADD_S16
#define SUB_U16 SUB_S16
#define AND_U16 AND_S16
#define XOR_U16 XOR_S16
#define OR_U16 OR_S16
#define AND_U16 AND_S16
#define EQ_U16 EQ_S16
#define NE_U16 NE_S16

// Forward identical 8 bit instructions
#define MOV_U8 MOV_S8
#define SHL_S8 SHL_U8
#define ADD_U8 ADD_S8
#define SUB_U8 SUB_S8
#define AND_U8 AND_S8
#define XOR_U8 XOR_S8
#define OR_U8 OR_S8
#define AND_U8 AND_S8
#define EQ_U8 EQ_S8
#define NE_U8 NE_S8

// More convenient to emit code
#define GATHER1 GATHER
#define SCATTER1 SCATTER
#define MASKED_GATHER1 MASKED_GATHER
#define MASKED_SCATTER1 MASKED_SCATTER

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

// May be needed for some macro hell
#define COMMA ,
#endif /* __GBE_SIM_VECTOR_H__ */

