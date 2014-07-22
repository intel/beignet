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
 */
#include "immediate.hpp"

using namespace gbe;
using namespace ir;

#define SCALAR_SAME_TYPE_ASSERT()                           \
      GBE_ASSERT(this->getType() == right.getType()       && \
                 this->getElemNum() == right.getElemNum() && \
                 this->getElemNum() == 1                  && \
                 this->getType() != TYPE_BOOL);

#define DECLAR_BINARY_ALL_TYPE_OP(OP) \
    Immediate Immediate::operator OP (const Immediate &right) const { \
      SCALAR_SAME_TYPE_ASSERT(); \
      switch (this->getType()) { \
        default: \
          GBE_ASSERT(0); \
        case TYPE_S8:     return Immediate(*this->data.s8 OP *right.data.s8);   \
        case TYPE_U8:     return Immediate(*this->data.u8 OP *right.data.u8);   \
        case TYPE_S16:    return Immediate(*this->data.s16 OP *right.data.s16); \
        case TYPE_U16:    return Immediate(*this->data.u16 OP *right.data.u16); \
        case TYPE_S32:    return Immediate(*this->data.s32 OP *right.data.s32); \
        case TYPE_U32:    return Immediate(*this->data.u32 OP *right.data.u32); \
        case TYPE_S64:    return Immediate(*this->data.s64 OP *right.data.s64); \
        case TYPE_U64:    return Immediate(*this->data.u64 OP *right.data.u64); \
        case TYPE_FLOAT:  return Immediate(*this->data.f32 OP *right.data.f32); \
        case TYPE_DOUBLE: return Immediate(*this->data.f64 OP *right.data.f64); \
      }\
      return *this;\
    }

    DECLAR_BINARY_ALL_TYPE_OP(+)
    DECLAR_BINARY_ALL_TYPE_OP(-)
    DECLAR_BINARY_ALL_TYPE_OP(*)
    DECLAR_BINARY_ALL_TYPE_OP(/)

#undef DECLAR_BINARY_ALL_TYPE_OP

#define DECLAR_BINARY_INT_TYPE_OP(OP) \
    Immediate Immediate::operator OP (const Immediate &right) const { \
      SCALAR_SAME_TYPE_ASSERT(); \
      switch (this->getType()) { \
        default: \
          GBE_ASSERT(0); \
        case TYPE_S8:     return Immediate(*this->data.s8 OP *right.data.s8);   \
        case TYPE_U8:     return Immediate(*this->data.u8 OP *right.data.u8);   \
        case TYPE_S16:    return Immediate(*this->data.s16 OP *right.data.s16); \
        case TYPE_U16:    return Immediate(*this->data.u16 OP *right.data.u16); \
        case TYPE_S32:    return Immediate(*this->data.s32 OP *right.data.s32); \
        case TYPE_U32:    return Immediate(*this->data.u32 OP *right.data.u32); \
        case TYPE_S64:    return Immediate(*this->data.s64 OP *right.data.s64); \
        case TYPE_U64:    return Immediate(*this->data.u64 OP *right.data.u64); \
      }\
      return *this;\
    }
    DECLAR_BINARY_INT_TYPE_OP(%)
    DECLAR_BINARY_INT_TYPE_OP(&)
    DECLAR_BINARY_INT_TYPE_OP(|)
    DECLAR_BINARY_INT_TYPE_OP(^)
#undef DECLAR_BINARY_INT_TYPE_OP


#define DECLAR_BINARY_ASHIFT_OP(OP) \
    Immediate Immediate::operator OP (const Immediate &right) const { \
      GBE_ASSERT(this->getType() > TYPE_BOOL && this->getType() <= TYPE_U64); \
      int32_t shift = right.getIntegerValue(); \
      if (shift == 0) \
        return *this; \
      else \
        switch (this->getType()) { \
          default: \
            GBE_ASSERT(0); \
          case TYPE_S8:  return Immediate((*this->data.s8 OP shift));  \
          case TYPE_U8:  return Immediate((*this->data.u8 OP shift));  \
          case TYPE_S16: return Immediate((*this->data.s16 OP shift)); \
          case TYPE_U16: return Immediate((*this->data.u16 OP shift)); \
          case TYPE_S32: return Immediate((*this->data.s32 OP shift)); \
          case TYPE_U32: return Immediate((*this->data.u32 OP shift)); \
          case TYPE_S64: return Immediate((*this->data.s64 OP shift)); \
          case TYPE_U64: return Immediate((*this->data.u64 OP shift)); \
        } \
    }

    DECLAR_BINARY_ASHIFT_OP(>>)
    DECLAR_BINARY_ASHIFT_OP(<<)

#undef DECLAR_BINARY_ASHIFT_OP
    Immediate Immediate::lshr (const Immediate &left, const Immediate &right) {
      GBE_ASSERT(left.getType() > TYPE_BOOL && left.getType() <= TYPE_U64);
      int32_t shift = right.getIntegerValue();
      if (shift == 0)
        return left;
      else
        switch (left.getType()) {
          default:
            GBE_ASSERT(0);
          case TYPE_S8:  
          case TYPE_U8:  return Immediate((*left.data.u8 >> shift));
          case TYPE_S16: 
          case TYPE_U16: return Immediate((*left.data.u16 >> shift));
          case TYPE_S32: 
          case TYPE_U32: return Immediate((*left.data.u32 >> shift));
          case TYPE_S64: 
          case TYPE_U64: return Immediate((*left.data.u64 >> shift));
        }
    }

    Immediate::Immediate(ImmOpCode op, const Immediate &left, const Immediate &right, Type dstType) {
      switch (op) {
        default:
          GBE_ASSERT(0 && "unsupported imm op\n");
        case IMM_ADD: *this = left + right; break;
        case IMM_SUB: *this = left - right; break;
        case IMM_MUL: *this = left * right; break;
        case IMM_DIV: *this = left / right; break;
        case IMM_AND: *this = left & right; break;
        case IMM_OR:  *this = left | right; break;
        case IMM_XOR: *this = left ^ right; break;
        case IMM_REM:
        {
          if (left.getType() > TYPE_BOOL && left.getType() <= TYPE_U64)
            *this = left % right;
          else if (left.getType() == TYPE_FLOAT && right.getType() == TYPE_FLOAT) {
            *this = Immediate(left);
            *this->data.f32 = fmodf(left.getFloatValue(), right.getFloatValue());
          }
          else if (left.getType() == TYPE_DOUBLE && right.getType() == TYPE_DOUBLE) {
            *this = Immediate(left);
            *this->data.f64 += fmod(left.getDoubleValue(), right.getDoubleValue());
          }
          else
            GBE_ASSERT(0);
          break;
        }
        case IMM_LSHR:
        {
          if (left.getElemNum() == 1)
            lshr(left, right);
          else {
            GBE_ASSERT(right.getIntegerValue() <= (left.getElemNum() * left.getTypeSize() * 8));
            GBE_ASSERT(right.getIntegerValue() % (left.getTypeSize() * 8) == 0);
            copy(left, right.getIntegerValue() / (left.getTypeSize() * 8), left.getElemNum());
          }
          break;
        }
        case IMM_ASHR:
        {
          if (left.getElemNum() == 1)
            *this = left >> right;
          else {
            GBE_ASSERT(0 && "Doesn't support ashr on array constant.");
            copy(left, right.getIntegerValue() / (left.getTypeSize() * 8), left.getElemNum());
          }
          break;
        }
        case IMM_SHL:
        {
          if (left.getElemNum() == 1)
            *this = left << right;
          else {
            GBE_ASSERT(right.getIntegerValue() <= (left.getElemNum() * left.getTypeSize() * 8));
            GBE_ASSERT(right.getIntegerValue() % (left.getTypeSize() * 8) == 0);
            copy(left, -right.getIntegerValue() / (left.getTypeSize() * 8), left.getElemNum());
          }
          break;
        }
      }
      // If the dst type is large int, we will not change the imm type to large int.
      GBE_ASSERT(type == (ImmType)dstType || dstType == TYPE_LARGE_INT);
    }

    Immediate::Immediate(const vector<const Immediate*> immVec) {
      if (immVec.size() == 1) {
        *this = *immVec[0];
      } else if (!(immVec[0]->isCompType()) && immVec[0]->elemNum == 1) {
        this->type = immVec[0]->type;
        this->elemNum = immVec.size();
        if (immVec[0]->getTypeSize() * immVec.size() < 8)
          this->data.p = &this->defaultData;
        else
          this->data.p = malloc(immVec[0]->getTypeSize() * immVec.size());
        uint8_t *p = (uint8_t*)this->data.p;
        for(uint32_t i = 0; i < immVec.size(); i++) {
          GBE_ASSERT(immVec[i]->type == immVec[0]->type && immVec[i]->elemNum == 1);
          memcpy(p, immVec[i]->data.p, immVec[i]->getTypeSize());
          p += immVec[i]->getTypeSize();
        }
      } else {
        this->type = IMM_TYPE_COMP;
        if (immVec.size() * sizeof(Immediate*) < 8)
          this->data.p = &this->defaultData;
        else
          this->data.p = malloc(immVec.size() * sizeof(Immediate*));
        this->elemNum = immVec.size();
        for(uint32_t i = 0; i < immVec.size(); i++)
          this->data.immVec[i] = immVec[i];
      }
    }


    // operator = and copy() are only called from constructor functions
    // which this never hold a memory pointer, we don't need to bother
    // to check the data.p before assignment.
    Immediate & Immediate::operator= (const Immediate & other) {
      if (this != &other) {
        type = other.type;
        elemNum = other.elemNum;
        if (other.data.p != &other.defaultData) {
          data.p = malloc(other.elemNum * other.getTypeSize());
          memcpy(data.p, other.data.p, other.elemNum * other.getTypeSize());
        }
        else {
          defaultData = other.defaultData;
          data.p = &defaultData;
        }
      }
      return *this;
    }

    void Immediate::copy(const Immediate &other, int32_t offset, uint32_t num) {
      if (this != &other) {
        if (other.type == IMM_TYPE_COMP && num == 1) {
          GBE_ASSERT(offset >= 0 && offset <= (int32_t)other.elemNum);
          *this = *other.data.immVec[offset];
          return;
        }
        type = other.type;
        elemNum = num;
        if (num * other.getTypeSize() < 8)
          data.p = &defaultData;
        else
          data.p = malloc(num * other.getTypeSize());
        uint8_t* datap = (uint8_t*)data.p;
        memset(datap, 0, num * other.getTypeSize());
        if (offset < 0) {
          datap += (-offset) * other.getTypeSize();
          num -= num < (uint32_t)(-offset) ? num : (-offset);
          offset = 0;
        } else if (offset > 0 && num > 1) {
          GBE_ASSERT((int32_t)num > offset);
          num -= offset;
        }
        memcpy(datap, (uint8_t*)other.data.p + offset * other.getTypeSize(),
               num * other.getTypeSize());
      }
    }
