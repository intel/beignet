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
 * \file llvm_gen_backend.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

/* Transform the LLVM IR code into Gen IR code i.e. our temporary representation
 * for programs running on Gen.
 *
 * Overview
 * ========
 *
 * This code is mostly inspired by the (now defunct and replaced by CppBackend)
 * CBackend. Basically, there are two ways to transform LLVM code into machine
 * code (or anything else)
 * - You write a complete LLVM backend by the book. LLVM proposes a lot of
 *   useful tools to do so. This is obviously the path chosen by all CPU guys
 *   but also by AMD and nVidia which both use the backend infrastructure to
 *   output their own intermediate language. The good point is that you can
 *   reuse a lot of tools (like proper PHI elimination with phi congruence and
 *   global copy propagation a la Chaitin). Bad points are:
 *     1/ It is a *long* journey to generate anything.
 *     2/ More importantly, the code is hugely biased towards CPUs. Typically,
 *        the way registers are defined do not fit well Gen register file (which
 *        is really more like a regular piece of memory). Same issue apply for
 *        predicated instructions with mask which is a bit boring to use with
 *        SSA. Indeed, since DAGSelection still manipulates SSA values, anything
 *        predicated requires to insert extra sources
 * - You write function passes to do the translation yourself. Obviously, you
 *   reinvent the wheel. However, it is easy to do and easier to maintain
 *   (somehow)
 *
 * So, the code here just traverses LLVM asm and generates our own ISA. The
 * generated code is OK even if a global copy propagation pass is still overdue.
 * Right now, it is pretty straighforward and simplistic in that regard
 *
 * About Clang and the ABI / target
 * ================================
 *
 * A major question is: how did we actually generate this LLVM code from OpenCL?
 * Well, thing is that there is no generic target in LLVM since there are many
 * dependencies on endianness or ABIs. Fortunately, the ptx (and nvptx for LLVM
 * 3.2) profile is pretty well adapted to our needs since NV and Gen GPU are
 * kind of similar, or at least they are similar enough to share the same front
 * end.
 *
 * Problems
 * ========
 *
 * - Several things regarding constants like ConstantExpr are not properly handled.
 * - ptx front end generates function calls. Since we do not support them yet,
 *   the user needs to force the inlining of all functions. If a function call
 *   is intercepted, we just abort
 */

#include "llvm/Config/llvm-config.h"
#if LLVM_VERSION_MINOR <= 2
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#else
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#endif  /* LLVM_VERSION_MINOR <= 2 */
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#if LLVM_VERSION_MINOR <= 2
#include "llvm/Intrinsics.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/InlineAsm.h"
#else
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InlineAsm.h"
#endif  /* LLVM_VERSION_MINOR <= 2 */
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/ConstantsScanner.h"
#include "llvm/Analysis/FindUsedTypes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/IntrinsicLowering.h"

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >=5
#include "llvm/IR/Mangler.h"
#else
#include "llvm/Target/Mangler.h"
#endif

#include "llvm/Transforms/Scalar.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#if !defined(LLVM_VERSION_MAJOR) || (LLVM_VERSION_MINOR == 1)
#include "llvm/Target/TargetData.h"
#elif LLVM_VERSION_MINOR == 2
#include "llvm/DataLayout.h"
#else
#include "llvm/IR/DataLayout.h"
#endif

#if LLVM_VERSION_MINOR >= 5
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CFG.h"
#else
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CFG.h"
#endif

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR <= 2)
#include "llvm/Support/InstVisitor.h"
#elif LLVM_VERSION_MINOR >= 5
#include "llvm/IR/InstVisitor.h"
#else
#include "llvm/InstVisitor.h"
#endif
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/SourceMgr.h"

#include "llvm/llvm_gen_backend.hpp"
#include "ir/context.hpp"
#include "ir/unit.hpp"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "sys/set.hpp"
#include "sys/cvar.hpp"
#include "backend/program.h"
#include <sstream>

/* Not defined for LLVM 3.0 */
#if !defined(LLVM_VERSION_MAJOR)
#define LLVM_VERSION_MAJOR 3
#endif /* !defined(LLVM_VERSION_MAJOR) */

#if !defined(LLVM_VERSION_MINOR)
#define LLVM_VERSION_MINOR 0
#endif /* !defined(LLVM_VERSION_MINOR) */

#if (LLVM_VERSION_MAJOR != 3) || (LLVM_VERSION_MINOR < 3)
#error "Only LLVM 3.3 and newer are supported"
#endif /* (LLVM_VERSION_MAJOR != 3) || (LLVM_VERSION_MINOR > 4) */

using namespace llvm;

namespace gbe
{
  /*! Gen IR manipulates only scalar types */
  static bool isScalarType(const Type *type)
  {
    return type->isFloatTy()   ||
           type->isIntegerTy() ||
           type->isDoubleTy()  ||
           type->isPointerTy();
  }

  /*! LLVM IR Type to Gen IR type translation */
  static ir::Type getType(ir::Context &ctx, const Type *type)
  {
    GBE_ASSERT(isScalarType(type));
    if (type->isFloatTy() == true)
      return ir::TYPE_FLOAT;
    if (type->isDoubleTy() == true)
      return ir::TYPE_DOUBLE;
    if (type->isPointerTy() == true) {
      if (ctx.getPointerSize() == ir::POINTER_32_BITS)
        return ir::TYPE_U32;
      else
        return ir::TYPE_U64;
    }
    GBE_ASSERT(type->isIntegerTy() == true);
    if (type == Type::getInt1Ty(type->getContext()))
      return ir::TYPE_BOOL;
    if (type == Type::getInt8Ty(type->getContext()))
      return ir::TYPE_S8;
    if (type == Type::getInt16Ty(type->getContext()))
      return ir::TYPE_S16;
    if (type == Type::getInt32Ty(type->getContext()))
      return ir::TYPE_S32;
    if (type == Type::getInt64Ty(type->getContext()))
      return ir::TYPE_S64;
    return ir::TYPE_LARGE_INT;
  }

  /*! LLVM IR Type to Gen IR unsigned type translation */
  static ir::Type getUnsignedType(ir::Context &ctx, const Type *type)
  {
    GBE_ASSERT(type->isIntegerTy() == true);
    if (type == Type::getInt1Ty(type->getContext()))
      return ir::TYPE_BOOL;
    if (type == Type::getInt8Ty(type->getContext()))
      return ir::TYPE_U8;
    if (type == Type::getInt16Ty(type->getContext()))
      return ir::TYPE_U16;
    if (type == Type::getInt32Ty(type->getContext()))
      return ir::TYPE_U32;
    if (type == Type::getInt64Ty(type->getContext()))
      return ir::TYPE_U64;
    ctx.getUnit().setValid(false);
    return ir::TYPE_U64;
  }

  /*! Type to register family translation */
  static ir::RegisterFamily getFamily(ir::Context &ctx, const Type *type)
  {
    GBE_ASSERT(isScalarType(type) == true);
    if (type == Type::getInt1Ty(type->getContext()))
      return ir::FAMILY_BOOL;
    if (type == Type::getInt8Ty(type->getContext()))
      return ir::FAMILY_BYTE;
    if (type == Type::getInt16Ty(type->getContext()))
      return ir::FAMILY_WORD;
    if (type == Type::getInt32Ty(type->getContext()) || type->isFloatTy())
      return ir::FAMILY_DWORD;
    if (type == Type::getInt64Ty(type->getContext()) || type->isDoubleTy())
      return ir::FAMILY_QWORD;
    if (type->isPointerTy())
      return ctx.getPointerFamily();
    ctx.getUnit().setValid(false);
    return ir::FAMILY_BOOL;
  }

  /*! Get number of element to process dealing either with a vector or a scalar
   *  value
   */
  static ir::Type getVectorInfo(ir::Context &ctx, Type *llvmType, Value *value, uint32_t &elemNum, bool useUnsigned = false)
  {
    ir::Type type;
    if (llvmType->isVectorTy() == true) {
      VectorType *vectorType = cast<VectorType>(llvmType);
      Type *elementType = vectorType->getElementType();
      elemNum = vectorType->getNumElements();
      if (useUnsigned)
        type = getUnsignedType(ctx, elementType);
      else
        type = getType(ctx, elementType);
    } else {
      elemNum = 1;
      if (useUnsigned)
        type = getUnsignedType(ctx, llvmType);
      else
        type = getType(ctx, llvmType);
    }
    return type;
  }

  /*! OCL to Gen-IR address type */
  static INLINE ir::AddressSpace addressSpaceLLVMToGen(unsigned llvmMemSpace) {
    switch (llvmMemSpace) {
      case 0: return ir::MEM_PRIVATE;
      case 1: return ir::MEM_GLOBAL;
      case 2: return ir::MEM_CONSTANT;
      case 3: return ir::MEM_LOCAL;
      case 4: return ir::IMAGE;
    }
    GBE_ASSERT(false);
    return ir::MEM_GLOBAL;
  }

  static Constant *extractConstantElem(Constant *CPV, uint32_t index) {
    ConstantVector *CV = dyn_cast<ConstantVector>(CPV);
    GBE_ASSERT(CV != NULL);
#if GBE_DEBUG
    const uint32_t elemNum = CV->getNumOperands();
    GBE_ASSERTM(index < elemNum, "Out-of-bound constant vector access");
#endif /* GBE_DEBUG */
    CPV = cast<Constant>(CV->getOperand(index));
    return CPV;
  }

  /*! Handle the LLVM IR Value to Gen IR register translation. This has 2 roles:
   *  - Split the LLVM vector into several scalar values
   *  - Handle the transparent copies (bitcast or use of intrincics functions
   *    like get_local_id / get_global_id
   */
  class RegisterTranslator
  {
  public:
    /*! Indices will be zero for scalar values */
    typedef std::pair<Value*, uint32_t> ValueIndex;
    RegisterTranslator(ir::Context &ctx) : ctx(ctx) {}

    /*! Empty the maps */
    void clear(void) {
      valueMap.clear();
      scalarMap.clear();
    }
    /*! Some values will not be allocated. For example, a bit-cast destination
     *  like: %fake = bitcast %real or a vector insertion since we do not have
     *  vectors in Gen-IR
     */
    void newValueProxy(Value *real,
                       Value *fake,
                       uint32_t realIndex = 0u,
                       uint32_t fakeIndex = 0u) {
      const ValueIndex key(fake, fakeIndex);
      const ValueIndex value(real, realIndex);
      GBE_ASSERT(valueMap.find(key) == valueMap.end()); // Do not insert twice
      valueMap[key] = value;
    }
    /*! Mostly used for the preallocated registers (lids, gids) */
    void newScalarProxy(ir::Register reg, Value *value, uint32_t index = 0u) {
      const ValueIndex key(value, index);
      GBE_ASSERT(scalarMap.find(key) == scalarMap.end());
      scalarMap[key] = reg;
    }
    /*! Allocate a new scalar register */
    ir::Register newScalar(Value *value, Value *key = NULL, uint32_t index = 0u, bool uniform = false)
    {
      // we don't allow normal constant, but GlobalValue is a special case,
      // it needs a register to store its address
      GBE_ASSERT(! (isa<Constant>(value) && !isa<GlobalValue>(value)));
      Type *type = value->getType();
      auto typeID = type->getTypeID();
      switch (typeID) {
        case Type::IntegerTyID:
        case Type::FloatTyID:
        case Type::DoubleTyID:
        case Type::PointerTyID:
          GBE_ASSERT(index == 0);
          return this->_newScalar(value, key, type, index, uniform);
          break;
        case Type::VectorTyID:
        {
          auto vectorType = cast<VectorType>(type);
          auto elementType = vectorType->getElementType();
          auto elementTypeID = elementType->getTypeID();
          if (elementTypeID != Type::IntegerTyID &&
              elementTypeID != Type::FloatTyID &&
              elementTypeID != Type::DoubleTyID)
            GBE_ASSERTM(false, "Vectors of elements are not supported");
            return this->_newScalar(value, key, elementType, index, uniform);
          break;
        }
        default: NOT_SUPPORTED;
      };
      return ir::Register();
    }

    /*! iterating in the value map to get the final real register */
    void getRealValue(Value* &value, uint32_t& index) {
      auto end = valueMap.end();
      for (;;) {
        auto it = valueMap.find(std::make_pair(value, index));
        if (it == end)
          break;
        else {
          value = it->second.first;
          index = it->second.second;
        }
      }
    }

    /*! Get the register from the given value at given index possibly iterating
     *  in the value map to get the final real register
     */
    ir::Register getScalar(Value *value, uint32_t index = 0u) {
      getRealValue(value, index);

      const auto key = std::make_pair(value, index);
      GBE_ASSERT(scalarMap.find(key) != scalarMap.end());
      return scalarMap[key];
    }
    /*! Insert a given register at given Value position */
    void insertRegister(const ir::Register &reg, Value *value, uint32_t index) {
      const auto key = std::make_pair(value, index);
      GBE_ASSERT(scalarMap.find(key) == scalarMap.end());
      scalarMap[key] = reg;
    }
    /*! Says if the value exists. Otherwise, it is undefined */
    bool valueExists(Value *value, uint32_t index) {
      getRealValue(value, index);

      const auto key = std::make_pair(value, index);
      return scalarMap.find(key) != scalarMap.end();
    }
    /*! if it's a undef const value, return true. Otherwise, return false. */
    bool isUndefConst(Value *value, uint32_t index) {
      getRealValue(value, index);

      Constant *CPV = dyn_cast<Constant>(value);
      if(CPV && dyn_cast<ConstantVector>(CPV))
        CPV = extractConstantElem(CPV, index);
      return (CPV && (isa<UndefValue>(CPV)));
    }
  private:
    /*! This creates a scalar register for a Value (index is the vector index when
     *  the value is a vector of scalars)
     */
    ir::Register _newScalar(Value *value, Value *key, Type *type, uint32_t index, bool uniform) {
      const ir::RegisterFamily family = getFamily(ctx, type);
      const ir::Register reg = ctx.reg(family, uniform);
      key = key == NULL ? value : key;
      this->insertRegister(reg, key, index);
      return reg;
    }
    /*! Map value to ir::Register */
    map<ValueIndex, ir::Register> scalarMap;
    /*! Map values to values when this is only a translation (eq bitcast) */
    map<ValueIndex, ValueIndex> valueMap;
    /*! Actually allocates the registers */
    ir::Context &ctx;
  };

  /*! Translate LLVM IR code to Gen IR code */
  class GenWriter : public FunctionPass, public InstVisitor<GenWriter>
  {
    /*! Unit to compute */
    ir::Unit &unit;
    /*! Helper structure to compute the unit */
    ir::Context ctx;
    /*! Make the LLVM-to-Gen translation */
    RegisterTranslator regTranslator;
    /*! Map target basic block to its ir::LabelIndex */
    map<const BasicBlock*, ir::LabelIndex> labelMap;
    /*! Condition inversion can simplify branch code. We store here all the
     *  compare instructions we need to invert to decrease branch complexity
     */
    set<const Value*> conditionSet;
    map<const Value*, int> globalPointer;
    /*!
     *  <phi,phiCopy> node information for later optimization
     */
    map<const ir::Register, const ir::Register> phiMap;
    /*! We visit each function twice. Once to allocate the registers and once to
     *  emit the Gen IR instructions
     */
    enum Pass {
      PASS_EMIT_REGISTERS = 0,
      PASS_EMIT_INSTRUCTIONS = 1
    } pass;

    typedef enum {
      CONST_INT,
      CONST_FLOAT,
      CONST_DOUBLE
    } ConstTypeId;

    LoopInfo *LI;
    const Module *TheModule;
    int btiBase;
  public:
    static char ID;
    explicit GenWriter(ir::Unit &unit)
      : FunctionPass(ID),
        unit(unit),
        ctx(unit),
        regTranslator(ctx),
        LI(0),
        TheModule(0),
        btiBase(BTI_RESERVED_NUM)
    {
      initializeLoopInfoPass(*PassRegistry::getPassRegistry());
      pass = PASS_EMIT_REGISTERS;
    }

    virtual const char *getPassName() const { return "Gen Back-End"; }

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.setPreservesAll();
    }

    virtual bool doInitialization(Module &M);
    /*! helper function for parsing global constant data */
    void getConstantData(const Constant * c, void* mem, uint32_t& offset) const;
    void collectGlobalConstant(void) const;
    ir::ImmediateIndex processConstantImmIndex(Constant *CPV, int32_t index = 0u);
    const ir::Immediate &processConstantImm(Constant *CPV, int32_t index = 0u);

    uint32_t incBtiBase() {
      GBE_ASSERT(btiBase <= BTI_MAX_ID);
      return btiBase++;
    }

    bool runOnFunction(Function &F) {
     // Do not codegen any 'available_externally' functions at all, they have
     // definitions outside the translation unit.
     if (F.hasAvailableExternallyLinkage())
       return false;

      // As we inline all function calls, so skip non-kernel functions
      bool bKernel = isKernelFunction(F);
      if(!bKernel) return false;

      LI = &getAnalysis<LoopInfo>();
      emitFunction(F);
      phiMap.clear();
      globalPointer.clear();
      // Reset for next function
      btiBase = BTI_RESERVED_NUM;
      return false;
    }

    virtual bool doFinalization(Module &M) { return false; }
    /*! handle global variable register allocation (local, constant space) */
    void allocateGlobalVariableRegister(Function &F);
    /*! gather all the loops in the function and add them to ir::Function */
    void gatherLoopInfo(ir::Function &fn);
    /*! Emit the complete function code and declaration */
    void emitFunction(Function &F);
    /*! Handle input and output function parameters */
    void emitFunctionPrototype(Function &F);
    /*! Emit the code for a basic block */
    void emitBasicBlock(BasicBlock *BB);
    /*! Each block end may require to emit MOVs for further PHIs */
    void emitMovForPHI(BasicBlock *curr, BasicBlock *succ);
    /*! Alocate one or several registers (if vector) for the value */
    INLINE void newRegister(Value *value, Value *key = NULL, bool uniform = false);
    /*! get the register for a llvm::Constant */
    ir::Register getConstantRegister(Constant *c, uint32_t index = 0);
    /*! get constant pointer */
    ir::Register getConstantPointerRegister(ConstantExpr *ce, uint32_t index = 0);
    /*! Return a valid register from an operand (can use LOADI to make one) */
    INLINE ir::Register getRegister(Value *value, uint32_t index = 0);
    /*! Create a new immediate from a constant */
    ir::ImmediateIndex newImmediate(Constant *CPV, uint32_t index = 0);
    /*! Insert a new label index when this is a scalar value */
    INLINE void newLabelIndex(const BasicBlock *bb);
    /*! Inspect the terminator instruction and try to see if we should invert
     *  the value to simplify the code
     */
    INLINE void simplifyTerminator(BasicBlock *bb);
    /*! Helper function to emit loads and stores */
    template <bool isLoad, typename T> void emitLoadOrStore(T &I);
    /*! Will try to remove MOVs due to PHI resolution */
    void removeMOVs(const ir::Liveness &liveness, ir::Function &fn);
    /*! Optimize phi move based on liveness information */
    void optimizePhiCopy(ir::Liveness &liveness, ir::Function &fn);
    /*! Will try to remove redundants LOADI in basic blocks */
    void removeLOADIs(const ir::Liveness &liveness, ir::Function &fn);
    /*! To avoid lost copy, we need two values for PHI. This function create a
     * fake value for the copy (basically ptr+1)
     */
    INLINE Value *getPHICopy(Value *PHI);
    // Currently supported instructions
#define DECL_VISIT_FN(NAME, TYPE) \
    void regAllocate##NAME(TYPE &I); \
    void emit##NAME(TYPE &I); \
    void visit##NAME(TYPE &I) { \
      if (pass == PASS_EMIT_INSTRUCTIONS) \
        emit##NAME(I); \
      else \
        regAllocate##NAME(I); \
    }
    DECL_VISIT_FN(BinaryOperator, Instruction);
    DECL_VISIT_FN(CastInst, CastInst);
    DECL_VISIT_FN(ReturnInst, ReturnInst);
    DECL_VISIT_FN(LoadInst, LoadInst);
    DECL_VISIT_FN(StoreInst, StoreInst);
    DECL_VISIT_FN(CallInst, CallInst);
    DECL_VISIT_FN(ICmpInst, ICmpInst);
    DECL_VISIT_FN(FCmpInst, FCmpInst);
    DECL_VISIT_FN(InsertElement, InsertElementInst);
    DECL_VISIT_FN(ExtractElement, ExtractElementInst);
    DECL_VISIT_FN(ShuffleVectorInst, ShuffleVectorInst);
    DECL_VISIT_FN(SelectInst, SelectInst);
    DECL_VISIT_FN(BranchInst, BranchInst);
    DECL_VISIT_FN(PHINode, PHINode);
    DECL_VISIT_FN(AllocaInst, AllocaInst);
#undef DECL_VISIT_FN

    // Emit unary instructions from gen native function
    void emitUnaryCallInst(CallInst &I, CallSite &CS, ir::Opcode opcode);
    // Emit unary instructions from gen native function
    void emitAtomicInst(CallInst &I, CallSite &CS, ir::AtomicOps opcode);

    uint8_t appendSampler(CallSite::arg_iterator AI);

    // These instructions are not supported at all
    void visitVAArgInst(VAArgInst &I) {NOT_SUPPORTED;}
    void visitSwitchInst(SwitchInst &I) {NOT_SUPPORTED;}
    void visitInvokeInst(InvokeInst &I) {NOT_SUPPORTED;}
#if LLVM_VERSION_MINOR == 0
    void visitUnwindInst(UnwindInst &I) {NOT_SUPPORTED;}
#endif /* __LLVM_30__ */
    void visitResumeInst(ResumeInst &I) {NOT_SUPPORTED;}
    void visitInlineAsm(CallInst &I) {NOT_SUPPORTED;}
    void visitIndirectBrInst(IndirectBrInst &I) {NOT_SUPPORTED;}
    void visitUnreachableInst(UnreachableInst &I) {NOT_SUPPORTED;}
    void visitGetElementPtrInst(GetElementPtrInst &I) {NOT_SUPPORTED;}
    void visitInsertValueInst(InsertValueInst &I) {NOT_SUPPORTED;}
    void visitExtractValueInst(ExtractValueInst &I) {NOT_SUPPORTED;}
    template <bool isLoad, typename T> void visitLoadOrStore(T &I);

    INLINE void gatherBTI(Value *pointer, ir::BTI &bti);
    // batch vec4/8/16 load/store
    INLINE void emitBatchLoadOrStore(const ir::Type type, const uint32_t elemNum,
                  Value *llvmValue, const ir::Register ptr,
                  const ir::AddressSpace addrSpace, Type * elemType, bool isLoad, ir::BTI bti);
    void visitInstruction(Instruction &I) {NOT_SUPPORTED;}
    private:
      ir::ImmediateIndex processConstantImmIndexImpl(Constant *CPV, int32_t index = 0u);
      template <typename T, typename P = T>
      ir::ImmediateIndex processSeqConstant(ConstantDataSequential *seq,
                                            int index, ConstTypeId tid);
      ir::ImmediateIndex processConstantVector(ConstantVector *cv, int index);
  };

  char GenWriter::ID = 0;
  void getSequentialData(const ConstantDataSequential *cda, void *ptr, uint32_t &offset) {
    StringRef data = cda->getRawDataValues();
    memcpy((char*)ptr+offset, data.data(), data.size());
    offset += data.size();
    return;
  }

  void GenWriter::getConstantData(const Constant * c, void* mem, uint32_t& offset) const {
    Type * type = c->getType();
    Type::TypeID id = type->getTypeID();

    GBE_ASSERT(c);
    if(isa<UndefValue>(c)) {
      uint32_t size = getTypeByteSize(unit, type);
      offset += size;
      return;
    } else if(isa<ConstantAggregateZero>(c)) {
      uint32_t size = getTypeByteSize(unit, type);
      memset((char*)mem+offset, 0, size);
      offset += size;
      return;
    }

    switch(id) {
      case Type::TypeID::StructTyID:
        {
          const StructType * strTy = cast<StructType>(c->getType());
          uint32_t size = 0;

          for(uint32_t op=0; op < strTy->getNumElements(); op++)
          {
            Type* elementType = strTy->getElementType(op);
            uint32_t align = 8 * getAlignmentByte(unit, elementType);
            uint32_t padding = getPadding(size, align);
            size += padding;
            size += getTypeBitSize(unit, elementType);

            offset += padding/8;
            const Constant* sub = cast<Constant>(c->getOperand(op));
            GBE_ASSERT(sub);
            getConstantData(sub, mem, offset);
          }
          break;
        }
      case Type::TypeID::ArrayTyID:
        {
          const ConstantDataSequential *cds = dyn_cast<ConstantDataSequential>(c);
          if(cds)
            getSequentialData(cds, mem, offset);
          else {
            const ConstantArray *ca = dyn_cast<ConstantArray>(c);
            const ArrayType *arrTy = ca->getType();
            Type* elemTy = arrTy->getElementType();
            uint32_t elemSize = getTypeBitSize(unit, elemTy);
            uint32_t padding = getPadding(elemSize, 8 * getAlignmentByte(unit, elemTy));
            padding /= 8;
            uint32_t ops = c->getNumOperands();
            for(uint32_t op = 0; op < ops; ++op) {
              Constant * ca = dyn_cast<Constant>(c->getOperand(op));
              getConstantData(ca, mem, offset);
              offset += padding;
            }
          }
          break;
        }
      case Type::TypeID::VectorTyID:
        {
          const ConstantDataSequential *cds = dyn_cast<ConstantDataSequential>(c);
          const VectorType *vecTy = cast<VectorType>(type);
          GBE_ASSERT(cds);
          getSequentialData(cds, mem, offset);
          if(vecTy->getNumElements() == 3) // OCL spec require align to vec4
            offset += getTypeByteSize(unit, vecTy->getElementType());
          break;
        }
      case Type::TypeID::IntegerTyID:
        {
          const ConstantInt *ci = dyn_cast<ConstantInt>(c);
          uint32_t size = ci->getBitWidth() / 8;
          uint64_t data = ci->isNegative() ? ci->getSExtValue() : ci->getZExtValue();
          memcpy((char*)mem+offset, &data, size);
          offset += size;
          break;
        }
      case Type::TypeID::FloatTyID:
        {
          const ConstantFP *cf = dyn_cast<ConstantFP>(c);
          *(float *)((char*)mem + offset) = cf->getValueAPF().convertToFloat();
          offset += sizeof(float);
          break;
        }
      case Type::TypeID::DoubleTyID:
        {
          const ConstantFP *cf = dyn_cast<ConstantFP>(c);
          *(double *)((char*)mem + offset) = cf->getValueAPF().convertToDouble();
          offset += sizeof(double);
          break;
        }
      default:
        NOT_IMPLEMENTED;
    }
  }

  void GenWriter::collectGlobalConstant(void) const {
    const Module::GlobalListType &globalList = TheModule->getGlobalList();
    for(auto i = globalList.begin(); i != globalList.end(); i ++) {
      const GlobalVariable &v = *i;
      if(!v.isConstantUsed()) continue;
      const char *name = v.getName().data();
      unsigned addrSpace = v.getType()->getAddressSpace();
      if(addrSpace == ir::AddressSpace::MEM_CONSTANT) {
        GBE_ASSERT(v.hasInitializer());
        const Constant *c = v.getInitializer();
        Type * type = c->getType();

        uint32_t size = getTypeByteSize(unit, type);
        void* mem = malloc(size);
        uint32_t offset = 0;
        getConstantData(c, mem, offset);
        uint32_t alignment = getAlignmentByte(unit, type);
        unit.newConstant((char *)mem, name, size, alignment);
        free(mem);
      }
    }
  }

  bool GenWriter::doInitialization(Module &M) {
    FunctionPass::doInitialization(M);

    // Initialize
    TheModule = &M;
    collectGlobalConstant();
    return false;
  }

  #define GET_EFFECT_DATA(_seq, _index, _tid) \
    ((_tid == CONST_INT) ? _seq->getElementAsInteger(_index) : \
    ((_tid == CONST_FLOAT) ? _seq->getElementAsFloat(_index) : \
    _seq->getElementAsDouble(_index)))

  // typename P is for bool only, as c++ set the &vector<bool)vec[0] to void
  // type. We have to use uint8_t for bool vector.
  template <typename T, typename P>
  ir::ImmediateIndex GenWriter::processSeqConstant(ConstantDataSequential *seq,
                                                   int index, ConstTypeId tid) {
    if (index >= 0) {
      const T data = GET_EFFECT_DATA(seq, index, tid);
      return ctx.newImmediate(data);
    } else {
      vector<P> array;
      for(int i = 0; i < seq->getNumElements(); i++)
        array.push_back(GET_EFFECT_DATA(seq, i, tid));
      return ctx.newImmediate((T*)&array[0], array.size());
    }
  }

  ir::ImmediateIndex GenWriter::processConstantVector(ConstantVector *cv, int index) {
    if (index >= 0) {
      Constant *c = cv->getOperand(index);
      return processConstantImmIndex(c, -1);
    } else {
      vector<ir::ImmediateIndex> immVector;
      for (uint32_t i = 0; i < cv->getNumOperands(); i++)
        immVector.push_back(processConstantImmIndex(cv->getOperand(i)));
      return ctx.newImmediate(immVector);
    }
  }

  ir::ImmediateIndex GenWriter::processConstantImmIndexImpl(Constant *CPV, int32_t index)
  {
    GBE_ASSERT(dyn_cast<ConstantExpr>(CPV) == NULL);

#if LLVM_VERSION_MINOR > 0
    ConstantDataSequential *seq = dyn_cast<ConstantDataSequential>(CPV);

    if (seq) {
      Type *Ty = seq->getElementType();
      if (Ty == Type::getInt1Ty(CPV->getContext())) {
        return processSeqConstant<bool, uint8_t>(seq, index, CONST_INT);
      } else if (Ty == Type::getInt8Ty(CPV->getContext())) {
        return processSeqConstant<uint8_t>(seq, index, CONST_INT);
      } else if (Ty == Type::getInt16Ty(CPV->getContext())) {
        return processSeqConstant<uint16_t>(seq, index, CONST_INT);
      } else if (Ty == Type::getInt32Ty(CPV->getContext())) {
        return processSeqConstant<uint32_t>(seq, index, CONST_INT);
      } else if (Ty == Type::getInt64Ty(CPV->getContext())) {
        return processSeqConstant<uint64_t>(seq, index, CONST_INT);
      } else if (Ty == Type::getFloatTy(CPV->getContext())) {
        return processSeqConstant<float>(seq, index, CONST_FLOAT);
      } else if (Ty == Type::getDoubleTy(CPV->getContext())) {
        return processSeqConstant<double>(seq, index, CONST_DOUBLE);
      }
    } else
#endif /* LLVM_VERSION_MINOR > 0 */

    if (dyn_cast<ConstantAggregateZero>(CPV)) {
      Type* Ty = CPV->getType();
      if(Ty->isVectorTy())
        Ty = (cast<VectorType>(Ty))->getElementType();
      if (Ty == Type::getInt1Ty(CPV->getContext())) {
        const bool b = 0;
        return ctx.newImmediate(b);
      } else if (Ty == Type::getInt8Ty(CPV->getContext())) {
        const uint8_t u8 = 0;
        return ctx.newImmediate(u8);
      } else if (Ty == Type::getInt16Ty(CPV->getContext())) {
        const uint16_t u16 = 0;
        return ctx.newImmediate(u16);
      } else if (Ty == Type::getInt32Ty(CPV->getContext())) {
        const uint32_t u32 = 0;
        return ctx.newImmediate(u32);
      } else if (Ty == Type::getInt64Ty(CPV->getContext())) {
        const uint64_t u64 = 0;
        return ctx.newImmediate(u64);
      } else if (Ty == Type::getFloatTy(CPV->getContext())) {
        const float f32 = 0;
        return ctx.newImmediate(f32);
      } else if (Ty == Type::getDoubleTy(CPV->getContext())) {
        const double f64 = 0;
        return ctx.newImmediate(f64);
      } else {
        GBE_ASSERTM(false, "Unsupporte aggregate zero type.");
        return ctx.newImmediate(uint32_t(0));
      }
    } else {
      if (dyn_cast<ConstantVector>(CPV))
        return processConstantVector(dyn_cast<ConstantVector>(CPV), index);
      GBE_ASSERTM(dyn_cast<ConstantExpr>(CPV) == NULL, "Unsupported constant expression");

      // Integers
      if (ConstantInt *CI = dyn_cast<ConstantInt>(CPV)) {
        Type* Ty = CI->getType();
        if (Ty == Type::getInt1Ty(CPV->getContext())) {
          const bool b = CI->getZExtValue();
          return ctx.newImmediate(b);
        } else if (Ty == Type::getInt8Ty(CPV->getContext())) {
          const uint8_t u8 = CI->getZExtValue();
          return ctx.newImmediate(u8);
        } else if (Ty == Type::getInt16Ty(CPV->getContext())) {
          const uint16_t u16 = CI->getZExtValue();
          return ctx.newImmediate(u16);
        } else if (Ty == Type::getInt32Ty(CPV->getContext())) {
          const uint32_t u32 = CI->getZExtValue();
          return ctx.newImmediate(u32);
        } else if (Ty == Type::getInt64Ty(CPV->getContext())) {
          const uint64_t u64 = CI->getZExtValue();
          return ctx.newImmediate(u64);
        } else {
          if (CI->getValue().getActiveBits() > 64) {
            ctx.getUnit().setValid(false);
            return ctx.newImmediate(uint64_t(0));
          }
          return ctx.newImmediate(uint64_t(CI->getZExtValue()));
        }
      }

      // NULL pointers
      if(isa<ConstantPointerNull>(CPV)) {
        return ctx.newImmediate(uint32_t(0));
      }

      const Type::TypeID typeID = CPV->getType()->getTypeID();
      if (isa<UndefValue>(CPV)) {
        Type* Ty = CPV->getType();
        if (Ty == Type::getInt1Ty(CPV->getContext())) return ctx.newImmediate(false);
        if (Ty == Type::getInt8Ty(CPV->getContext())) return ctx.newImmediate((uint8_t)0);
        if (Ty == Type::getInt16Ty(CPV->getContext())) return ctx.newImmediate((uint16_t)0);
        if (Ty == Type::getInt32Ty(CPV->getContext())) return ctx.newImmediate((uint32_t)0);
        if (Ty == Type::getInt64Ty(CPV->getContext())) return ctx.newImmediate((uint64_t)0);
        if (Ty == Type::getFloatTy(CPV->getContext())) return ctx.newImmediate((float)0);
        if (Ty == Type::getDoubleTy(CPV->getContext())) return ctx.newImmediate((double)0);
        GBE_ASSERT(0 && "Unsupported undef value type.\n");
      }

      // Floats and doubles
      switch (typeID) {
        case Type::FloatTyID:
        case Type::DoubleTyID:
        {
          ConstantFP *FPC = cast<ConstantFP>(CPV);
          GBE_ASSERT(isa<UndefValue>(CPV) == false);

          if (FPC->getType() == Type::getFloatTy(CPV->getContext())) {
            const float f32 = FPC->getValueAPF().convertToFloat();
            return ctx.newImmediate(f32);
          } else {
            const double f64 = FPC->getValueAPF().convertToDouble();
            return ctx.newImmediate(f64);
          }
        }
        break;
        default:
          GBE_ASSERTM(false, "Unsupported constant type");
          break;
      }
    }

    GBE_ASSERTM(false, "Unsupported constant type");
    return ctx.newImmediate(uint64_t(0));
  }

  ir::ImmediateIndex GenWriter::processConstantImmIndex(Constant *CPV, int32_t index) {
    if (dyn_cast<ConstantExpr>(CPV) == NULL)
      return processConstantImmIndexImpl(CPV, index);

    if (dyn_cast<ConstantExpr>(CPV)) {
      ConstantExpr *ce = dyn_cast<ConstantExpr>(CPV);
      ir::Type type = getType(ctx, ce->getType());
      switch (ce->getOpcode()) {
        default:
          //ce->dump();
          GBE_ASSERT(0 && "unsupported ce opcode.\n");
        case Instruction::Trunc:
        {
          const ir::ImmediateIndex immIndex = processConstantImmIndex(ce->getOperand(0), -1);
          return ctx.processImm(ir::IMM_TRUNC, immIndex, type);
        }
        case Instruction::BitCast:
        {
          const ir::ImmediateIndex immIndex = processConstantImmIndex(ce->getOperand(0), -1);
          if (type == ir::TYPE_LARGE_INT)
            return immIndex;
          return ctx.processImm(ir::IMM_BITCAST, immIndex, type);
        }
        case Instruction::Add:
        case Instruction::Sub:
        case Instruction::Mul:
        case Instruction::SDiv:
        case Instruction::SRem:
        case Instruction::Shl:
        case Instruction::AShr:
        case Instruction::LShr:
        case Instruction::And:
        case Instruction::Or:
        case Instruction::Xor: {
          const ir::ImmediateIndex lhs  = processConstantImmIndex(ce->getOperand(0), -1);
          const ir::ImmediateIndex rhs  = processConstantImmIndex(ce->getOperand(1), -1);
          switch (ce->getOpcode()) {
          default:
            //ce->dump();
            GBE_ASSERTM(0, "Unsupported constant expression.\n");
          case Instruction::Add:
            return ctx.processImm(ir::IMM_ADD, lhs, rhs, type);
          case Instruction::Sub:
            return ctx.processImm(ir::IMM_SUB, lhs, rhs, type);
          case Instruction::Mul:
            return ctx.processImm(ir::IMM_MUL, lhs, rhs, type);
          case Instruction::SDiv:
            return ctx.processImm(ir::IMM_DIV, lhs, rhs, type);
          case Instruction::SRem:
            return ctx.processImm(ir::IMM_REM, lhs, rhs, type);
          case Instruction::Shl:
            return ctx.processImm(ir::IMM_SHL, lhs, rhs, type);
          case Instruction::AShr:
            return ctx.processImm(ir::IMM_ASHR, lhs, rhs, type);
          case Instruction::LShr:
            return ctx.processImm(ir::IMM_LSHR, lhs, rhs, type);
          case Instruction::And:
            return ctx.processImm(ir::IMM_AND, lhs, rhs, type);
          case Instruction::Or:
            return ctx.processImm(ir::IMM_OR, lhs, rhs, type);
          case Instruction::Xor:
            return ctx.processImm(ir::IMM_XOR, lhs, rhs, type);
          }
        }
      }
    }
    GBE_ASSERT(0 && "unsupported constant.\n");
    return ctx.newImmediate((uint32_t)0);
  }

  const ir::Immediate &GenWriter::processConstantImm(Constant *CPV, int32_t index) {
    ir::ImmediateIndex immIndex = processConstantImmIndex(CPV, index);
    return ctx.getFunction().getImmediate(immIndex);
  }

  ir::ImmediateIndex GenWriter::newImmediate(Constant *CPV, uint32_t index) {
    return processConstantImmIndex(CPV, index);
  }

  void GenWriter::newRegister(Value *value, Value *key, bool uniform) {
    auto type = value->getType();
    auto typeID = type->getTypeID();
    switch (typeID) {
      case Type::IntegerTyID:
      case Type::FloatTyID:
      case Type::DoubleTyID:
      case Type::PointerTyID:
        regTranslator.newScalar(value, key, 0, uniform);
        break;
      case Type::VectorTyID:
      {
        auto vectorType = cast<VectorType>(type);
        const uint32_t elemNum = vectorType->getNumElements();
        for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
          regTranslator.newScalar(value, key, elemID, uniform);
        break;
      }
      default: NOT_SUPPORTED;
    };
  }

  ir::Register GenWriter::getConstantPointerRegister(ConstantExpr *expr, uint32_t elemID) {
    Value* val = expr->getOperand(0);

    if (expr->isCast()) {
      ir::Register pointer_reg;
      if(isa<ConstantExpr>(val)) {
        // try to get the real pointer register, for case like:
        // store i64 ptrtoint (i8 addrspace(3)* getelementptr inbounds ...
        // in which ptrtoint and getelementptr are ConstantExpr.
        pointer_reg = getConstantPointerRegister(dyn_cast<ConstantExpr>(val), elemID);
      } else {
        pointer_reg = regTranslator.getScalar(val, elemID);
      }
      // if ptrToInt request another type other than 32bit, convert as requested
      ir::Type dstType = getType(ctx, expr->getType());
      ir::Type srcType = getType(ctx, val->getType());
      if(srcType != dstType && dstType != ir::TYPE_S32) {
        ir::Register tmp = ctx.reg(getFamily(dstType));
        ctx.CVT(dstType, srcType, tmp, pointer_reg);
        return tmp;
      }
      return pointer_reg;
    }
    else if (expr->getOpcode() == Instruction::GetElementPtr) {
      uint32_t TypeIndex;
      uint32_t constantOffset = 0;

      Value *pointer = val;
      CompositeType* CompTy = cast<CompositeType>(pointer->getType());
      for(uint32_t op=1; op<expr->getNumOperands(); ++op) {
        uint32_t offset = 0;
        ConstantInt* ConstOP = dyn_cast<ConstantInt>(expr->getOperand(op));
        GBE_ASSERT(ConstOP);
        TypeIndex = ConstOP->getZExtValue();
        if (op == 1) {
          if (TypeIndex != 0) {
            Type *elementType = (cast<PointerType>(pointer->getType()))->getElementType();
            uint32_t elementSize = getTypeByteSize(unit, elementType);
            uint32_t align = getAlignmentByte(unit, elementType);
            elementSize += getPadding(elementSize, align);
            offset += elementSize * TypeIndex;
          }
        } else {
          for(uint32_t ty_i=0; ty_i<TypeIndex; ty_i++)
          {
            Type* elementType = CompTy->getTypeAtIndex(ty_i);
            uint32_t align = getAlignmentByte(unit, elementType);
            offset += getPadding(offset, align);
            offset += getTypeByteSize(unit, elementType);
          }
          const uint32_t align = getAlignmentByte(unit, CompTy->getTypeAtIndex(TypeIndex));
          offset += getPadding(offset, align);
        }

        constantOffset += offset;
        CompTy = dyn_cast<CompositeType>(CompTy->getTypeAtIndex(TypeIndex));
      }

      ir::Register pointer_reg;
      if(isa<ConstantExpr>(pointer))
        pointer_reg = getConstantPointerRegister(dyn_cast<ConstantExpr>(pointer), elemID);
      else
        pointer_reg = regTranslator.getScalar(pointer, elemID);

      ir::Register offset_reg = ctx.reg(ir::RegisterFamily::FAMILY_DWORD);
      ctx.LOADI(ir::Type::TYPE_S32, offset_reg, ctx.newIntegerImmediate(constantOffset, ir::Type::TYPE_S32));
      ir::Register reg = ctx.reg(ir::RegisterFamily::FAMILY_DWORD);
      ctx.ADD(ir::Type::TYPE_S32, reg, pointer_reg, offset_reg);
      return reg;
    }
    else
      assert(0);
  }

  ir::Register GenWriter::getConstantRegister(Constant *c, uint32_t elemID) {
    GBE_ASSERT(c != NULL);
    if(isa<GlobalValue>(c)) {
      return regTranslator.getScalar(c, elemID);
    }
    if(isa<UndefValue>(c)) {
      Type* llvmType = c->getType();
      ir::Type dstType = getType(ctx, llvmType);
      ir::Register reg = ctx.reg(getFamily(dstType));

      ir::ImmediateIndex immIndex;
      if(llvmType->isIntegerTy())
        immIndex = ctx.newIntegerImmediate(0, dstType);
      else if(llvmType->isFloatTy()) {
        immIndex = ctx.newFloatImmediate((float)0.0);
      } else {
        immIndex = ctx.newDoubleImmediate((double)0.0);
      }
      ctx.LOADI(dstType, reg, immIndex);
      return reg;
    }

    if(isa<ConstantExpr>(c)) {
      // Check whether this is a constant drived from a pointer.
      Constant *itC = c;
      while(isa<ConstantExpr>(itC))
        itC = dyn_cast<ConstantExpr>(itC)->getOperand(0);
      if (itC->getType()->isPointerTy())
        return getConstantPointerRegister(dyn_cast<ConstantExpr>(c), elemID);
    }

    const ir::ImmediateIndex immIndex = this->newImmediate(c, elemID);
    const ir::Immediate imm = ctx.getImmediate(immIndex);
    const ir::Register reg = ctx.reg(getFamily(imm.getType()));
    ctx.LOADI(imm.getType(), reg, immIndex);
    return reg;
  }

  ir::Register GenWriter::getRegister(Value *value, uint32_t elemID) {
    //the real value may be constant, so get real value before constant check
    regTranslator.getRealValue(value, elemID);
    if(isa<Constant>(value)) {
      Constant *c = dyn_cast<Constant>(value);
      return getConstantRegister(c, elemID);
    } else
      return regTranslator.getScalar(value, elemID);
  }

  INLINE Value *GenWriter::getPHICopy(Value *PHI) {
    const uintptr_t ptr = (uintptr_t) PHI;
    return (Value*) (ptr+1);
  }

  void GenWriter::newLabelIndex(const BasicBlock *bb) {
    if (labelMap.find(bb) == labelMap.end()) {
      const ir::LabelIndex label = ctx.label();
      labelMap[bb] = label;
    }
  }

  void GenWriter::simplifyTerminator(BasicBlock *bb) {
    Value *value = --bb->end();
    BranchInst *I = NULL;
    if ((I = dyn_cast<BranchInst>(value)) != NULL) {
      if (I->isConditional() == false)
        return;
      // If the "taken" successor is the next block, we try to invert the
      // branch.
      BasicBlock *succ = I->getSuccessor(0);
      if (std::next(Function::iterator(bb)) != Function::iterator(succ))
        return;

      // More than one use is too complicated: we skip it
      Value *condition = I->getCondition();
      if (condition->hasOneUse() == false)
        return;

      // Right now, we only invert comparison instruction
      ICmpInst *CI = dyn_cast<ICmpInst>(condition);
      if (CI != NULL) {
        GBE_ASSERT(conditionSet.find(CI) == conditionSet.end());
        conditionSet.insert(CI);
        return;
      }
    }
  }

  void GenWriter::emitBasicBlock(BasicBlock *BB) {
    GBE_ASSERT(labelMap.find(BB) != labelMap.end());
    ctx.LABEL(labelMap[BB]);
    for (auto II = BB->begin(), E = BB->end(); II != E; ++II) visit(*II);
  }

  void GenWriter::emitMovForPHI(BasicBlock *curr, BasicBlock *succ) {
    for (BasicBlock::iterator I = succ->begin(); isa<PHINode>(I); ++I) {
      PHINode *PN = cast<PHINode>(I);
      Value *IV = PN->getIncomingValueForBlock(curr);
      Type *llvmType = PN->getType();
      const ir::Type type = getType(ctx, llvmType);
      Value *PHICopy = this->getPHICopy(PN);
      const ir::Register dst = this->getRegister(PHICopy);
      if (!isa<UndefValue>(IV)) {

        // Emit the MOV required by the PHI function. We do it simple and do not
        // try to optimize them. A next data flow analysis pass on the Gen IR
        // will remove them
        Constant *CP = dyn_cast<Constant>(IV);
        if (CP) {
          GBE_ASSERT(isa<GlobalValue>(CP) == false);
          ConstantVector *CPV = dyn_cast<ConstantVector>(CP);
          if (CPV && dyn_cast<ConstantVector>(CPV) &&
              isa<UndefValue>(extractConstantElem(CPV, 0)))
            continue;
          ctx.MOV(type, dst, getRegister(CP));
        } else if (regTranslator.valueExists(IV,0) || dyn_cast<Constant>(IV)) {
          const ir::Register src = this->getRegister(IV);
          ctx.MOV(type, dst, src);
        }
        assert(!ctx.getBlock()->undefPhiRegs.contains(dst));
        ctx.getBlock()->definedPhiRegs.insert(dst);
      } else {
        // If this is an undefined value, we don't need emit phi copy here.
        // But we need to record it. As latter, at liveness's backward analysis,
        // we don't need to pass the phi value/register to this BB which the phi
        // value is undefined. Otherwise, the phi value's liveness will be extent
        // incorrectly and may be extent to the basic block zero which is really bad.
        ctx.getBlock()->undefPhiRegs.insert(dst);
      }
    }
  }

  void GenWriter::emitFunctionPrototype(Function &F)
  {
    GBE_ASSERTM(F.hasStructRetAttr() == false,
                "Returned value for kernel functions is forbidden");

    // Loop over the kernel metadatas to set the required work group size.
    NamedMDNode *clKernelMetaDatas = TheModule->getNamedMetadata("opencl.kernels");
    size_t reqd_wg_sz[3] = {0, 0, 0};
    size_t hint_wg_sz[3] = {0, 0, 0};
    ir::FunctionArgument::InfoFromLLVM llvmInfo;
    MDNode *node = NULL;
    MDNode *addrSpaceNode = NULL;
    MDNode *typeNameNode = NULL;
    MDNode *accessQualNode = NULL;
    MDNode *typeQualNode = NULL;
    MDNode *argNameNode = NULL;

    std::string functionAttributes;

    /* First find the meta data belong to this function. */
    for(uint i = 0; i < clKernelMetaDatas->getNumOperands(); i++) {
      node = clKernelMetaDatas->getOperand(i);
      if (node->getOperand(0) == &F) break;
      node = NULL;
    }

    /* because "-cl-kernel-arg-info", should always have meta data. */
    if (!F.arg_empty())
      assert(node);


    for(uint j = 0; j < node->getNumOperands() - 1; j++) {
      MDNode *attrNode = dyn_cast_or_null<MDNode>(node->getOperand(1 + j));
      if (attrNode == NULL) break;
      MDString *attrName = dyn_cast_or_null<MDString>(attrNode->getOperand(0));
      if (!attrName) continue;

      if (attrName->getString() == "reqd_work_group_size") {
        GBE_ASSERT(attrNode->getNumOperands() == 4);
        ConstantInt *x = dyn_cast<ConstantInt>(attrNode->getOperand(1));
        ConstantInt *y = dyn_cast<ConstantInt>(attrNode->getOperand(2));
        ConstantInt *z = dyn_cast<ConstantInt>(attrNode->getOperand(3));
        GBE_ASSERT(x && y && z);
        reqd_wg_sz[0] = x->getZExtValue();
        reqd_wg_sz[1] = y->getZExtValue();
        reqd_wg_sz[2] = z->getZExtValue();
        functionAttributes += attrName->getString();
        std::stringstream param;
        char buffer[100];
        param <<"(";
        param << reqd_wg_sz[0];
        param << ",";
        param << reqd_wg_sz[1];
        param << ",";
        param << reqd_wg_sz[2];
        param <<")";
        param >> buffer;
        functionAttributes += buffer;
        functionAttributes += " ";
        break;
      } else if (attrName->getString() == "kernel_arg_addr_space") {
        addrSpaceNode = attrNode;
      } else if (attrName->getString() == "kernel_arg_access_qual") {
        accessQualNode = attrNode;
      } else if (attrName->getString() == "kernel_arg_type") {
        typeNameNode = attrNode;
      } else if (attrName->getString() == "kernel_arg_type_qual") {
        typeQualNode = attrNode;
      } else if (attrName->getString() == "kernel_arg_name") {
        argNameNode = attrNode;
      } else if (attrName->getString() == "vec_type_hint") {
        GBE_ASSERT(attrNode->getNumOperands() == 3);
        functionAttributes += attrName->getString();
        functionAttributes += " ";
      } else if (attrName->getString() == "work_group_size_hint") {
        GBE_ASSERT(attrNode->getNumOperands() == 4);
        ConstantInt *x = dyn_cast<ConstantInt>(attrNode->getOperand(1));
        ConstantInt *y = dyn_cast<ConstantInt>(attrNode->getOperand(2));
        ConstantInt *z = dyn_cast<ConstantInt>(attrNode->getOperand(3));
        GBE_ASSERT(x && y && z);
        hint_wg_sz[0] = x->getZExtValue();
        hint_wg_sz[1] = y->getZExtValue();
        hint_wg_sz[2] = z->getZExtValue();
        functionAttributes += attrName->getString();
        std::stringstream param;
        char buffer[100];
        param <<"(";
        param << hint_wg_sz[0];
        param << ",";
        param << hint_wg_sz[1];
        param << ",";
        param << hint_wg_sz[2];
        param <<")";
        param >> buffer;
        functionAttributes += buffer;
        functionAttributes += " ";
      }
    }
    ctx.appendSurface(1, ir::ocl::stackbuffer);

    ctx.getFunction().setCompileWorkGroupSize(reqd_wg_sz[0], reqd_wg_sz[1], reqd_wg_sz[2]);

    ctx.getFunction().setFunctionAttributes(functionAttributes);
    // Loop over the arguments and output registers for them
    if (!F.arg_empty()) {
      uint32_t argID = 0;
      Function::arg_iterator I = F.arg_begin(), E = F.arg_end();

      // Insert a new register for each function argument
#if LLVM_VERSION_MINOR <= 1
      const AttrListPtr &PAL = F.getAttributes();
#endif /* LLVM_VERSION_MINOR <= 1 */
      for (; I != E; ++I, ++argID) {
        const std::string &argName = I->getName().str();
        Type *type = I->getType();

        llvmInfo.addrSpace = (cast<ConstantInt>(addrSpaceNode->getOperand(1 + argID)))->getZExtValue();
        llvmInfo.typeName = (cast<MDString>(typeNameNode->getOperand(1 + argID)))->getString();
        if (llvmInfo.typeName.find("image") != std::string::npos &&
            llvmInfo.typeName.find("*") != std::string::npos) {
          uint32_t start = llvmInfo.typeName.find("image");
          uint32_t end = llvmInfo.typeName.find("*");
          llvmInfo.typeName = llvmInfo.typeName.substr(start, end - start);
        }
        llvmInfo.accessQual = (cast<MDString>(accessQualNode->getOperand(1 + argID)))->getString();
        llvmInfo.typeQual = (cast<MDString>(typeQualNode->getOperand(1 + argID)))->getString();
        llvmInfo.argName = (cast<MDString>(argNameNode->getOperand(1 + argID)))->getString();

        // function arguments are uniform values.
        this->newRegister(I, NULL, true);
        // add support for vector argument.
        if(type->isVectorTy()) {
          VectorType *vectorType = cast<VectorType>(type);
          ir::Register reg = getRegister(I, 0);
          Type *elemType = vectorType->getElementType();
          const uint32_t elemSize = getTypeByteSize(unit, elemType);
          const uint32_t elemNum = vectorType->getNumElements();
          //vector's elemType always scalar type
          ctx.input(argName, ir::FunctionArgument::VALUE, reg, llvmInfo, elemNum*elemSize, getAlignmentByte(unit, type), 0);

          ir::Function& fn = ctx.getFunction();
          for(uint32_t i=1; i < elemNum; i++) {
            ir::PushLocation argLocation(fn, argID, elemSize*i);
            reg = getRegister(I, i);
            ctx.appendPushedConstant(reg, argLocation);  //add to push map for reg alloc
          }
          continue;
        }

        GBE_ASSERTM(isScalarType(type) == true,
                    "vector type in the function argument is not supported yet");
        const ir::Register reg = getRegister(I);
        if (type->isPointerTy() == false)
          ctx.input(argName, ir::FunctionArgument::VALUE, reg, llvmInfo, getTypeByteSize(unit, type), getAlignmentByte(unit, type), 0);
        else {
          PointerType *pointerType = dyn_cast<PointerType>(type);
          Type *pointed = pointerType->getElementType();
          // By value structure
#if LLVM_VERSION_MINOR <= 1
          if (PAL.paramHasAttr(argID+1, Attribute::ByVal)) {
#else
          if (I->hasByValAttr()) {
#endif /* LLVM_VERSION_MINOR <= 1 */
            const size_t structSize = getTypeByteSize(unit, pointed);
            ctx.input(argName, ir::FunctionArgument::STRUCTURE, reg, llvmInfo, structSize, getAlignmentByte(unit, type), 0);
          }
          // Regular user provided pointer (global, local or constant)
          else {
            const uint32_t addr = pointerType->getAddressSpace();
            const ir::AddressSpace addrSpace = addressSpaceLLVMToGen(addr);
            const uint32_t ptrSize = getTypeByteSize(unit, type);
            const uint32_t align = getAlignmentByte(unit, pointed);
              switch (addrSpace) {
              case ir::MEM_GLOBAL:
                globalPointer.insert(std::make_pair(I, btiBase));
                ctx.appendSurface(btiBase, reg);
                ctx.input(argName, ir::FunctionArgument::GLOBAL_POINTER, reg, llvmInfo, ptrSize, align, btiBase);
                incBtiBase();
              break;
              case ir::MEM_LOCAL:
                ctx.input(argName, ir::FunctionArgument::LOCAL_POINTER, reg,  llvmInfo, ptrSize, align, 0xfe);
                ctx.getFunction().setUseSLM(true);
              break;
              case ir::MEM_CONSTANT:
                ctx.input(argName, ir::FunctionArgument::CONSTANT_POINTER, reg,  llvmInfo, ptrSize, align, 0x2);
              break;
              case ir::IMAGE:
                ctx.input(argName, ir::FunctionArgument::IMAGE, reg, llvmInfo, ptrSize, align, 0x0);
                ctx.getFunction().getImageSet()->append(reg, &ctx, incBtiBase());
              break;
              default: GBE_ASSERT(addrSpace != ir::MEM_PRIVATE);
            }
          }
        }
      }
    }

    // When returning a structure, first input register is the pointer to the
    // structure
#if GBE_DEBUG
    const Type *type = F.getReturnType();
    GBE_ASSERTM(type->isVoidTy() == true,
                "Returned value for kernel functions is forbidden");

    // Variable number of arguments is not supported
    FunctionType *FT = cast<FunctionType>(F.getFunctionType());
    GBE_ASSERT(FT->isVarArg() == false);
#endif /* GBE_DEBUG */
  }

  static inline bool isFPIntBitCast(const Instruction &I) {
    if (!isa<BitCastInst>(I))
      return false;
    Type *SrcTy = I.getOperand(0)->getType();
    Type *DstTy = I.getType();
    return (SrcTy->isFloatingPointTy() && DstTy->isIntegerTy()) ||
           (DstTy->isFloatingPointTy() && SrcTy->isIntegerTy());
  }

  /*! To track last read and write of the registers */
  struct RegInfoForMov {
    ir::Instruction *lastWriteInsn;
    ir::Instruction *lastReadInsn;
    uint32_t lastWrite;
    uint32_t lastRead;
  };

  /*! Replace register "from" by register "to" in the destination(s) */
  static void replaceDst(ir::Instruction *insn, ir::Register from, ir::Register to) {
    const uint32_t dstNum = insn->getDstNum();
    for (uint32_t dstID = 0; dstID < dstNum; ++dstID)
      if (insn->getDst(dstID) == from)
        insn->setDst(dstID, to);
  }

  /*! Replace register "from" by register "to" in the source(s) */
  static void replaceSrc(ir::Instruction *insn, ir::Register from, ir::Register to) {
    const uint32_t srcNum = insn->getSrcNum();
    for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
      if (insn->getSrc(srcID) == from)
        insn->setSrc(srcID, to);
  }

  /*! lastUse maintains data about last uses (reads/writes) for each
   * ir::Register
   */
  static void buildRegInfo(ir::BasicBlock &bb, vector<RegInfoForMov> &lastUse)
  {
    // Clear the register usages
    for (auto &x : lastUse) {
      x.lastWrite = x.lastRead = 0;
      x.lastWriteInsn = x.lastReadInsn = NULL;
    }

    // Find use intervals for all registers (distinguish sources and
    // destinations)
    uint32_t insnID = 2;
    bb.foreach([&](ir::Instruction &insn) {
      const uint32_t dstNum = insn.getDstNum();
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const ir::Register reg = insn.getSrc(srcID);
        lastUse[reg].lastRead = insnID;
        lastUse[reg].lastReadInsn = &insn;
      }
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        const ir::Register reg = insn.getDst(dstID);
        lastUse[reg].lastWrite = insnID+1;
        lastUse[reg].lastWriteInsn = &insn;
      }
      insnID+=2;
    });
  }

  void GenWriter::optimizePhiCopy(ir::Liveness &liveness, ir::Function &fn)
  {
    // The overall idea behind is we check whether there is any interference
    // between phi and phiCopy live range. If there is no point that
    // phi & phiCopy are both alive, then we can optimize off the move
    // from phiCopy to phi, and use phiCopy directly instead of phi.
    using namespace ir;
    ir::FunctionDAG *dag = new ir::FunctionDAG(liveness);

    for (auto &it : phiMap) {
      const Register phi = it.first;
      const Register phiCopy = it.second;

      const ir::DefSet *phiCopyDef = dag->getRegDef(phiCopy);
      const ir::UseSet *phiUse = dag->getRegUse(phi);
      const DefSet *phiDef = dag->getRegDef(phi);
      bool isOpt = true;
      for (auto &x : *phiCopyDef) {
        const ir::Instruction * phiCopyDefInsn = x->getInstruction();
        const ir::BasicBlock *bb = phiCopyDefInsn->getParent();
        const Liveness::LiveOut &out = liveness.getLiveOut(bb);
        // phi & phiCopy are both alive at the endpoint of bb,
        // thus can not be optimized.
        if (out.contains(phi)) {
          isOpt = false;
          break;
        }
        // If phi is used in the same BB that define the phiCopy,
        // we need carefully check the liveness of phi & phiCopy.
        // Make sure their live ranges do not interfere.
        bool phiUsedInSameBB = false;
        for (auto &y : *phiUse) {
          const ir::Instruction *phiUseInsn = y->getInstruction();
          const ir::BasicBlock *bb2 = phiUseInsn->getParent();
          if (bb2 == bb) {
            phiUsedInSameBB = true;
          }
        }
        // Check phi is not used between phiCopy def point and bb's end point,
        // which is often referred as 'phi swap issue', just like below:
        //   MOV phiCopy_1, x;
        //   MOV phiCopy_2, phi_1;
        if (phiUsedInSameBB ) {
          for (auto it = --bb->end(); it != bb->end() ; --it) {
            const Instruction &p = *it;

            if (&p == phiCopyDefInsn) break;
            // we only care MOV here
            if (p.getSrcNum() == 1 && p.getSrc(0) == phi) {
              isOpt = false;
              break;
            }
          }
        }
      }

      // [MOV phi, phiCopy;] can be removed. So we remove it
      // and replace phi uses with phiCopy
      if (isOpt) {
        for (auto &x : *phiDef) {
          const_cast<Instruction *>(x->getInstruction())->remove();
        }
        for (auto &x : *phiUse) {
          const Instruction *phiUseInsn = x->getInstruction();
          replaceSrc(const_cast<Instruction *>(phiUseInsn), phi, phiCopy);
        }
      }
    }
    delete dag;
  }

  void GenWriter::removeMOVs(const ir::Liveness &liveness, ir::Function &fn)
  {
    // We store the last write and last read for each register
    const uint32_t regNum = fn.regNum();
    vector<RegInfoForMov> lastUse;
    lastUse.resize(regNum);

    // Remove the MOVs per block (local analysis only) Note that we do not try
    // to remove MOV for variables that outlives the block. So we use liveness
    // information to figure out which variable is alive
    fn.foreachBlock([&](ir::BasicBlock &bb)
    {
      // We need to know when each register will be read or written
      buildRegInfo(bb, lastUse);

      // Liveinfo helps us to know if the source outlives the block
      const ir::Liveness::BlockInfo &info = liveness.getBlockInfo(&bb);

      auto it = --bb.end();
      if (it->isMemberOf<ir::BranchInstruction>() == true) --it;
      for (auto it = --bb.end(); it != bb.end();) {
        ir::Instruction *insn = &*it; it--;
        const ir::Opcode op = insn->getOpcode();
        if (op == ir::OP_MOV) {
          const ir::Register dst = insn->getDst(0);
          const ir::Register src = insn->getSrc(0);
          // Outlives the block. We do not do anything
          if (info.inLiveOut(src))
            continue;
          const RegInfoForMov &dstInfo = lastUse[dst];
          const RegInfoForMov &srcInfo = lastUse[src];
          // The source is not computed in this block
          if (srcInfo.lastWrite == 0)
            continue;
          // dst is read after src is written. We cannot overwrite dst
          if (dstInfo.lastRead > srcInfo.lastWrite)
            continue;
          // We are good. We first patch the destination then all the sources
          replaceDst(srcInfo.lastWriteInsn, src, dst);
          // Then we patch all subsequent uses of the source
          ir::Instruction *next = static_cast<ir::Instruction*>(srcInfo.lastWriteInsn->next);
          while (next != insn) {
            replaceSrc(next, src, dst);
            next = static_cast<ir::Instruction*>(next->next);
          }
          insn->remove();
        } else if (op == ir::OP_LOADI)
          continue;
        else
          break;
      }
    });
  }

  void GenWriter::removeLOADIs(const ir::Liveness &liveness, ir::Function &fn)
  {
    // We store the last write and last read for each register
    const uint32_t regNum = fn.regNum();
    vector<RegInfoForMov> lastUse;
    lastUse.resize(regNum);

    // Traverse all blocks and remove redundant immediates. Do *not* remove
    // immediates that outlive the block
    fn.foreachBlock([&](ir::BasicBlock &bb)
    {
      // Each immediate that is already loaded in the block
      map<ir::Immediate, ir::Register> loadedImm;

      // Immediate to immediate translation
      map<ir::Register, ir::Register> immTranslate;

      // Liveinfo helps us to know if the loaded immediate outlives the block
      const ir::Liveness::BlockInfo &info = liveness.getBlockInfo(&bb);

      // We need to know when each register will be read or written
      buildRegInfo(bb, lastUse);

      // Top bottom traversal -> remove useless LOADIs
      uint32_t insnID = 2;
      bb.foreach([&](ir::Instruction &insn)
      {
        // We either try to remove the LOADI or we will try to use it as a
        // replacement for the next same LOADIs
        if (insn.isMemberOf<ir::LoadImmInstruction>()) {
          ir::LoadImmInstruction &loadImm = cast<ir::LoadImmInstruction>(insn);
          const ir::Immediate imm = loadImm.getImmediate();
          const ir::Register dst = loadImm.getDst(0);

          // Not here: cool, we put it in the map if the register is not
          // overwritten. If it is, we just ignore it for simplicity. Note that
          // it should not happen with the way we "unSSA" the code
          auto it = loadedImm.find(imm);
          auto end = loadedImm.end();
          if (it == end && lastUse[dst].lastWrite == insnID+1)
            loadedImm.insert(std::make_pair(imm, dst));
          // We already pushed the same immediate and we do not outlive the
          // block. We are good to replace this immediate by the previous one
          else if (it != end && info.inLiveOut(dst) == false) {
            immTranslate.insert(std::make_pair(dst, it->second));
            insn.remove();
          }
        }
        // Traverse all the destinations and sources and perform the
        // substitutions (if any)
        else {
          const uint32_t srcNum = insn.getSrcNum();
          const uint32_t dstNum = insn.getDstNum();
          for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
            const ir::Register src = insn.getSrc(srcID);
            auto it = immTranslate.find(src);
            if (it != immTranslate.end())
              insn.setSrc(srcID, it->second);
          }
          for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
            const ir::Register dst = insn.getDst(dstID);
            auto it = immTranslate.find(dst);
            if (it != immTranslate.end())
              insn.setDst(dstID, it->second);
          }
        }
        insnID += 2;
      });
    });
  }

  BVAR(OCL_OPTIMIZE_PHI_MOVES, true);
  BVAR(OCL_OPTIMIZE_LOADI, true);

  static const Instruction *getInstructionUseLocal(const Value *v) {
    // Local variable can only be used in one kernel function. So, if we find
    // one instruction that use the local variable, simply return.
    const Instruction *insn = NULL;
    for(Value::const_use_iterator iter = v->use_begin(); iter != v->use_end(); ++iter) {
    // After LLVM 3.5, use_iterator points to 'Use' instead of 'User', which is more straightforward.
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 5)
      const User *theUser = *iter;
#else
      const User *theUser = iter->getUser();
#endif
      if(isa<Instruction>(theUser)) return cast<const Instruction>(theUser);
      insn = getInstructionUseLocal(theUser);
      if(insn != NULL) break;
    }
    return insn;
  }

  void GenWriter::allocateGlobalVariableRegister(Function &F)
  {
    // Allocate a address register for each global variable
    const Module::GlobalListType &globalList = TheModule->getGlobalList();
    size_t j = 0;
    for(auto i = globalList.begin(); i != globalList.end(); i ++) {
      const GlobalVariable &v = *i;
      if(!v.isConstantUsed()) continue;

      ir::AddressSpace addrSpace = addressSpaceLLVMToGen(v.getType()->getAddressSpace());
      if(addrSpace == ir::MEM_LOCAL) {
        const Value * val = cast<Value>(&v);
        const Instruction *insn = getInstructionUseLocal(val);
        GBE_ASSERT(insn && "Can't find a valid reference instruction for local variable.");

        const BasicBlock * bb = insn->getParent();
        const Function * func = bb->getParent();
        if(func != &F) continue;

        ir::Function &f = ctx.getFunction();
        f.setUseSLM(true);
        const Constant *c = v.getInitializer();
        Type *ty = c->getType();
        uint32_t oldSlm = f.getSLMSize();
        uint32_t align = 8 * getAlignmentByte(unit, ty);
        uint32_t padding = getPadding(oldSlm*8, align);

        f.setSLMSize(oldSlm + padding/8 + getTypeByteSize(unit, ty));

        this->newRegister(const_cast<GlobalVariable*>(&v));
        ir::Register reg = regTranslator.getScalar(const_cast<GlobalVariable*>(&v), 0);
        ctx.LOADI(ir::TYPE_S32, reg, ctx.newIntegerImmediate(oldSlm + padding/8, ir::TYPE_S32));
      } else if(addrSpace == ir::MEM_CONSTANT) {
        GBE_ASSERT(v.hasInitializer());
        this->newRegister(const_cast<GlobalVariable*>(&v));
        ir::Register reg = regTranslator.getScalar(const_cast<GlobalVariable*>(&v), 0);
        ir::Constant &con = unit.getConstantSet().getConstant(j ++);
        GBE_ASSERT(con.getName() == v.getName());
        ctx.LOADI(ir::TYPE_S32, reg, ctx.newIntegerImmediate(con.getOffset(), ir::TYPE_S32));
      } else {
        if(v.getName().equals(StringRef("__gen_ocl_printf_buf"))) {
          ctx.appendSurface(btiBase, ir::ocl::printfbptr);
          ctx.getFunction().getPrintfSet()->setBufBTI(btiBase);
          globalPointer.insert(std::make_pair(&v, incBtiBase()));
          regTranslator.newScalarProxy(ir::ocl::printfbptr, const_cast<GlobalVariable*>(&v));
        } else if(v.getName().equals(StringRef("__gen_ocl_printf_index_buf"))) {
          ctx.appendSurface(btiBase, ir::ocl::printfiptr);
          ctx.getFunction().getPrintfSet()->setIndexBufBTI(btiBase);
          globalPointer.insert(std::make_pair(&v, incBtiBase()));
          regTranslator.newScalarProxy(ir::ocl::printfiptr, const_cast<GlobalVariable*>(&v));
	} else if(v.getName().str().substr(0, 4) == ".str") {
          /* When there are multi printf statements in multi kernel fucntions within the same
             translate unit, if they have the same sting parameter, such as
             kernel_func1 () {
               printf("Line is %d\n", line_num1);
             }
             kernel_func2 () {
               printf("Line is %d\n", line_num2);
             }
             The Clang will just generate one global string named .strXXX to represent "Line is %d\n"
             So when translating the kernel_func1, we can not unref that global var, so we will
             get here. Just ignore it to avoid assert. */
        } else {
          GBE_ASSERT(0);
        }
      }
    }

  }
  static INLINE void findAllLoops(LoopInfo * LI, std::vector<std::pair<Loop*, int>> &lp)
  {
      for (Loop::reverse_iterator I = LI->rbegin(), E = LI->rend(); I != E; ++I) {
        lp.push_back(std::make_pair(*I, -1));
      }
      if (lp.size() == 0) return;

      uint32_t i = 0;
      do {
        const std::vector<Loop*> subLoops = lp[i].first->getSubLoops();
        for(auto sub : subLoops)
          lp.push_back(std::make_pair(sub, i));
        i++;
      } while(i < lp.size());
  }

  void GenWriter::gatherLoopInfo(ir::Function &fn) {
    vector<ir::LabelIndex> loopBBs;
    vector<std::pair<ir::LabelIndex, ir::LabelIndex>> loopExits;
    std::vector<std::pair<Loop*, int>> lp;

    findAllLoops(LI, lp);
#if GBE_DEBUG
    // check two loops' interference
    for(unsigned int i = 0; i < lp.size(); i++) {
        SmallVector<Loop::Edge, 8> exitBBs;
        lp[i].first->getExitEdges(exitBBs);

      const std::vector<BasicBlock*> &inBBs = lp[i].first->getBlocks();
      std::vector<ir::LabelIndex> bbs1;
      for(auto x : inBBs) {
        bbs1.push_back(labelMap[x]);
      }
      std::sort(bbs1.begin(), bbs1.end());
      for(unsigned int j = i+1; j < lp.size(); j++) {
        if(! lp[i].first->contains(lp[j].first)) {
          const std::vector<BasicBlock*> &inBBs2 = lp[j].first->getBlocks();
          std::vector<ir::LabelIndex> bbs2;
          std::vector<ir::LabelIndex> bbs3;

          for(auto x : inBBs2) {
            bbs2.push_back(labelMap[x]);
          }

          std::sort(bbs2.begin(), bbs2.end());
          std::set_intersection(bbs1.begin(), bbs1.end(), bbs2.begin(), bbs2.end(), std::back_inserter(bbs3));
          GBE_ASSERT(bbs3.size() < 1);
        }
      }
    }
#endif

    for (auto loop : lp) {
      loopBBs.clear();
      loopExits.clear();

      const std::vector<BasicBlock*> &inBBs = loop.first->getBlocks();
      for (auto b : inBBs) {
        GBE_ASSERT(labelMap.find(b) != labelMap.end());
        loopBBs.push_back(labelMap[b]);
      }

      SmallVector<Loop::Edge, 8> exitBBs;
      loop.first->getExitEdges(exitBBs);
      for(auto b : exitBBs){
        GBE_ASSERT(labelMap.find(b.first) != labelMap.end());
        GBE_ASSERT(labelMap.find(b.second) != labelMap.end());
        loopExits.push_back(std::make_pair(labelMap[b.first], labelMap[b.second]));
      }
      fn.addLoop(loopBBs, loopExits);
    }
  }

  void GenWriter::emitFunction(Function &F)
  {
    switch (F.getCallingConv()) {
#if LLVM_VERSION_MINOR <= 2
      case CallingConv::PTX_Device: // we do not emit device function
        return;
      case CallingConv::PTX_Kernel:
#else
      case CallingConv::C:
#endif
        break;
      default: GBE_ASSERTM(false, "Unsupported calling convention");
    }

    ctx.startFunction(F.getName());
    ir::Function &fn = ctx.getFunction();
    this->regTranslator.clear();
    this->labelMap.clear();
    this->emitFunctionPrototype(F);

    this->allocateGlobalVariableRegister(F);
    // Visit all the instructions and emit the IR registers or the value to
    // value mapping when a new register is not needed
    pass = PASS_EMIT_REGISTERS;
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
      visit(*I);

    // First create all the labels (one per block) ...
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      this->newLabelIndex(BB);

    // Then, for all branch instructions that have conditions, see if we can
    // simplify the code by inverting condition code
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      this->simplifyTerminator(BB);

    // gather loop info, which is useful for liveness analysis
    gatherLoopInfo(fn);

    // ... then, emit the instructions for all basic blocks
    pass = PASS_EMIT_INSTRUCTIONS;
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      emitBasicBlock(BB);
    ctx.endFunction();

    // Liveness can be shared when we optimized the immediates and the MOVs
    ir::Liveness liveness(fn);

    if (OCL_OPTIMIZE_LOADI) this->removeLOADIs(liveness, fn);
    if (OCL_OPTIMIZE_PHI_MOVES) this->optimizePhiCopy(liveness, fn);
    if (OCL_OPTIMIZE_PHI_MOVES) this->removeMOVs(liveness, fn);
  }

  void GenWriter::regAllocateReturnInst(ReturnInst &I) {}

  void GenWriter::emitReturnInst(ReturnInst &I) {
    const ir::Function &fn = ctx.getFunction();
    GBE_ASSERTM(fn.outputNum() <= 1, "no more than one value can be returned");
    if (fn.outputNum() == 1 && I.getNumOperands() > 0) {
      const ir::Register dst = fn.getOutput(0);
      const ir::Register src = this->getRegister(I.getOperand(0));
      const ir::RegisterFamily family = fn.getRegisterFamily(dst);
      ctx.MOV(ir::getType(family), dst, src);
    }
    ctx.RET();
  }

  void GenWriter::regAllocateBinaryOperator(Instruction &I) {
    this->newRegister(&I);
  }

  void GenWriter::emitBinaryOperator(Instruction &I) {
#if GBE_DEBUG
    GBE_ASSERT(I.getType()->isPointerTy() == false);
    // We accept logical operations on booleans
    switch (I.getOpcode()) {
      case Instruction::And:
      case Instruction::Or:
      case Instruction::Xor:
        break;
      default:
        GBE_ASSERT(I.getType() != Type::getInt1Ty(I.getContext()));
    }
#endif /* GBE_DEBUG */

    // Get the element type for a vector
    const ir::Type type = getType(ctx, I.getType());

    // Emit the instructions in a row
    const ir::Register dst = this->getRegister(&I);
    const ir::Register src0 = this->getRegister(I.getOperand(0));
    const ir::Register src1 = this->getRegister(I.getOperand(1));

    switch (I.getOpcode()) {
      case Instruction::Add:
      case Instruction::FAdd: ctx.ADD(type, dst, src0, src1); break;
      case Instruction::Sub:
      case Instruction::FSub: ctx.SUB(type, dst, src0, src1); break;
      case Instruction::Mul:
      case Instruction::FMul: ctx.MUL(type, dst, src0, src1); break;
      case Instruction::URem: ctx.REM(getUnsignedType(ctx, I.getType()), dst, src0, src1); break;
      case Instruction::SRem:
      case Instruction::FRem: ctx.REM(type, dst, src0, src1); break;
      case Instruction::UDiv: ctx.DIV(getUnsignedType(ctx, I.getType()), dst, src0, src1); break;
      case Instruction::SDiv:
      case Instruction::FDiv: ctx.DIV(type, dst, src0, src1); break;
      case Instruction::And:  ctx.AND(type, dst, src0, src1); break;
      case Instruction::Or:   ctx.OR(type, dst, src0, src1); break;
      case Instruction::Xor:  ctx.XOR(type, dst, src0, src1); break;
      case Instruction::Shl:  ctx.SHL(type, dst, src0, src1); break;
      case Instruction::LShr: ctx.SHR(getUnsignedType(ctx, I.getType()), dst, src0, src1); break;
      case Instruction::AShr: ctx.ASR(type, dst, src0, src1); break;
      default: NOT_SUPPORTED;
    }
  }

  void GenWriter::regAllocateICmpInst(ICmpInst &I) {
    this->newRegister(&I);
  }

  static ir::Type makeTypeSigned(const ir::Type &type) {
    if (type == ir::TYPE_U8) return ir::TYPE_S8;
    else if (type == ir::TYPE_U16) return ir::TYPE_S16;
    else if (type == ir::TYPE_U32) return ir::TYPE_S32;
    else if (type == ir::TYPE_U64) return ir::TYPE_S64;
    return type;
  }

  static ir::Type makeTypeUnsigned(const ir::Type &type) {
    if (type == ir::TYPE_S8) return ir::TYPE_U8;
    else if (type == ir::TYPE_S16) return ir::TYPE_U16;
    else if (type == ir::TYPE_S32) return ir::TYPE_U32;
    else if (type == ir::TYPE_S64) return ir::TYPE_U64;
    return type;
  }

  void GenWriter::emitICmpInst(ICmpInst &I) {
    GBE_ASSERT(I.getOperand(0)->getType() != Type::getInt1Ty(I.getContext()));

    // Get the element type and the number of elements
    Type *operandType = I.getOperand(0)->getType();
    const ir::Type type = getType(ctx, operandType);
    const ir::Type signedType = makeTypeSigned(type);
    const ir::Type unsignedType = makeTypeUnsigned(type);

    // Emit the instructions in a row
    const ir::Register dst = this->getRegister(&I);
    const ir::Register src0 = this->getRegister(I.getOperand(0));
    const ir::Register src1 = this->getRegister(I.getOperand(1));

    // We must invert the condition to simplify the branch code
    if (conditionSet.find(&I) != conditionSet.end()) {
      switch (I.getPredicate()) {
        case ICmpInst::ICMP_EQ:  ctx.NE(type, dst, src0, src1); break;
        case ICmpInst::ICMP_NE:  ctx.EQ(type, dst, src0, src1); break;
        case ICmpInst::ICMP_ULE: ctx.GT((unsignedType), dst, src0, src1); break;
        case ICmpInst::ICMP_SLE: ctx.GT(signedType, dst, src0, src1); break;
        case ICmpInst::ICMP_UGE: ctx.LT(unsignedType, dst, src0, src1); break;
        case ICmpInst::ICMP_SGE: ctx.LT(signedType, dst, src0, src1); break;
        case ICmpInst::ICMP_ULT: ctx.GE(unsignedType, dst, src0, src1); break;
        case ICmpInst::ICMP_SLT: ctx.GE(signedType, dst, src0, src1); break;
        case ICmpInst::ICMP_UGT: ctx.LE(unsignedType, dst, src0, src1); break;
        case ICmpInst::ICMP_SGT: ctx.LE(signedType, dst, src0, src1); break;
        default: NOT_SUPPORTED;
      }
    }
    // Nothing special to do
    else {
      switch (I.getPredicate()) {
        case ICmpInst::ICMP_EQ:  ctx.EQ(type, dst, src0, src1); break;
        case ICmpInst::ICMP_NE:  ctx.NE(type, dst, src0, src1); break;
        case ICmpInst::ICMP_ULE: ctx.LE((unsignedType), dst, src0, src1); break;
        case ICmpInst::ICMP_SLE: ctx.LE(signedType, dst, src0, src1); break;
        case ICmpInst::ICMP_UGE: ctx.GE(unsignedType, dst, src0, src1); break;
        case ICmpInst::ICMP_SGE: ctx.GE(signedType, dst, src0, src1); break;
        case ICmpInst::ICMP_ULT: ctx.LT(unsignedType, dst, src0, src1); break;
        case ICmpInst::ICMP_SLT: ctx.LT(signedType, dst, src0, src1); break;
        case ICmpInst::ICMP_UGT: ctx.GT(unsignedType, dst, src0, src1); break;
        case ICmpInst::ICMP_SGT: ctx.GT(signedType, dst, src0, src1); break;
        default: NOT_SUPPORTED;
      }
    }
  }

  void GenWriter::regAllocateFCmpInst(FCmpInst &I) {
    this->newRegister(&I);
  }

  void GenWriter::emitFCmpInst(FCmpInst &I) {

    // Get the element type and the number of elements
    Type *operandType = I.getOperand(0)->getType();
    const ir::Type type = getType(ctx, operandType);
    const ir::Type insnType = getType(ctx, I.getType());

    // Emit the instructions in a row
    const ir::Register dst = this->getRegister(&I);
    const ir::Register src0 = this->getRegister(I.getOperand(0));
    const ir::Register src1 = this->getRegister(I.getOperand(1));
    const ir::Register tmp = ctx.reg(getFamily(ctx, I.getType()));
    Value *cv = ConstantInt::get(I.getType(), 1);

    switch (I.getPredicate()) {
      case ICmpInst::FCMP_OEQ: ctx.EQ(type, dst, src0, src1); break;
      case ICmpInst::FCMP_ONE: ctx.NE(type, dst, src0, src1); break;
      case ICmpInst::FCMP_OLE: ctx.LE(type, dst, src0, src1); break;
      case ICmpInst::FCMP_OGE: ctx.GE(type, dst, src0, src1); break;
      case ICmpInst::FCMP_OLT: ctx.LT(type, dst, src0, src1); break;
      case ICmpInst::FCMP_OGT: ctx.GT(type, dst, src0, src1); break;
      case ICmpInst::FCMP_ORD:
        //If there is a constant between src0 and src1, this constant value
        //must ordered, otherwise, llvm will optimize the instruction to ture.
        //So discard this constant value, only compare the other src.
        if(isa<ConstantFP>(I.getOperand(0)))
          ctx.EQ(type, dst, src1, src1);
        else if(isa<ConstantFP>(I.getOperand(1)))
          ctx.EQ(type, dst, src0, src0);
        else
          ctx.ORD(type, dst, src0, src1);
        break;
      case ICmpInst::FCMP_UNO:
        if(isa<ConstantFP>(I.getOperand(0)))
          ctx.NE(type, dst, src1, src1);
        else if(isa<ConstantFP>(I.getOperand(1)))
          ctx.NE(type, dst, src0, src0);
        else {
          ctx.ORD(type, tmp, src0, src1);
          ctx.XOR(insnType, dst, tmp, getRegister(cv));  //TODO: Use NOT directly
        }
        break;
      case ICmpInst::FCMP_UEQ:
        ctx.NE(type, tmp, src0, src1);
        ctx.XOR(insnType, dst, tmp, getRegister(cv));
        break;
      case ICmpInst::FCMP_UGT:
        ctx.LE(type, tmp, src0, src1);
        ctx.XOR(insnType, dst, tmp, getRegister(cv));
        break;
      case ICmpInst::FCMP_UGE:
        ctx.LT(type, tmp, src0, src1);
        ctx.XOR(insnType, dst, tmp, getRegister(cv));
        break;
      case ICmpInst::FCMP_ULT:
        ctx.GE(type, tmp, src0, src1);
        ctx.XOR(insnType, dst, tmp, getRegister(cv));
        break;
      case ICmpInst::FCMP_ULE:
        ctx.GT(type, tmp, src0, src1);
        ctx.XOR(insnType, dst, tmp, getRegister(cv));
        break;
      case ICmpInst::FCMP_UNE:
        ctx.EQ(type, tmp, src0, src1);
        ctx.XOR(insnType, dst, tmp, getRegister(cv));
        break;
      case ICmpInst::FCMP_TRUE:
        ctx.MOV(insnType, dst, getRegister(cv));
        break;
      default: NOT_SUPPORTED;
    }
  }

  void GenWriter::regAllocateCastInst(CastInst &I) {
    Value *dstValue = &I;
    Value *srcValue = I.getOperand(0);
    const auto op = I.getOpcode();

    switch (op)
    {
      // When casting pointer to integers, be aware with integers
      case Instruction::PtrToInt:
      case Instruction::IntToPtr:
      {
        Constant *CPV = dyn_cast<Constant>(srcValue);
        if (CPV == NULL) {
#if GBE_DEBUG
          Type *dstType = dstValue->getType();
          Type *srcType = srcValue->getType();
          GBE_ASSERT(getTypeByteSize(unit, dstType) == getTypeByteSize(unit, srcType));
#endif /* GBE_DEBUG */
          regTranslator.newValueProxy(srcValue, dstValue);
        } else
          this->newRegister(dstValue);
      }
      break;
      // Bitcast just forward registers
      case Instruction::BitCast:
      {
        Type *srcType = srcValue->getType();
        Type *dstType = dstValue->getType();

        if(srcType->isVectorTy() || dstType->isVectorTy())
          this->newRegister(dstValue);
        else
          regTranslator.newValueProxy(srcValue, dstValue);
      }
      break;
      // Various conversion operations -> just allocate registers for them
      case Instruction::FPToUI:
      case Instruction::FPToSI:
      case Instruction::SIToFP:
      case Instruction::UIToFP:
      case Instruction::SExt:
      case Instruction::ZExt:
      case Instruction::FPExt:
      case Instruction::FPTrunc:
      case Instruction::Trunc:
        this->newRegister(&I);
      break;
      default: NOT_SUPPORTED;
    }
  }

  void GenWriter::emitCastInst(CastInst &I) {
    switch (I.getOpcode())
    {
      case Instruction::PtrToInt:
      case Instruction::IntToPtr:
      {
        Value *dstValue = &I;
        Value *srcValue = I.getOperand(0);
        Constant *CPV = dyn_cast<Constant>(srcValue);
        if (CPV != NULL) {
          const ir::ImmediateIndex index = ctx.newImmediate(CPV);
          const ir::Immediate imm = ctx.getImmediate(index);
          const ir::Register reg = this->getRegister(dstValue);
          ctx.LOADI(imm.getType(), reg, index);
        }
      }
      break;
      case Instruction::BitCast:
      {
        Value *srcValue = I.getOperand(0);
        Value *dstValue = &I;
        uint32_t srcElemNum = 0, dstElemNum = 0 ;
        ir::Type srcType = getVectorInfo(ctx, srcValue->getType(), srcValue, srcElemNum);
        ir::Type dstType = getVectorInfo(ctx, dstValue->getType(), dstValue, dstElemNum);
        // As long and double are not compatible in register storage
        // and we do not support double yet, simply put an assert here
        GBE_ASSERT(!(srcType == ir::TYPE_S64 && dstType == ir::TYPE_DOUBLE));
        GBE_ASSERT(!(dstType == ir::TYPE_S64 && srcType == ir::TYPE_DOUBLE));

        if(srcElemNum > 1 || dstElemNum > 1) {
          // Build the tuple data in the vector
          vector<ir::Register> srcTupleData;
          vector<ir::Register> dstTupleData;
          uint32_t elemID = 0;
          for (elemID = 0; elemID < srcElemNum; ++elemID) {
            ir::Register reg;
            reg = this->getRegister(srcValue, elemID);
            srcTupleData.push_back(reg);
          }
          for (elemID = 0; elemID < dstElemNum; ++elemID) {
            ir::Register reg;
            reg = this->getRegister(dstValue, elemID);
            dstTupleData.push_back(reg);
          }

          const ir::Tuple srcTuple = ctx.arrayTuple(&srcTupleData[0], srcElemNum);
          const ir::Tuple dstTuple = ctx.arrayTuple(&dstTupleData[0], dstElemNum);

          ctx.BITCAST(dstType, srcType, dstTuple, srcTuple, dstElemNum, srcElemNum);
        }
      }
      break; // nothing to emit here
      case Instruction::FPToUI:
      case Instruction::FPToSI:
      case Instruction::SIToFP:
      case Instruction::UIToFP:
      case Instruction::SExt:
      case Instruction::ZExt:
      case Instruction::FPExt:
      case Instruction::FPTrunc:
      case Instruction::Trunc:
      {
        // Get the element type for a vector
        Type *llvmDstType = I.getType();
        Type *llvmSrcType = I.getOperand(0)->getType();
        ir::Type dstType;
        if (I.getOpcode() == Instruction::FPToUI)
          dstType = getUnsignedType(ctx, llvmDstType);
        else
          dstType = getType(ctx, llvmDstType);
        ir::Type srcType;
        if (I.getOpcode() == Instruction::ZExt || I.getOpcode() == Instruction::UIToFP) {
          srcType = getUnsignedType(ctx, llvmSrcType);
        } else {
          srcType = getType(ctx, llvmSrcType);
        }

        // We use a select (0,1) not a convert when the destination is a boolean
        if (srcType == ir::TYPE_BOOL) {
          const ir::RegisterFamily family = getFamily(dstType);
          const ir::ImmediateIndex zero = ctx.newIntegerImmediate(0, dstType);
          ir::ImmediateIndex one;
          if (I.getOpcode() == Instruction::SExt
              && (dstType == ir::TYPE_S8 || dstType == ir::TYPE_S16 || dstType == ir::TYPE_S32 || dstType == ir::TYPE_S64))
            one = ctx.newIntegerImmediate(-1, dstType);
          else
            one = ctx.newIntegerImmediate(1, dstType);
          const ir::Register zeroReg = ctx.reg(family);
          const ir::Register oneReg = ctx.reg(family);
          ctx.LOADI(dstType, zeroReg, zero);
          ctx.LOADI(dstType, oneReg, one);
          const ir::Register dst = this->getRegister(&I);
          const ir::Register src = this->getRegister(I.getOperand(0));
          ctx.SEL(dstType, dst, src, oneReg, zeroReg);
        }
        // Use a convert for the other cases
        else {
          const ir::Register dst = this->getRegister(&I);
          const ir::Register src = this->getRegister(I.getOperand(0));
          ctx.CVT(dstType, srcType, dst, src);
        }
      }
      break;
      default: NOT_SUPPORTED;
    }
  }

  /*! Because there are still fake insert/extract instruction for
   *  load/store, so keep empty function here */
  void GenWriter::regAllocateInsertElement(InsertElementInst &I) {}
  void GenWriter::emitInsertElement(InsertElementInst &I) {
    const VectorType *type = dyn_cast<VectorType>(I.getType());
    GBE_ASSERT(type);
    const int elemNum = type->getNumElements();

    Value *vec = I.getOperand(0);
    Value *value = I.getOperand(1);
    const Value *index = I.getOperand(2);
    const ConstantInt *c = dyn_cast<ConstantInt>(index);
    int i = c->getValue().getSExtValue();

    for(int j=0; j<elemNum; j++) {
      if(i == j)
        regTranslator.newValueProxy(value, &I, 0, i);
      else
        regTranslator.newValueProxy(vec, &I, j, j);
    }
  }

  void GenWriter::regAllocateExtractElement(ExtractElementInst &I) {
    Value *vec = I.getVectorOperand();
    const Value *index = I.getIndexOperand();
    const ConstantInt *c = dyn_cast<ConstantInt>(index);
    GBE_ASSERT(c);
    int i = c->getValue().getSExtValue();
    regTranslator.newValueProxy(vec, &I, i, 0);
  }

  void GenWriter::emitExtractElement(ExtractElementInst &I) {
  }

  void GenWriter::regAllocateShuffleVectorInst(ShuffleVectorInst &I) {}
  void GenWriter::emitShuffleVectorInst(ShuffleVectorInst &I) {}

  void GenWriter::regAllocateSelectInst(SelectInst &I) {
    this->newRegister(&I);
  }

  void GenWriter::emitSelectInst(SelectInst &I) {
    // Get the element type for a vector
    const ir::Type type = getType(ctx, I.getType());

    // Emit the instructions in a row
    const ir::Register dst = this->getRegister(&I);
    const ir::Register cond = this->getRegister(I.getOperand(0));
    const ir::Register src0 = this->getRegister(I.getOperand(1));
    const ir::Register src1 = this->getRegister(I.getOperand(2));
    ctx.SEL(type, dst, cond, src0, src1);
  }

  void GenWriter::regAllocatePHINode(PHINode &I) {
    // Copy 1 for the PHI
    this->newRegister(&I);
    // Copy 2 to avoid lost copy issue
    Value *copy = this->getPHICopy(&I);
    this->newRegister(&I, copy);
  }

  void GenWriter::emitPHINode(PHINode &I) {
    Value *copy = this->getPHICopy(&I);
    const ir::Type type = getType(ctx, I.getType());

    const ir::Register dst = this->getRegister(&I);
    const ir::Register src = this->getRegister(copy);
    ctx.MOV(type, dst, src);
    phiMap.insert(std::make_pair(dst, src));
  }

  void GenWriter::regAllocateBranchInst(BranchInst &I) {}

  void GenWriter::emitBranchInst(BranchInst &I) {
    // Emit MOVs if required
    BasicBlock *bb = I.getParent();
    this->emitMovForPHI(bb, I.getSuccessor(0));
    if (I.isConditional())
      this->emitMovForPHI(bb, I.getSuccessor(1));

    // Inconditional branch. Just check that we jump to a block which is not our
    // successor
    if (I.isConditional() == false) {
      BasicBlock *target = I.getSuccessor(0);
      if (std::next(Function::iterator(bb)) != Function::iterator(target)) {
        GBE_ASSERT(labelMap.find(target) != labelMap.end());
        const ir::LabelIndex labelIndex = labelMap[target];
        ctx.BRA(labelIndex);
      }
    }
    // The LLVM branch has two targets
    else {
      BasicBlock *taken = NULL, *nonTaken = NULL;
      Value *condition = I.getCondition();

      // We may inverted the branch condition to simplify the branching code
      const bool inverted = conditionSet.find(condition) != conditionSet.end();
      taken = inverted ? I.getSuccessor(1) : I.getSuccessor(0);
      nonTaken = inverted ? I.getSuccessor(0) : I.getSuccessor(1);

      // Get both taken label and predicate register
      GBE_ASSERT(labelMap.find(taken) != labelMap.end());
      const ir::LabelIndex index = labelMap[taken];
      const ir::Register reg = this->getRegister(condition);
      ctx.BRA(index, reg);

      // If non-taken target is the next block, there is nothing to do
      BasicBlock *bb = I.getParent();
      if (std::next(Function::iterator(bb)) == Function::iterator(nonTaken))
        return;

      // This is slightly more complicated here. We need to issue one more
      // branch for the non-taken condition.
      GBE_ASSERT(labelMap.find(nonTaken) != labelMap.end());
      const ir::LabelIndex untakenIndex = ctx.label();
      ctx.LABEL(untakenIndex);
      ctx.BRA(labelMap[nonTaken]);
    }
  }

  void GenWriter::regAllocateCallInst(CallInst &I) {
    Value *dst = &I;
    Value *Callee = I.getCalledValue();
    GBE_ASSERT(ctx.getFunction().getProfile() == ir::PROFILE_OCL);
    GBE_ASSERT(isa<InlineAsm>(I.getCalledValue()) == false);
    GBE_ASSERT(I.hasStructRetAttr() == false);

    // We only support a small number of intrinsics right now
    if (Function *F = I.getCalledFunction()) {
      const Intrinsic::ID intrinsicID = (Intrinsic::ID) F->getIntrinsicID();
      if (intrinsicID != 0) {
        switch (F->getIntrinsicID()) {
          case Intrinsic::stacksave:
            this->newRegister(&I);
          break;
          case Intrinsic::stackrestore:
          break;
#if LLVM_VERSION_MINOR >= 2
          case Intrinsic::lifetime_start:
          case Intrinsic::lifetime_end:
          break;
          case Intrinsic::fmuladd:
            this->newRegister(&I);
          break;
#endif /* LLVM_VERSION_MINOR >= 2 */
          case Intrinsic::debugtrap:
          case Intrinsic::dbg_value:
          case Intrinsic::dbg_declare:
          break;
          default:
          GBE_ASSERTM(false, "Unsupported intrinsics");
        }
        return;
      }
    }

    // Get the name of the called function and handle it
    const std::string fnName = Callee->getName();
    auto it = instrinsicMap.map.find(fnName);
    GBE_ASSERT(it != instrinsicMap.map.end());
    switch (it->second) {
      case GEN_OCL_GET_GROUP_ID0:
        regTranslator.newScalarProxy(ir::ocl::groupid0, dst); break;
      case GEN_OCL_GET_GROUP_ID1:
        regTranslator.newScalarProxy(ir::ocl::groupid1, dst); break;
      case GEN_OCL_GET_GROUP_ID2:
        regTranslator.newScalarProxy(ir::ocl::groupid2, dst); break;
      case GEN_OCL_GET_LOCAL_ID0:
        regTranslator.newScalarProxy(ir::ocl::lid0, dst); break;
      case GEN_OCL_GET_LOCAL_ID1:
        regTranslator.newScalarProxy(ir::ocl::lid1, dst); break;
      case GEN_OCL_GET_LOCAL_ID2:
        regTranslator.newScalarProxy(ir::ocl::lid2, dst); break;
      case GEN_OCL_GET_NUM_GROUPS0:
        regTranslator.newScalarProxy(ir::ocl::numgroup0, dst); break;
      case GEN_OCL_GET_NUM_GROUPS1:
        regTranslator.newScalarProxy(ir::ocl::numgroup1, dst); break;
      case GEN_OCL_GET_NUM_GROUPS2:
        regTranslator.newScalarProxy(ir::ocl::numgroup2, dst); break;
      case GEN_OCL_GET_LOCAL_SIZE0:
        regTranslator.newScalarProxy(ir::ocl::lsize0, dst); break;
      case GEN_OCL_GET_LOCAL_SIZE1:
        regTranslator.newScalarProxy(ir::ocl::lsize1, dst); break;
      case GEN_OCL_GET_LOCAL_SIZE2:
        regTranslator.newScalarProxy(ir::ocl::lsize2, dst); break;
      case GEN_OCL_GET_GLOBAL_SIZE0:
        regTranslator.newScalarProxy(ir::ocl::gsize0, dst); break;
      case GEN_OCL_GET_GLOBAL_SIZE1:
        regTranslator.newScalarProxy(ir::ocl::gsize1, dst); break;
      case GEN_OCL_GET_GLOBAL_SIZE2:
        regTranslator.newScalarProxy(ir::ocl::gsize2, dst); break;
      case GEN_OCL_GET_GLOBAL_OFFSET0:
        regTranslator.newScalarProxy(ir::ocl::goffset0, dst); break;
      case GEN_OCL_GET_GLOBAL_OFFSET1:
        regTranslator.newScalarProxy(ir::ocl::goffset1, dst); break;
      case GEN_OCL_GET_GLOBAL_OFFSET2:
        regTranslator.newScalarProxy(ir::ocl::goffset2, dst); break;
      case GEN_OCL_GET_WORK_DIM:
        regTranslator.newScalarProxy(ir::ocl::workdim, dst); break;
      case GEN_OCL_PRINTF_BUF_ADDR:
        regTranslator.newScalarProxy(ir::ocl::printfbptr, dst); break;
      case GEN_OCL_PRINTF_INDEX_BUF_ADDR:
        regTranslator.newScalarProxy(ir::ocl::printfiptr, dst); break;
      case GEN_OCL_FBH:
      case GEN_OCL_FBL:
      case GEN_OCL_COS:
      case GEN_OCL_SIN:
      case GEN_OCL_SQR:
      case GEN_OCL_RSQ:
      case GEN_OCL_LOG:
      case GEN_OCL_EXP:
      case GEN_OCL_POW:
      case GEN_OCL_RCP:
      case GEN_OCL_ABS:
      case GEN_OCL_FABS:
      case GEN_OCL_RNDZ:
      case GEN_OCL_RNDE:
      case GEN_OCL_RNDU:
      case GEN_OCL_RNDD:
      case GEN_OCL_GET_IMAGE_WIDTH:
      case GEN_OCL_GET_IMAGE_HEIGHT:
      case GEN_OCL_GET_IMAGE_CHANNEL_DATA_TYPE:
      case GEN_OCL_GET_IMAGE_CHANNEL_ORDER:
      case GEN_OCL_GET_IMAGE_DEPTH:
      case GEN_OCL_ATOMIC_ADD0:
      case GEN_OCL_ATOMIC_ADD1:
      case GEN_OCL_ATOMIC_SUB0:
      case GEN_OCL_ATOMIC_SUB1:
      case GEN_OCL_ATOMIC_AND0:
      case GEN_OCL_ATOMIC_AND1:
      case GEN_OCL_ATOMIC_OR0:
      case GEN_OCL_ATOMIC_OR1:
      case GEN_OCL_ATOMIC_XOR0:
      case GEN_OCL_ATOMIC_XOR1:
      case GEN_OCL_ATOMIC_XCHG0:
      case GEN_OCL_ATOMIC_XCHG1:
      case GEN_OCL_ATOMIC_UMAX0:
      case GEN_OCL_ATOMIC_UMAX1:
      case GEN_OCL_ATOMIC_UMIN0:
      case GEN_OCL_ATOMIC_UMIN1:
      case GEN_OCL_ATOMIC_IMAX0:
      case GEN_OCL_ATOMIC_IMAX1:
      case GEN_OCL_ATOMIC_IMIN0:
      case GEN_OCL_ATOMIC_IMIN1:
      case GEN_OCL_ATOMIC_INC0:
      case GEN_OCL_ATOMIC_INC1:
      case GEN_OCL_ATOMIC_DEC0:
      case GEN_OCL_ATOMIC_DEC1:
      case GEN_OCL_ATOMIC_CMPXCHG0:
      case GEN_OCL_ATOMIC_CMPXCHG1:
        // No structure can be returned
        this->newRegister(&I);
        break;
      case GEN_OCL_FORCE_SIMD8:
      case GEN_OCL_FORCE_SIMD16:
      case GEN_OCL_LBARRIER:
      case GEN_OCL_GBARRIER:
      case GEN_OCL_LGBARRIER:
        ctx.getFunction().setUseSLM(true);
        break;
      case GEN_OCL_WRITE_IMAGE_I_1D:
      case GEN_OCL_WRITE_IMAGE_UI_1D:
      case GEN_OCL_WRITE_IMAGE_F_1D:
      case GEN_OCL_WRITE_IMAGE_I_2D:
      case GEN_OCL_WRITE_IMAGE_UI_2D:
      case GEN_OCL_WRITE_IMAGE_F_2D:
      case GEN_OCL_WRITE_IMAGE_I_3D:
      case GEN_OCL_WRITE_IMAGE_UI_3D:
      case GEN_OCL_WRITE_IMAGE_F_3D:
        break;
      case GEN_OCL_READ_IMAGE_I_1D:
      case GEN_OCL_READ_IMAGE_UI_1D:
      case GEN_OCL_READ_IMAGE_F_1D:
      case GEN_OCL_READ_IMAGE_I_2D:
      case GEN_OCL_READ_IMAGE_UI_2D:
      case GEN_OCL_READ_IMAGE_F_2D:
      case GEN_OCL_READ_IMAGE_I_3D:
      case GEN_OCL_READ_IMAGE_UI_3D:
      case GEN_OCL_READ_IMAGE_F_3D:

      case GEN_OCL_READ_IMAGE_I_1D_I:
      case GEN_OCL_READ_IMAGE_UI_1D_I:
      case GEN_OCL_READ_IMAGE_F_1D_I:
      case GEN_OCL_READ_IMAGE_I_2D_I:
      case GEN_OCL_READ_IMAGE_UI_2D_I:
      case GEN_OCL_READ_IMAGE_F_2D_I:
      case GEN_OCL_READ_IMAGE_I_3D_I:
      case GEN_OCL_READ_IMAGE_UI_3D_I:
      case GEN_OCL_READ_IMAGE_F_3D_I:
      {
        // dst is a 4 elements vector. We allocate all 4 registers here.
        uint32_t elemNum;
        (void)getVectorInfo(ctx, I.getType(), &I, elemNum);
        GBE_ASSERT(elemNum == 4);
        this->newRegister(&I);
        break;
      }
      case GEN_OCL_MUL_HI_INT:
      case GEN_OCL_MUL_HI_UINT:
      case GEN_OCL_MUL_HI_I64:
      case GEN_OCL_MUL_HI_UI64:
      case GEN_OCL_UPSAMPLE_SHORT:
      case GEN_OCL_UPSAMPLE_INT:
      case GEN_OCL_UPSAMPLE_LONG:
      case GEN_OCL_MAD:
      case GEN_OCL_FMAX:
      case GEN_OCL_FMIN:
      case GEN_OCL_SADD_SAT_CHAR:
      case GEN_OCL_SADD_SAT_SHORT:
      case GEN_OCL_SADD_SAT_INT:
      case GEN_OCL_SADD_SAT_LONG:
      case GEN_OCL_UADD_SAT_CHAR:
      case GEN_OCL_UADD_SAT_SHORT:
      case GEN_OCL_UADD_SAT_INT:
      case GEN_OCL_UADD_SAT_LONG:
      case GEN_OCL_SSUB_SAT_CHAR:
      case GEN_OCL_SSUB_SAT_SHORT:
      case GEN_OCL_SSUB_SAT_INT:
      case GEN_OCL_SSUB_SAT_LONG:
      case GEN_OCL_USUB_SAT_CHAR:
      case GEN_OCL_USUB_SAT_SHORT:
      case GEN_OCL_USUB_SAT_INT:
      case GEN_OCL_USUB_SAT_LONG:
      case GEN_OCL_HADD:
      case GEN_OCL_RHADD:
      case GEN_OCL_I64HADD:
      case GEN_OCL_I64RHADD:
      case GEN_OCL_I64_MAD_SAT:
      case GEN_OCL_I64_MAD_SATU:
      case GEN_OCL_SAT_CONV_U8_TO_I8:
      case GEN_OCL_SAT_CONV_I16_TO_I8:
      case GEN_OCL_SAT_CONV_U16_TO_I8:
      case GEN_OCL_SAT_CONV_I32_TO_I8:
      case GEN_OCL_SAT_CONV_U32_TO_I8:
      case GEN_OCL_SAT_CONV_F32_TO_I8:
      case GEN_OCL_SAT_CONV_I8_TO_U8:
      case GEN_OCL_SAT_CONV_I16_TO_U8:
      case GEN_OCL_SAT_CONV_U16_TO_U8:
      case GEN_OCL_SAT_CONV_I32_TO_U8:
      case GEN_OCL_SAT_CONV_U32_TO_U8:
      case GEN_OCL_SAT_CONV_F32_TO_U8:
      case GEN_OCL_SAT_CONV_U16_TO_I16:
      case GEN_OCL_SAT_CONV_I32_TO_I16:
      case GEN_OCL_SAT_CONV_U32_TO_I16:
      case GEN_OCL_SAT_CONV_F32_TO_I16:
      case GEN_OCL_SAT_CONV_I16_TO_U16:
      case GEN_OCL_SAT_CONV_I32_TO_U16:
      case GEN_OCL_SAT_CONV_U32_TO_U16:
      case GEN_OCL_SAT_CONV_F32_TO_U16:
      case GEN_OCL_SAT_CONV_U32_TO_I32:
      case GEN_OCL_SAT_CONV_F32_TO_I32:
      case GEN_OCL_SAT_CONV_I32_TO_U32:
      case GEN_OCL_SAT_CONV_F32_TO_U32:
      case GEN_OCL_CONV_F16_TO_F32:
      case GEN_OCL_CONV_F32_TO_F16:
      case GEN_OCL_SIMD_ANY:
      case GEN_OCL_SIMD_ALL:
        this->newRegister(&I);
        break;
      case GEN_OCL_PRINTF:
        break;
      default:
        GBE_ASSERTM(false, "Function call are not supported yet");
    };
  }

  void GenWriter::emitUnaryCallInst(CallInst &I, CallSite &CS, ir::Opcode opcode) {
    CallSite::arg_iterator AI = CS.arg_begin();
#if GBE_DEBUG
    CallSite::arg_iterator AE = CS.arg_end();
#endif /* GBE_DEBUG */
    GBE_ASSERT(AI != AE);
    const ir::Register src = this->getRegister(*AI);
    const ir::Register dst = this->getRegister(&I);
    ctx.ALU1(opcode, ir::TYPE_FLOAT, dst, src);
  }

  void GenWriter::emitAtomicInst(CallInst &I, CallSite &CS, ir::AtomicOps opcode) {
    CallSite::arg_iterator AI = CS.arg_begin();
    CallSite::arg_iterator AE = CS.arg_end();
    GBE_ASSERT(AI != AE);

    unsigned int llvmSpace = (*AI)->getType()->getPointerAddressSpace();
    const ir::AddressSpace addrSpace = addressSpaceLLVMToGen(llvmSpace);
    const ir::Register dst = this->getRegister(&I);

    ir::BTI bti;
    gatherBTI(*AI, bti);
    vector<ir::Register> src;
    uint32_t srcNum = 0;
    while(AI != AE) {
      src.push_back(this->getRegister(*(AI++)));
      srcNum++;
    }
    const ir::Tuple srcTuple = ctx.arrayTuple(&src[0], srcNum);
    ctx.ATOMIC(opcode, dst, addrSpace, bti, srcTuple);
  }

  /* append a new sampler. should be called before any reference to
   * a sampler_t value. */
  uint8_t GenWriter::appendSampler(CallSite::arg_iterator AI) {
    Constant *CPV = dyn_cast<Constant>(*AI);
    uint8_t index;
    if (CPV != NULL)
    {
      // This is not a kernel argument sampler, we need to append it to sampler set,
      // and allocate a sampler slot for it.
      const ir::Immediate &x = processConstantImm(CPV);
      GBE_ASSERTM(x.getType() == ir::TYPE_U16 || x.getType() == ir::TYPE_S16, "Invalid sampler type");

      index = ctx.getFunction().getSamplerSet()->append(x.getIntegerValue(), &ctx);
    } else {
      const ir::Register samplerReg = this->getRegister(*AI);
      index = ctx.getFunction().getSamplerSet()->append(samplerReg, &ctx);
    }
    return index;
  }

  void GenWriter::emitCallInst(CallInst &I) {
    if (Function *F = I.getCalledFunction()) {
      if (F->getIntrinsicID() != 0) {
        const ir::Function &fn = ctx.getFunction();
        switch (F->getIntrinsicID()) {
          case Intrinsic::stacksave:
          {
            const ir::Register dst = this->getRegister(&I);
            const ir::Register src = ir::ocl::stackptr;
            const ir::RegisterFamily family = fn.getRegisterFamily(dst);
            ctx.MOV(ir::getType(family), dst, src);
          }
          break;
          case Intrinsic::stackrestore:
          {
            const ir::Register dst = ir::ocl::stackptr;
            const ir::Register src = this->getRegister(I.getOperand(0));
            const ir::RegisterFamily family = fn.getRegisterFamily(dst);
            ctx.MOV(ir::getType(family), dst, src);
          }
          break;
#if LLVM_VERSION_MINOR >= 2
          case Intrinsic::fmuladd:
          {
            const ir::Register tmp  = ctx.reg(ir::FAMILY_DWORD);
            const ir::Register dst  = this->getRegister(&I);
            const ir::Register src0 = this->getRegister(I.getOperand(0));
            const ir::Register src1 = this->getRegister(I.getOperand(1));
            const ir::Register src2 = this->getRegister(I.getOperand(2));
            ctx.MUL(ir::TYPE_FLOAT, tmp, src0, src1);
            ctx.ADD(ir::TYPE_FLOAT, dst, tmp, src2);
            break;
          }
          break;
          case Intrinsic::lifetime_start:
          case Intrinsic::lifetime_end:
          break;
#endif /* LLVM_VERSION_MINOR >= 2 */
          case Intrinsic::debugtrap:
          case Intrinsic::dbg_value:
          case Intrinsic::dbg_declare:
          break;
          default: NOT_IMPLEMENTED;
        }
      } else {
        int image_dim;
        // Get the name of the called function and handle it
        Value *Callee = I.getCalledValue();
        const std::string fnName = Callee->getName();
        auto it = instrinsicMap.map.find(fnName);
        GBE_ASSERT(it != instrinsicMap.map.end());

        // Get the function arguments
        CallSite CS(&I);
        CallSite::arg_iterator AI = CS.arg_begin();
#if GBE_DEBUG
        CallSite::arg_iterator AE = CS.arg_end();
#endif /* GBE_DEBUG */

        switch (it->second) {
          case GEN_OCL_POW:
          {
            const ir::Register src0 = this->getRegister(*AI); ++AI;
            const ir::Register src1 = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.POW(ir::TYPE_FLOAT, dst, src0, src1);
            break;
          }
          case GEN_OCL_FBH: this->emitUnaryCallInst(I,CS,ir::OP_FBH); break;
          case GEN_OCL_FBL: this->emitUnaryCallInst(I,CS,ir::OP_FBL); break;
          case GEN_OCL_ABS:
          {
            const ir::Register src = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU1(ir::OP_ABS, ir::TYPE_S32, dst, src);
            break;
          }
          case GEN_OCL_SIMD_ALL:
          {
            const ir::Register src = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU1(ir::OP_SIMD_ALL, ir::TYPE_S16, dst, src);
            break;
          }
          case GEN_OCL_SIMD_ANY:
          {
            const ir::Register src = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU1(ir::OP_SIMD_ANY, ir::TYPE_S16, dst, src);
            break;
          }
          case GEN_OCL_COS: this->emitUnaryCallInst(I,CS,ir::OP_COS); break;
          case GEN_OCL_SIN: this->emitUnaryCallInst(I,CS,ir::OP_SIN); break;
          case GEN_OCL_LOG: this->emitUnaryCallInst(I,CS,ir::OP_LOG); break;
          case GEN_OCL_EXP: this->emitUnaryCallInst(I,CS,ir::OP_EXP); break;
          case GEN_OCL_SQR: this->emitUnaryCallInst(I,CS,ir::OP_SQR); break;
          case GEN_OCL_RSQ: this->emitUnaryCallInst(I,CS,ir::OP_RSQ); break;
          case GEN_OCL_RCP: this->emitUnaryCallInst(I,CS,ir::OP_RCP); break;
          case GEN_OCL_FABS: this->emitUnaryCallInst(I,CS,ir::OP_ABS); break;
          case GEN_OCL_RNDZ: this->emitUnaryCallInst(I,CS,ir::OP_RNDZ); break;
          case GEN_OCL_RNDE: this->emitUnaryCallInst(I,CS,ir::OP_RNDE); break;
          case GEN_OCL_RNDU: this->emitUnaryCallInst(I,CS,ir::OP_RNDU); break;
          case GEN_OCL_RNDD: this->emitUnaryCallInst(I,CS,ir::OP_RNDD); break;
          case GEN_OCL_FORCE_SIMD8: ctx.setSimdWidth(8); break;
          case GEN_OCL_FORCE_SIMD16: ctx.setSimdWidth(16); break;
          case GEN_OCL_LBARRIER: ctx.SYNC(ir::syncLocalBarrier); break;
          case GEN_OCL_GBARRIER: ctx.SYNC(ir::syncGlobalBarrier); break;
          case GEN_OCL_LGBARRIER: ctx.SYNC(ir::syncLocalBarrier | ir::syncGlobalBarrier); break;
          case GEN_OCL_ATOMIC_ADD0:
          case GEN_OCL_ATOMIC_ADD1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_ADD); break;
          case GEN_OCL_ATOMIC_SUB0:
          case GEN_OCL_ATOMIC_SUB1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_SUB); break;
          case GEN_OCL_ATOMIC_AND0:
          case GEN_OCL_ATOMIC_AND1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_AND); break;
          case GEN_OCL_ATOMIC_OR0:
          case GEN_OCL_ATOMIC_OR1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_OR); break;
          case GEN_OCL_ATOMIC_XOR0:
          case GEN_OCL_ATOMIC_XOR1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_XOR); break;
          case GEN_OCL_ATOMIC_XCHG0:
          case GEN_OCL_ATOMIC_XCHG1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_XCHG); break;
          case GEN_OCL_ATOMIC_INC0:
          case GEN_OCL_ATOMIC_INC1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_INC); break;
          case GEN_OCL_ATOMIC_DEC0:
          case GEN_OCL_ATOMIC_DEC1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_DEC); break;
          case GEN_OCL_ATOMIC_UMIN0:
          case GEN_OCL_ATOMIC_UMIN1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_UMIN); break;
          case GEN_OCL_ATOMIC_UMAX0:
          case GEN_OCL_ATOMIC_UMAX1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_UMAX); break;
          case GEN_OCL_ATOMIC_IMIN0:
          case GEN_OCL_ATOMIC_IMIN1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_IMIN); break;
          case GEN_OCL_ATOMIC_IMAX0:
          case GEN_OCL_ATOMIC_IMAX1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_IMAX); break;
          case GEN_OCL_ATOMIC_CMPXCHG0:
          case GEN_OCL_ATOMIC_CMPXCHG1: this->emitAtomicInst(I,CS,ir::ATOMIC_OP_CMPXCHG); break;
          case GEN_OCL_GET_IMAGE_WIDTH:
          case GEN_OCL_GET_IMAGE_HEIGHT:
          case GEN_OCL_GET_IMAGE_DEPTH:
          case GEN_OCL_GET_IMAGE_CHANNEL_DATA_TYPE:
          case GEN_OCL_GET_IMAGE_CHANNEL_ORDER:
          {
            GBE_ASSERT(AI != AE); const ir::Register surfaceReg = this->getRegister(*AI); ++AI;
            const ir::Register reg = this->getRegister(&I, 0);
            int infoType = it->second - GEN_OCL_GET_IMAGE_WIDTH;
            const uint8_t surfaceID = ctx.getFunction().getImageSet()->getIdx(surfaceReg);
            ir::ImageInfoKey key(surfaceID, infoType);
            const ir::Register infoReg = ctx.getFunction().getImageSet()->appendInfo(key, &ctx);
            ctx.GET_IMAGE_INFO(infoType, reg, surfaceID, infoReg);
            break;
          }

          case GEN_OCL_READ_IMAGE_I_1D:
          case GEN_OCL_READ_IMAGE_UI_1D:
          case GEN_OCL_READ_IMAGE_F_1D:
          case GEN_OCL_READ_IMAGE_I_1D_I:
          case GEN_OCL_READ_IMAGE_UI_1D_I:
          case GEN_OCL_READ_IMAGE_F_1D_I:
            image_dim = 1;
            goto handle_read_image;
          case GEN_OCL_READ_IMAGE_I_2D:
          case GEN_OCL_READ_IMAGE_UI_2D:
          case GEN_OCL_READ_IMAGE_F_2D:
          case GEN_OCL_READ_IMAGE_I_2D_I:
          case GEN_OCL_READ_IMAGE_UI_2D_I:
          case GEN_OCL_READ_IMAGE_F_2D_I:
            image_dim = 2;
            goto handle_read_image;
          case GEN_OCL_READ_IMAGE_I_3D:
          case GEN_OCL_READ_IMAGE_UI_3D:
          case GEN_OCL_READ_IMAGE_F_3D:
          case GEN_OCL_READ_IMAGE_I_3D_I:
          case GEN_OCL_READ_IMAGE_UI_3D_I:
          case GEN_OCL_READ_IMAGE_F_3D_I:
            image_dim = 3;
handle_read_image:
          {
            GBE_ASSERT(AI != AE); const ir::Register surfaceReg = this->getRegister(*AI); ++AI;
            const uint8_t surfaceID = ctx.getFunction().getImageSet()->getIdx(surfaceReg);
            GBE_ASSERT(AI != AE);
            const uint8_t sampler = this->appendSampler(AI);
            ++AI;

            ir::Register ucoord;
            ir::Register vcoord;
            ir::Register wcoord;

            GBE_ASSERT(AI != AE); ucoord = this->getRegister(*AI); ++AI;
            if (image_dim > 1) {
              GBE_ASSERT(AI != AE);
              vcoord = this->getRegister(*AI);
              ++AI;
            } else {
              vcoord = ir::ocl::invalid;
            }

            if (image_dim > 2) {
              GBE_ASSERT(AI != AE);
              wcoord = this->getRegister(*AI);
              ++AI;
            } else {
              wcoord = ir::ocl::invalid;
            }

            vector<ir::Register> dstTupleData, srcTupleData;
            const uint32_t elemNum = 4;
            for (uint32_t elemID = 0; elemID < elemNum; ++elemID) {
              const ir::Register reg = this->getRegister(&I, elemID);
              dstTupleData.push_back(reg);
            }
            srcTupleData.push_back(ucoord);
            srcTupleData.push_back(vcoord);
            srcTupleData.push_back(wcoord);
            uint8_t samplerOffset = 0;
#ifdef GEN7_SAMPLER_CLAMP_BORDER_WORKAROUND
            GBE_ASSERT(AI != AE); Constant *CPV = dyn_cast<Constant>(*AI);
            assert(CPV);
            const ir::Immediate &x = processConstantImm(CPV);
            GBE_ASSERTM(x.getType() == ir::TYPE_U32 || x.getType() == ir::TYPE_S32, "Invalid sampler type");
            samplerOffset = x.getIntegerValue();
#endif
            const ir::Tuple dstTuple = ctx.arrayTuple(&dstTupleData[0], elemNum);
            const ir::Tuple srcTuple = ctx.arrayTuple(&srcTupleData[0], 3);

            ir::Type dstType = ir::TYPE_U32;

            switch(it->second) {
              case GEN_OCL_READ_IMAGE_I_1D:
              case GEN_OCL_READ_IMAGE_UI_1D:
              case GEN_OCL_READ_IMAGE_I_2D:
              case GEN_OCL_READ_IMAGE_UI_2D:
              case GEN_OCL_READ_IMAGE_I_3D:
              case GEN_OCL_READ_IMAGE_UI_3D:
              case GEN_OCL_READ_IMAGE_I_1D_I:
              case GEN_OCL_READ_IMAGE_UI_1D_I:
              case GEN_OCL_READ_IMAGE_I_2D_I:
              case GEN_OCL_READ_IMAGE_UI_2D_I:
              case GEN_OCL_READ_IMAGE_I_3D_I:
              case GEN_OCL_READ_IMAGE_UI_3D_I:
                dstType = ir::TYPE_U32;
                break;
              case GEN_OCL_READ_IMAGE_F_1D:
              case GEN_OCL_READ_IMAGE_F_2D:
              case GEN_OCL_READ_IMAGE_F_3D:
              case GEN_OCL_READ_IMAGE_F_1D_I:
              case GEN_OCL_READ_IMAGE_F_2D_I:
              case GEN_OCL_READ_IMAGE_F_3D_I:
                dstType = ir::TYPE_FLOAT;
                break;
              default:
                GBE_ASSERT(0); // never been here.
            }

            bool isFloatCoord = it->second <= GEN_OCL_READ_IMAGE_F_3D;

            ctx.SAMPLE(surfaceID, dstTuple, srcTuple, dstType == ir::TYPE_FLOAT,
                       isFloatCoord, sampler, samplerOffset);
            break;
          }

          case GEN_OCL_WRITE_IMAGE_I_1D:
          case GEN_OCL_WRITE_IMAGE_UI_1D:
          case GEN_OCL_WRITE_IMAGE_F_1D:
            image_dim = 1;
            goto handle_write_image;
          case GEN_OCL_WRITE_IMAGE_I_2D:
          case GEN_OCL_WRITE_IMAGE_UI_2D:
          case GEN_OCL_WRITE_IMAGE_F_2D:
            image_dim = 2;
            goto handle_write_image;
          case GEN_OCL_WRITE_IMAGE_I_3D:
          case GEN_OCL_WRITE_IMAGE_UI_3D:
          case GEN_OCL_WRITE_IMAGE_F_3D:
            image_dim = 3;
handle_write_image:
          {
            GBE_ASSERT(AI != AE); const ir::Register surfaceReg = this->getRegister(*AI); ++AI;
            const uint8_t surfaceID = ctx.getFunction().getImageSet()->getIdx(surfaceReg);
            ir::Register ucoord, vcoord, wcoord;

            GBE_ASSERT(AI != AE); ucoord = this->getRegister(*AI); ++AI;

            if (image_dim > 1) {
              GBE_ASSERT(AI != AE);
              vcoord = this->getRegister(*AI);
              ++AI;
            } else
              vcoord = ir::ocl::invalid;

            if (image_dim > 2) {
              GBE_ASSERT(AI != AE);
              wcoord = this->getRegister(*AI);
              ++AI;
            } else {
              wcoord = ir::ocl::invalid;
            }

            GBE_ASSERT(AI != AE);
            vector<ir::Register> srcTupleData;

            srcTupleData.push_back(ucoord);
            srcTupleData.push_back(vcoord);
            srcTupleData.push_back(wcoord);

            const uint32_t elemNum = 4;
            for (uint32_t elemID = 0; elemID < elemNum; ++elemID) {
              const ir::Register reg = this->getRegister(*AI, elemID);
              srcTupleData.push_back(reg);
            }
            const ir::Tuple srcTuple = ctx.arrayTuple(&srcTupleData[0], 7);

            ir::Type srcType = ir::TYPE_U32;

            switch(it->second) {
              case GEN_OCL_WRITE_IMAGE_I_1D:
              case GEN_OCL_WRITE_IMAGE_UI_1D:
              case GEN_OCL_WRITE_IMAGE_I_2D:
              case GEN_OCL_WRITE_IMAGE_UI_2D:
              case GEN_OCL_WRITE_IMAGE_I_3D:
              case GEN_OCL_WRITE_IMAGE_UI_3D:
                srcType = ir::TYPE_U32;
                break;
              case GEN_OCL_WRITE_IMAGE_F_1D:
              case GEN_OCL_WRITE_IMAGE_F_2D:
              case GEN_OCL_WRITE_IMAGE_F_3D:
                srcType = ir::TYPE_FLOAT;
                break;
              default:
                GBE_ASSERT(0); // never been here.
            }

            ctx.TYPED_WRITE(surfaceID, srcTuple, srcType, ir::TYPE_U32);
            break;
          }
          case GEN_OCL_MUL_HI_INT:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.MUL_HI(getType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_MUL_HI_UINT:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.MUL_HI(getUnsignedType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_MUL_HI_I64:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.I64_MUL_HI(getType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_MUL_HI_UI64:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.I64_MUL_HI(getUnsignedType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_UPSAMPLE_SHORT:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.UPSAMPLE_SHORT(getType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_UPSAMPLE_INT:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.UPSAMPLE_INT(getType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_UPSAMPLE_LONG:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.UPSAMPLE_LONG(getType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_SADD_SAT_CHAR:
          case GEN_OCL_SADD_SAT_SHORT:
          case GEN_OCL_SADD_SAT_INT:
          case GEN_OCL_SADD_SAT_LONG:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.ADDSAT(getType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_UADD_SAT_CHAR:
          case GEN_OCL_UADD_SAT_SHORT:
          case GEN_OCL_UADD_SAT_INT:
          case GEN_OCL_UADD_SAT_LONG:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.ADDSAT(getUnsignedType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_SSUB_SAT_CHAR:
          case GEN_OCL_SSUB_SAT_SHORT:
          case GEN_OCL_SSUB_SAT_INT:
          case GEN_OCL_SSUB_SAT_LONG:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.SUBSAT(getType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_USUB_SAT_CHAR:
          case GEN_OCL_USUB_SAT_SHORT:
          case GEN_OCL_USUB_SAT_INT:
          case GEN_OCL_USUB_SAT_LONG:
          {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.SUBSAT(getUnsignedType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_I64_MAD_SAT:
           {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src2 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.I64MADSAT(getType(ctx, I.getType()), dst, src0, src1, src2);
            break;
           }
          case GEN_OCL_I64_MAD_SATU:
           {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src2 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.I64MADSAT(getUnsignedType(ctx, I.getType()), dst, src0, src1, src2);
            break;
           }
          case GEN_OCL_MAD: {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src2 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.MAD(getType(ctx, I.getType()), dst, src0, src1, src2);
            break;
          }
          case GEN_OCL_FMAX:
          case GEN_OCL_FMIN:{
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            const ir::Register cmp = ctx.reg(ir::FAMILY_BOOL);
            //Becasue cmp's sources are same as sel's source, so cmp instruction and sel
            //instruction will be merged to one sel_cmp instruction in the gen selection
            //Add two intruction here for simple.
            if(it->second == GEN_OCL_FMAX)
              ctx.GE(getType(ctx, I.getType()), cmp, src0, src1);
            else
              ctx.LT(getType(ctx, I.getType()), cmp, src0, src1);
            ctx.SEL(getType(ctx, I.getType()), dst, cmp, src0, src1);
            break;
          }
          case GEN_OCL_HADD: {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.HADD(getUnsignedType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_I64HADD:
           {
            GBE_ASSERT(AI != AE);
            const ir::Register src0 = this->getRegister(*(AI++));
            GBE_ASSERT(AI != AE);
            const ir::Register src1 = this->getRegister(*(AI++));
            const ir::Register dst = this->getRegister(&I);
            ctx.I64HADD(ir::TYPE_U64, dst, src0, src1);
            break;
           }
          case GEN_OCL_RHADD: {
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.RHADD(getUnsignedType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_I64RHADD:
           {
            GBE_ASSERT(AI != AE);
            const ir::Register src0 = this->getRegister(*(AI++));
            GBE_ASSERT(AI != AE);
            const ir::Register src1 = this->getRegister(*(AI++));
            const ir::Register dst = this->getRegister(&I);
            ctx.I64RHADD(ir::TYPE_U64, dst, src0, src1);
            break;
           }
#define DEF(DST_TYPE, SRC_TYPE) \
  { ctx.SAT_CVT(DST_TYPE, SRC_TYPE, getRegister(&I), getRegister(I.getOperand(0))); break; }
          case GEN_OCL_SAT_CONV_U8_TO_I8:
            DEF(ir::TYPE_S8, ir::TYPE_U8);
          case GEN_OCL_SAT_CONV_I16_TO_I8:
            DEF(ir::TYPE_S8, ir::TYPE_S16);
          case GEN_OCL_SAT_CONV_U16_TO_I8:
            DEF(ir::TYPE_S8, ir::TYPE_U16);
          case GEN_OCL_SAT_CONV_I32_TO_I8:
            DEF(ir::TYPE_S8, ir::TYPE_S32);
          case GEN_OCL_SAT_CONV_U32_TO_I8:
            DEF(ir::TYPE_S8, ir::TYPE_U32);
          case GEN_OCL_SAT_CONV_F32_TO_I8:
            DEF(ir::TYPE_S8, ir::TYPE_FLOAT);
          case GEN_OCL_SAT_CONV_I8_TO_U8:
            DEF(ir::TYPE_U8, ir::TYPE_S8);
          case GEN_OCL_SAT_CONV_I16_TO_U8:
            DEF(ir::TYPE_U8, ir::TYPE_S16);
          case GEN_OCL_SAT_CONV_U16_TO_U8:
            DEF(ir::TYPE_U8, ir::TYPE_U16);
          case GEN_OCL_SAT_CONV_I32_TO_U8:
            DEF(ir::TYPE_U8, ir::TYPE_S32);
          case GEN_OCL_SAT_CONV_U32_TO_U8:
            DEF(ir::TYPE_U8, ir::TYPE_U32);
          case GEN_OCL_SAT_CONV_F32_TO_U8:
            DEF(ir::TYPE_U8, ir::TYPE_FLOAT);
          case GEN_OCL_SAT_CONV_U16_TO_I16:
            DEF(ir::TYPE_S16, ir::TYPE_U16);
          case GEN_OCL_SAT_CONV_I32_TO_I16:
            DEF(ir::TYPE_S16, ir::TYPE_S32);
          case GEN_OCL_SAT_CONV_U32_TO_I16:
            DEF(ir::TYPE_S16, ir::TYPE_U32);
          case GEN_OCL_SAT_CONV_F32_TO_I16:
            DEF(ir::TYPE_S16, ir::TYPE_FLOAT);
          case GEN_OCL_SAT_CONV_I16_TO_U16:
            DEF(ir::TYPE_U16, ir::TYPE_S16);
          case GEN_OCL_SAT_CONV_I32_TO_U16:
            DEF(ir::TYPE_U16, ir::TYPE_S32);
          case GEN_OCL_SAT_CONV_U32_TO_U16:
            DEF(ir::TYPE_U16, ir::TYPE_U32);
          case GEN_OCL_SAT_CONV_F32_TO_U16:
            DEF(ir::TYPE_U16, ir::TYPE_FLOAT);
          case GEN_OCL_SAT_CONV_U32_TO_I32:
            DEF(ir::TYPE_S32, ir::TYPE_U32);
          case GEN_OCL_SAT_CONV_F32_TO_I32:
            DEF(ir::TYPE_S32, ir::TYPE_FLOAT);
          case GEN_OCL_SAT_CONV_I32_TO_U32:
            DEF(ir::TYPE_U32, ir::TYPE_S32);
          case GEN_OCL_SAT_CONV_F32_TO_U32:
            DEF(ir::TYPE_U32, ir::TYPE_FLOAT);
          case GEN_OCL_CONV_F16_TO_F32:
            ctx.F16TO32(ir::TYPE_FLOAT, ir::TYPE_U16, getRegister(&I), getRegister(I.getOperand(0)));
            break;
          case GEN_OCL_CONV_F32_TO_F16:
            ctx.F32TO16(ir::TYPE_U16, ir::TYPE_FLOAT, getRegister(&I), getRegister(I.getOperand(0)));
            break;
#undef DEF

          case GEN_OCL_PRINTF:
          {
            ir::PrintfSet::PrintfFmt* fmt = (ir::PrintfSet::PrintfFmt*)getPrintfInfo(&I);
            ctx.getFunction().getPrintfSet()->append(fmt, unit);
            assert(fmt);
            break;
          }
          case GEN_OCL_PRINTF_BUF_ADDR:
          case GEN_OCL_PRINTF_INDEX_BUF_ADDR:
          default: break;
        }
      }
    }
  }

  void GenWriter::regAllocateAllocaInst(AllocaInst &I) {
    this->newRegister(&I);
  }
  void GenWriter::emitAllocaInst(AllocaInst &I) {
    Value *src = I.getOperand(0);
    Type *elemType = I.getType()->getElementType();
    ir::ImmediateIndex immIndex;
    uint32_t elementSize = getTypeByteSize(unit, elemType);

    // Be aware, we manipulate pointers
    if (ctx.getPointerSize() == ir::POINTER_32_BITS)
      immIndex = ctx.newImmediate(uint32_t(elementSize));
    else
      immIndex = ctx.newImmediate(uint64_t(elementSize));

    // OK, we try to see if we know compile time the size we need to allocate
    if (I.isArrayAllocation() == true) {
      Constant *CPV = dyn_cast<Constant>(src);
      GBE_ASSERT(CPV);
      const ir::Immediate &imm = processConstantImm(CPV);
      const uint64_t elemNum = imm.getIntegerValue();
      elementSize *= elemNum;
      if (ctx.getPointerSize() == ir::POINTER_32_BITS)
        immIndex = ctx.newImmediate(uint32_t(ALIGN(elementSize, 4)));
      else
        immIndex = ctx.newImmediate(uint64_t(ALIGN(elementSize, 4)));
    }

    // Now emit the stream of instructions to get the allocated pointer
    const ir::RegisterFamily pointerFamily = ctx.getPointerFamily();
    const ir::Register dst = this->getRegister(&I);
    const ir::Register stack = ir::ocl::stackptr;
    const ir::Register reg = ctx.reg(pointerFamily);
    const ir::Immediate imm = ctx.getImmediate(immIndex);
    uint32_t align = getAlignmentByte(unit, elemType);
    // below code assume align is power of 2
    GBE_ASSERT(align && (align & (align-1)) == 0);

    // align the stack pointer according to data alignment
    if(align > 1) {
      uint32_t prevStackPtr = ctx.getFunction().getStackSize();
      uint32_t step = ((prevStackPtr + (align - 1)) & ~(align - 1)) - prevStackPtr;
      if (step != 0) {
        ir::ImmediateIndex stepImm = ctx.newIntegerImmediate(step, ir::TYPE_U32);
        ir::Register stepReg = ctx.reg(ctx.getPointerFamily());
        ctx.LOADI(ir::TYPE_S32, stepReg, stepImm);
        ctx.ADD(ir::TYPE_U32, stack, stack, stepReg);
        ctx.getFunction().pushStackSize(step);
      }
    }
    // Set the destination register properly
    ctx.MOV(imm.getType(), dst, stack);

    ctx.LOADI(imm.getType(), reg, immIndex);
    ctx.ADD(imm.getType(), stack, stack, reg);
    ctx.getFunction().pushStackSize(elementSize);
  }

  static INLINE Value *getLoadOrStoreValue(LoadInst &I) {
    return &I;
  }
  static INLINE Value *getLoadOrStoreValue(StoreInst &I) {
    return I.getValueOperand();
  }
  void GenWriter::regAllocateLoadInst(LoadInst &I) {
    this->newRegister(&I);
  }
  void GenWriter::regAllocateStoreInst(StoreInst &I) {}

  void GenWriter::emitBatchLoadOrStore(const ir::Type type, const uint32_t elemNum,
                                      Value *llvmValues, const ir::Register ptr,
                                      const ir::AddressSpace addrSpace,
                                      Type * elemType, bool isLoad, ir::BTI bti) {
    const ir::RegisterFamily pointerFamily = ctx.getPointerFamily();
    uint32_t totalSize = elemNum * getFamilySize(getFamily(type));
    uint32_t msgNum = totalSize > 16 ? totalSize / 16 : 1;
    const uint32_t perMsgNum = elemNum / msgNum;

    for (uint32_t msg = 0; msg < msgNum; ++msg) {
      // Build the tuple data in the vector
      vector<ir::Register> tupleData; // put registers here
      for (uint32_t elemID = 0; elemID < perMsgNum; ++elemID) {
        ir::Register reg;
        if(regTranslator.isUndefConst(llvmValues, elemID)) {
          Value *v = Constant::getNullValue(elemType);
          reg = this->getRegister(v);
        } else
          reg = this->getRegister(llvmValues, perMsgNum*msg+elemID);

        tupleData.push_back(reg);
      }
      const ir::Tuple tuple = ctx.arrayTuple(&tupleData[0], perMsgNum);

      // We may need to update to offset the pointer
      ir::Register addr;
      if (msg == 0)
        addr = ptr;
      else {
        const ir::Register offset = ctx.reg(pointerFamily);
        ir::ImmediateIndex immIndex;
        ir::Type immType;
        // each message can read/write 16 byte
        const int32_t stride = 16;
        if (pointerFamily == ir::FAMILY_DWORD) {
          immIndex = ctx.newImmediate(int32_t(msg*stride));
          immType = ir::TYPE_S32;
        } else {
          immIndex = ctx.newImmediate(int64_t(msg*stride));
          immType = ir::TYPE_S64;
        }

        addr = ctx.reg(pointerFamily);
        ctx.LOADI(immType, offset, immIndex);
        ctx.ADD(immType, addr, ptr, offset);
      }

      // Emit the instruction
      if (isLoad)
        ctx.LOAD(type, tuple, addr, addrSpace, perMsgNum, true, bti);
      else
        ctx.STORE(type, tuple, addr, addrSpace, perMsgNum, true, bti);
    }
  }

  // The idea behind is to search along the use-def chain, and find out all
  // possible source of the pointer. Then in later codeGen, we can emit
  // read/store instructions to these btis gathered.
  void GenWriter::gatherBTI(Value *pointer, ir::BTI &bti) {
    typedef map<const Value*, int>::iterator GlobalPtrIter;
    Value *p;
    size_t idx = 0;
    int nBTI = 0;
    std::vector<Value*> candidates;
    candidates.push_back(pointer);
    std::set<Value*> processed;

    while (idx < candidates.size()) {
      bool isPrivate = false;
      bool needNewBTI = true;
      p = candidates[idx];

      while (dyn_cast<User>(p) && !dyn_cast<GlobalVariable>(p)) {

        if (processed.find(p) == processed.end()) {
          processed.insert(p);
        } else {
          // This use-def chain falls into a loop,
          // it does not introduce a new buffer source.
          needNewBTI = false;
          break;
        }

        if (dyn_cast<SelectInst>(p)) {
          SelectInst *sel = cast<SelectInst>(p);
          p = sel->getTrueValue();
          candidates.push_back(sel->getFalseValue());
          continue;
        }

        if (dyn_cast<PHINode>(p)) {
          PHINode* phi = cast<PHINode>(p);
          int n = phi->getNumIncomingValues();
          for (int j = 1; j < n; j++)
            candidates.push_back(phi->getIncomingValue(j));
          p = phi->getIncomingValue(0);
          continue;
        }

        if (dyn_cast<AllocaInst>(p)) {
          isPrivate = true;
          break;
        }
        p = cast<User>(p)->getOperand(0);
      }

      if (needNewBTI == false) {
        // go to next possible pointer source
        idx++; continue;
      }

      uint8_t new_bti = 0;
      if (isPrivate) {
        new_bti = BTI_PRIVATE;
      } else {
        if(isa<Argument>(p) && dyn_cast<Argument>(p)->hasByValAttr()) {
          // structure value implementation is not complete now,
          // they are now treated as push constant, so, the load/store
          // here is not as meaningful.
          bti.bti[0] = BTI_PRIVATE;
          bti.count = 1;
          break;
        }
        Type *ty = p->getType();
        if(ty->getPointerAddressSpace() == 3) {
          // __local memory
          new_bti = 0xfe;
        } else {
          // __global memory
          GlobalPtrIter iter = globalPointer.find(p);
          GBE_ASSERT(iter != globalPointer.end());
          new_bti = iter->second;
        }
      }
      // avoid duplicate
      bool bFound = false;
      for (int j = 0; j < nBTI; j++) {
        if (bti.bti[j] == new_bti) {
          bFound = true; break;
        }
      }
      if (bFound == false) {
        bti.bti[nBTI++] = new_bti;
        bti.count = nBTI;
      }
      idx++;
    }
    GBE_ASSERT(bti.count <= MAX_MIXED_POINTER);
  }

  extern int OCL_SIMD_WIDTH;
  template <bool isLoad, typename T>
  INLINE void GenWriter::emitLoadOrStore(T &I)
  {
    unsigned int llvmSpace = I.getPointerAddressSpace();
    Value *llvmPtr = I.getPointerOperand();
    Value *llvmValues = getLoadOrStoreValue(I);
    Type *llvmType = llvmValues->getType();
    const bool dwAligned = (I.getAlignment() % 4) == 0;
    const ir::AddressSpace addrSpace = addressSpaceLLVMToGen(llvmSpace);
    const ir::Register ptr = this->getRegister(llvmPtr);
    ir::BTI binding;
    if(addrSpace == ir::MEM_GLOBAL || addrSpace == ir::MEM_PRIVATE) {
      gatherBTI(llvmPtr, binding);
    }
    // Scalar is easy. We neednot build register tuples
    if (isScalarType(llvmType) == true) {
      const ir::Type type = getType(ctx, llvmType);
      const ir::Register values = this->getRegister(llvmValues);
      if (isLoad)
        ctx.LOAD(type, ptr, addrSpace, dwAligned, binding, values);
      else
        ctx.STORE(type, ptr, addrSpace, dwAligned, binding, values);
    }
    // A vector type requires to build a tuple
    else {
      VectorType *vectorType = cast<VectorType>(llvmType);
      Type *elemType = vectorType->getElementType();

      // We follow OCL spec and support 2,3,4,8,16 elements only
      uint32_t elemNum = vectorType->getNumElements();
      GBE_ASSERTM(elemNum == 2 || elemNum == 3 || elemNum == 4 || elemNum == 8 || elemNum == 16,
                  "Only vectors of 2,3,4,8 or 16 elements are supported");
      // Per OPenCL 1.2 spec 6.1.5:
      //   For 3-component vector data types, the size of the data type is 4 * sizeof(component).
      // And the llvm does cast a type3 data to type4 for load/store instruction,
      // so a 4 elements vector may only have 3 valid elements. We need to fix it to correct element
      // count here.
      if (elemNum == 4 && regTranslator.isUndefConst(llvmValues, 3))
          elemNum = 3;

      // The code is going to be fairly different from types to types (based on
      // size of each vector element)
      const ir::Type type = getType(ctx, elemType);
      const ir::RegisterFamily pointerFamily = ctx.getPointerFamily();
      const ir::RegisterFamily dataFamily = getFamily(type);

      if(dataFamily == ir::FAMILY_DWORD && addrSpace != ir::MEM_CONSTANT) {
        // One message is enough here. Nothing special to do
        if (elemNum <= 4) {
          // Build the tuple data in the vector
          vector<ir::Register> tupleData; // put registers here
          for (uint32_t elemID = 0; elemID < elemNum; ++elemID) {
            ir::Register reg;
            if(regTranslator.isUndefConst(llvmValues, elemID)) {
              Value *v = Constant::getNullValue(elemType);
              reg = this->getRegister(v);
            } else
              reg = this->getRegister(llvmValues, elemID);

            tupleData.push_back(reg);
          }
          const ir::Tuple tuple = ctx.arrayTuple(&tupleData[0], elemNum);

          // Emit the instruction
          if (isLoad)
            ctx.LOAD(type, tuple, ptr, addrSpace, elemNum, dwAligned, binding);
          else
            ctx.STORE(type, tuple, ptr, addrSpace, elemNum, dwAligned, binding);
        }
        // Not supported by the hardware. So, we split the message and we use
        // strided loads and stores
        else {
          emitBatchLoadOrStore(type, elemNum, llvmValues, ptr, addrSpace, elemType, isLoad, binding);
        }
      }
      else if((dataFamily==ir::FAMILY_WORD && elemNum%2==0) || (dataFamily == ir::FAMILY_BYTE && elemNum%4 == 0)) {
          emitBatchLoadOrStore(type, elemNum, llvmValues, ptr, addrSpace, elemType, isLoad, binding);
      } else {
        for (uint32_t elemID = 0; elemID < elemNum; elemID++) {
          if(regTranslator.isUndefConst(llvmValues, elemID))
            continue;

          const ir::Register reg = this->getRegister(llvmValues, elemID);
          ir::Register addr;
          if (elemID == 0)
            addr = ptr;
          else {
              const ir::Register offset = ctx.reg(pointerFamily);
              ir::ImmediateIndex immIndex;
              int elemSize = getTypeByteSize(unit, elemType);
              immIndex = ctx.newImmediate(int32_t(elemID * elemSize));
              addr = ctx.reg(pointerFamily);
              ctx.LOADI(ir::TYPE_S32, offset, immIndex);
              ctx.ADD(ir::TYPE_S32, addr, ptr, offset);
          }
          if (isLoad)
           ctx.LOAD(type, addr, addrSpace, dwAligned, binding, reg);
          else
           ctx.STORE(type, addr, addrSpace, dwAligned, binding, reg);
        }
      }
    }
  }

  void GenWriter::emitLoadInst(LoadInst &I) {
    this->emitLoadOrStore<true>(I);
  }

  void GenWriter::emitStoreInst(StoreInst &I) {
    this->emitLoadOrStore<false>(I);
  }

  llvm::FunctionPass *createGenPass(ir::Unit &unit) {
    return new GenWriter(unit);
  }
} /* namespace gbe */

