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

#define ID(X) X
#define VEC_OP(DST_TYPE, SRC_TYPE, NAME, INTRINSIC_NAME, FN, FN0, FN1)      \
template <uint32_t vectorNum>                                               \
INLINE void NAME(DST_TYPE<vectorNum> &dst,                                      \
                 const SRC_TYPE<vectorNum> &v0,                                 \
                 const SRC_TYPE<vectorNum> &v1) {                               \
  for (uint32_t i = 0; i < vectorNum; ++i)                                  \
    dst.m[i] = FN(INTRINSIC_NAME(FN0(v0.m[i]), FN1(v1.m[i])));              \
}                                                                           \
template <uint32_t vectorNum>                                               \
INLINE void NAME(DST_TYPE<vectorNum> &dst,                                      \
                 const SRC_TYPE<vectorNum> &v0,                                 \
                 const SRC_TYPE##1 &v1) {                                       \
  for (uint32_t i = 0; i < vectorNum; ++i)                                  \
    dst.m[i] = FN(INTRINSIC_NAME(FN0(v0.m[i]), FN1(expand<0>(v1.m[0]))));   \
}                                                                           \
template <uint32_t vectorNum>                                               \
INLINE void NAME(DST_TYPE<vectorNum> &dst,                                      \
                 const SRC_TYPE##1 &v0,                                         \
                 const SRC_TYPE<vectorNum> &v1) {                               \
  for (uint32_t i = 0; i < vectorNum; ++i)                                  \
    dst.m[i] = FN(INTRINSIC_NAME(FN0(expand<0>(v0.m[0])), FN1(v1.m[i])));   \
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
VEC_OP(geni, geni, OR, _mm_or_ps, _mm_castps_si128, _mm_castsi128_ps, _mm_castsi128_ps);
VEC_OP(geni, geni, XOR, _mm_xor_ps, _mm_castps_si128, _mm_castsi128_ps, _mm_castsi128_ps);
VEC_OP(geni, geni, AND, _mm_and_ps, _mm_castps_si128, _mm_castsi128_ps, _mm_castsi128_ps);
VEC_OP(genm, genf, SLT, _mm_cmplt_ps, ID, ID, ID);
VEC_OP(genm, genf, SLE, _mm_cmple_ps, ID, ID, ID);
VEC_OP(genm, genf, SGT, _mm_cmpgt_ps, ID, ID, ID);
VEC_OP(genm, genf, SGE, _mm_cmpge_ps, ID, ID, ID);

#undef VEC_OP

#define SCALAR_OP(TYPE, NAME, INTRINSIC_NAME)                  \
INLINE void NAME(TYPE &dst, const TYPE &v0, const TYPE &v1) {  \
  dst.m[0] = INTRINSIC_NAME(v0.m[0], v1.m[0]);                 \
}
SCALAR_OP(genf1, ADD, _mm_add_ss);
SCALAR_OP(genf1, SUB, _mm_sub_ss);
SCALAR_OP(genf1, MUL, _mm_mul_ss);
SCALAR_OP(genf1, DIV, _mm_div_ss);
SCALAR_OP(geni1, ADD, _mm_add_epi32);
SCALAR_OP(geni1, SUB, _mm_sub_epi32);
#undef SCALAR_OP
#undef ID

/* load from contiguous floats / integers */
template <uint32_t vectorNum>
INLINE void LOAD(genf<vectorNum> &dst, const float *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_loadu_ps(ptr + 4*i);
}
INLINE void LOAD(genf1 &dst, const float *ptr) {
  dst.m[0] = _mm_load_ss(ptr);
}
template <uint32_t vectorNum>
INLINE void LOAD(geni<vectorNum> &dst, const float *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_castps_si128(_mm_loadu_ps(ptr + 4*i));
}
INLINE void LOAD(geni1 &dst, const float *ptr) {
  dst.m[0] = _mm_castps_si128(_mm_load_ss(ptr));
}

/* store to contiguous floats / integers */
template <uint32_t vectorNum>
INLINE void STORE(genf<vectorNum> &dst, float *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    _mm_storeu_ps(ptr + 4*i, dst.m[i]);
}
INLINE void STORE(genf1 &dst, float *ptr) {
  _mm_store_ss(ptr, dst.m[0]);
}
template <uint32_t vectorNum>
INLINE void STORE(geni<vectorNum> &dst, float *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
     _mm_storeu_ps(ptr + 4*i, _mm_castsi128_ps(dst.m[i]));
}
INLINE void STORE(geni1 &dst, float *ptr) {
  _mm_store_ss(ptr, _mm_castsi128_ps(dst.m[0]));
}

#undef INLINE

#endif /* __GBE_SIM_VECTOR_H__ */

