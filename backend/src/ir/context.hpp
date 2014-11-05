/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
 * \file context.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_CONTEXT_HPP__
#define __GBE_IR_CONTEXT_HPP__

#include "ir/instruction.hpp"
#include "ir/function.hpp"
#include "ir/register.hpp"
#include "ir/immediate.hpp"
#include "ir/unit.hpp"
#include "sys/vector.hpp"
#include <tuple>

namespace gbe {
namespace ir {

  /*! A context allows an easy creation of the functions (instruction stream and
   *  the set of immediates and registers needed for it) and constant arrays
   */
  class Context
  {
  public:
    /*! Create a new context for this unit */
    Context(Unit &unit);
    /*! Free resources needed by context */
    virtual ~Context(void);
    /*! Create a new function "name" */
    void startFunction(const std::string &name);
    /*! Close the function */
    void endFunction(void);
    /*! Get the current processed unit */
    INLINE Unit &getUnit(void) { return unit; }
    /*! Get the current processed function */
    Function &getFunction(void);
    /*! Get the current processed block */
    BasicBlock *getBlock(void) { return bb; }
    /*! Set the SIMD width of the function */
    void setSimdWidth(uint32_t width) const {
      GBE_ASSERT(width == 8 || width == 16);
      fn->simdWidth = width;
    }
    /*! Append a new pushed constant */
    void appendPushedConstant(Register reg, const PushLocation &pushed);
    /*! Create a new register with the given family for the current function */
    Register reg(RegisterFamily family, bool uniform = false);
    /*! Create a new immediate value */
    template <typename T> INLINE ImmediateIndex newImmediate(T value) {
      const Immediate imm(value);
      return fn->newImmediate(imm);
    }
    template <typename T> INLINE ImmediateIndex newImmediate(T value, uint32_t num) {
      const Immediate imm(value, num);
      return fn->newImmediate(imm);
    }
    /*! Create a new immediate value */
    INLINE ImmediateIndex newImmediate(vector<ImmediateIndex>indexVector) {
      vector<const Immediate*> immVector;
      for( uint32_t i = 0; i < indexVector.size(); i++)
        immVector.push_back(&fn->getImmediate(indexVector[i]));
      const Immediate imm(immVector);
      return fn->newImmediate(imm);
    }
    /*! Create an integer immediate value */
    INLINE ImmediateIndex newIntegerImmediate(int64_t x, Type type) {
      switch (type) {
        case TYPE_S8: return this->newImmediate(int8_t(x));
        case TYPE_U8: return this->newImmediate(uint8_t(x));
        case TYPE_S16: return this->newImmediate(int16_t(x));
        case TYPE_U16: return this->newImmediate(uint16_t(x));
        case TYPE_S32: return this->newImmediate(int32_t(x));
        case TYPE_U32: return this->newImmediate(uint32_t(x));
        case TYPE_S64: return this->newImmediate(int64_t(x));
        case TYPE_U64: return this->newImmediate(uint64_t(x));
        default: NOT_SUPPORTED; return ImmediateIndex(0);
      }
      return ImmediateIndex(0);
    }
    INLINE ImmediateIndex newFloatImmediate(float x) {
      return this->newImmediate(x);
    }
    INLINE ImmediateIndex newDoubleImmediate(double x) {
      return this->newImmediate(x);
    }

    INLINE ImmediateIndex processImm(ImmOpCode op, ImmediateIndex src, Type type) {
      const Immediate &imm = fn->getImmediate(src);
      const Immediate &dstImm = Immediate(op, imm, type);
      return fn->newImmediate(dstImm);
    }

    INLINE ImmediateIndex processImm(ImmOpCode op, ImmediateIndex src0,
                                     ImmediateIndex src1, Type type) {
      const Immediate &imm0 = fn->getImmediate(src0);
      const Immediate &imm1 = fn->getImmediate(src1);
      const Immediate &dstImm = Immediate(op, imm0, imm1, type);
      return fn->newImmediate(dstImm);
    }

    /*! Create a new register holding the given value. A LOADI is pushed */
    template <typename T> INLINE Register immReg(T value) {
      GBE_ASSERTM(fn != NULL, "No function currently defined");
      const Immediate imm(value);
      const ImmediateIndex index = fn->newImmediate(imm);
      const RegisterFamily family = getFamily(imm.getType());
      const Register reg = this->reg(family);
      this->LOADI(imm.getType(), reg, index);
      return reg;
    }
    /*! Create a new label for the current function */
    LabelIndex label(void);
    /*! Append a new input register for the function */
    void input(const std::string &name, FunctionArgument::Type type, Register reg,
               FunctionArgument::InfoFromLLVM& info, uint32_t elemSz = 0u, uint32_t align = 0, uint8_t bti = 0);
    /*! Append a new output register for the function */
    void output(Register reg);
    /*! Get the immediate value */
    INLINE Immediate getImmediate(ImmediateIndex index) const {
      return fn->getImmediate(index);
    }
    /*! Append a new tuple */
    template <typename... Args> INLINE Tuple tuple(Args...args) {
      GBE_ASSERTM(fn != NULL, "No function currently defined");
      return fn->file.appendTuple(args...);
    }
    /*! Make a tuple from an array of register */
    INLINE Tuple arrayTuple(const Register *reg, uint32_t regNum) {
      GBE_ASSERTM(fn != NULL, "No function currently defined");
      return fn->file.appendArrayTuple(reg, regNum);
    }
    /*! We just use variadic templates to forward instruction functions */
#define DECL_INSN(NAME, FAMILY) \
    template <typename... Args> INLINE void NAME(Args...args);
#include "ir/instruction.hxx"
#undef DECL_INSN
    /*! Return the pointer size handled by the unit */
    INLINE PointerSize getPointerSize(void) const {
      return unit.getPointerSize();
    }
    /*! Return the family of registers that contain pointer */
    INLINE RegisterFamily getPointerFamily(void) const {
      return unit.getPointerFamily();
    }
#define DECL_THREE_SRC_INSN(NAME) \
    INLINE void NAME(Type type, \
                     Register dst, \
                     Register src0, \
                     Register src1, \
                     Register src2) \
    { \
      const Tuple index = this->tuple(src0, src1, src2); \
      this->NAME(type, dst, index); \
    }
    DECL_THREE_SRC_INSN(SEL);
    DECL_THREE_SRC_INSN(I64MADSAT);
    DECL_THREE_SRC_INSN(MAD);
#undef DECL_THREE_SRC_INSN

    /*! For all unary functions */
    void ALU1(Opcode opcode, Type type, Register dst, Register src) {
      const Instruction insn = gbe::ir::ALU1(opcode, type, dst, src);
      this->append(insn);
    }

    /*! LOAD with the destinations directly specified */
    template <typename... Args>
    void LOAD(Type type, Register offset, AddressSpace space, bool dwAligned, BTI bti, Args...values)
    {
      const Tuple index = this->tuple(values...);
      const uint16_t valueNum = std::tuple_size<std::tuple<Args...>>::value;
      GBE_ASSERT(valueNum > 0);
      this->LOAD(type, index, offset, space, valueNum, dwAligned, bti);
    }

    /*! STORE with the sources directly specified */
    template <typename... Args>
    void STORE(Type type, Register offset, AddressSpace space, bool dwAligned, BTI bti, Args...values)
    {
      const Tuple index = this->tuple(values...);
      const uint16_t valueNum = std::tuple_size<std::tuple<Args...>>::value;
      GBE_ASSERT(valueNum > 0);
      this->STORE(type, index, offset, space, valueNum, dwAligned, bti);
    }
    void appendSurface(uint8_t bti, Register reg) { fn->appendSurface(bti, reg); }

  protected:
    /*! A block must be started with a label */
    void startBlock(void);
    /*! A block must be ended with a branch */
    void endBlock(void);
    /*! Append the instruction in the current basic block */
    void append(const Instruction &insn);
    Unit &unit;                 //!< A unit is associated to a contect
    Function *fn;               //!< Current function we are processing
    BasicBlock *bb;             //!< Current basic block we are filling
    static const uint8_t LABEL_IS_POINTED = 1 << 0; //!< Branch is using it
    static const uint8_t LABEL_IS_DEFINED = 1 << 1; //!< Label is defining it
    vector<uint8_t> *usedLabels;
    /*! Functions can be defined recursiely */
    struct StackElem {
      INLINE StackElem(Function *fn, BasicBlock *bb, vector<uint8_t> *usedLabels)
        : fn(fn), bb(bb), usedLabels(usedLabels)
      {}
      Function *fn;                //!< Function to process
      BasicBlock *bb;              //!< Basic block currently processed
      vector<uint8_t> *usedLabels; //!< Store all labels that are defined
    };
    vector<StackElem> fnStack;     //!< Stack of functions still to finish
    GBE_CLASS(Context);
  };

  // Use argument checker to assert argument value correctness
#define DECL_INSN(NAME, FAMILY) \
  template <typename... Args> \
  INLINE void Context::NAME(Args...args) { \
    GBE_ASSERTM(fn != NULL, "No function currently defined"); \
    const Instruction insn = gbe::ir::NAME(args...); \
    this->append(insn); \
  }
#include "ir/instruction.hxx"
#undef DECL_INSN

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_CONTEXT_HPP__ */

