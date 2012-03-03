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
 *
 * Transform the LLVM IR code into Gen IR code
 */
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Intrinsics.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/InlineAsm.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/ConstantsScanner.h"
#include "llvm/Analysis/FindUsedTypes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Config/config.h"

#include "llvm/llvm_gen_backend.hpp"
#include "ir/context.hpp"
#include "ir/unit.hpp"
#include "sys/map.hpp"
#include <algorithm>

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
  static ir::Type getType(const ir::Context &ctx, const Type *type)
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
    GBE_ASSERT(0);
    return ir::TYPE_S64;
  }

  /*! Type to register family translation */
  static ir::RegisterData::Family getFamily(const ir::Context &ctx, const Type *type)
  {
    GBE_ASSERT(isScalarType(type) == true); 
    if (type == Type::getInt1Ty(type->getContext()))
      return ir::RegisterData::BOOL;
    if (type == Type::getInt8Ty(type->getContext()))
      return ir::RegisterData::BYTE;
    if (type == Type::getInt16Ty(type->getContext()))
      return ir::RegisterData::WORD;
    if (type == Type::getInt32Ty(type->getContext()) || type->isFloatTy())
      return ir::RegisterData::DWORD;
    if (type == Type::getInt64Ty(type->getContext()) || type->isDoubleTy())
      return ir::RegisterData::QWORD;
    if (type->isPointerTy() && ctx.getPointerSize() == ir::POINTER_32_BITS)
      return ir::RegisterData::DWORD;
    if (type->isPointerTy() && ctx.getPointerSize() == ir::POINTER_64_BITS)
      return ir::RegisterData::QWORD;
    GBE_ASSERT(0);
    return ir::RegisterData::BOOL;
  }

  /*! Handle the LLVM IR Value to Gen IR register translation. This has 2 roles:
   *  - Split the LLVM vector into several scalar values
   *  - Handle the transparent copies (bitcast or use of intrincics functions
   *    like get_local_id / get_global_id
   */
  class RegisterTranslator
  {
  public:
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
    ir::Register newScalar(Value *value, uint32_t index = 0u)
    {
      GBE_ASSERT(dyn_cast<Constant>(value) == NULL);
      Type *type = value->getType();
      auto typeID = type->getTypeID();
      switch (typeID) {
        case Type::IntegerTyID:
        case Type::FloatTyID:
        case Type::DoubleTyID:
        case Type::PointerTyID:
          GBE_ASSERT(index == 0);
          return this->newScalar(value, type, index);
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
            return this->newScalar(value, elementType, index);
          break;
        }
        default: NOT_SUPPORTED;
      };
      return ir::Register();
    }
    /*! Get the register from the given value at given index possibly iterating
     *  in the value map to get the final real register
     */
    ir::Register getScalar(Value *value, uint32_t index = 0u) {
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
      const auto key = std::make_pair(value, index);
      GBE_ASSERT(scalarMap.find(key) != scalarMap.end());
      return scalarMap[key];
    }

  private:
    /*! This maps a scalar register to a Value (index is the vector index when
     *  the value is a vector of scalars)
     */
    ir::Register newScalar(Value *value, Type *type, uint32_t index) {
      const auto key = std::make_pair(value, index);
      GBE_ASSERT(scalarMap.find(key) == scalarMap.end());
      const ir::RegisterData::Family family = getFamily(ctx, type);
      const ir::Register reg = ctx.reg(family);
      scalarMap[key] = reg;
      return reg;
    }
    /*! Indices will be zero for scalar values */
    typedef std::pair<Value*, uint32_t> ValueIndex;
    /*! Map value to ir::Register */
    map<ValueIndex, ir::Register> scalarMap;
    /*! Map values to values when this is only a translation (eq bitcast) */
    map<ValueIndex, ValueIndex> valueMap;
    /*! Actually allocates the registers */
    ir::Context &ctx;
  };

  class CBEMCAsmInfo : public MCAsmInfo {
  public:
    CBEMCAsmInfo() {
      GlobalPrefix = "";
      PrivateGlobalPrefix = "";
    }
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
    /*! Map value to ir::LabelIndex */
    map<const Value*, ir::LabelIndex> labelMap;
    /*! We visit each function twice. Once to allocate the registers and once to
     *  emit the Gen IR instructions 
     */
    enum Pass {
      PASS_EMIT_REGISTERS = 0,
      PASS_EMIT_INSTRUCTIONS = 1
    } pass;

    Mangler *Mang;
    LoopInfo *LI;
    const Module *TheModule;
    const MCObjectFileInfo *MOFI;
    const TargetData* TD;
    const MCAsmInfo* TAsm;
    const MCRegisterInfo *MRI;
    MCContext *TCtx;

    std::map<const ConstantFP *, unsigned> FPConstantMap;
    std::set<Function*> intrinsicPrototypesAlreadyGenerated;
    std::set<const Argument*> ByValParams;
    unsigned FPCounter;
    unsigned OpaqueCounter;
    DenseMap<const Value*, unsigned> AnonValueNumbers;
    unsigned NextAnonValueNumber;

    /// UnnamedStructIDs - This contains a unique ID for each struct that is
    /// either anonymous or has no name.
    DenseMap<StructType*, unsigned> UnnamedStructIDs;

  public:
    static char ID;
    explicit GenWriter(ir::Unit &unit)
      : FunctionPass(ID),
        unit(unit),
        ctx(unit),
        regTranslator(ctx),
        Mang(0), LI(0),
        TheModule(0), MOFI(0), TD(0),
        OpaqueCounter(0), NextAnonValueNumber(0)
    {
      initializeLoopInfoPass(*PassRegistry::getPassRegistry());
      FPCounter = 0;
      pass = PASS_EMIT_REGISTERS;
    }

    virtual const char *getPassName() const { return "Gen Back-End"; }

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.setPreservesAll();
    }

    virtual bool doInitialization(Module &M);

    bool runOnFunction(Function &F) {
     // Do not codegen any 'available_externally' functions at all, they have
     // definitions outside the translation unit.
     if (F.hasAvailableExternallyLinkage())
       return false;

      LI = &getAnalysis<LoopInfo>();

      emitFunction(F);
      return false;
    }

    virtual bool doFinalization(Module &M) {
      delete TD;
      delete Mang;
      delete MOFI;
      FPConstantMap.clear();
      ByValParams.clear();
      intrinsicPrototypesAlreadyGenerated.clear();
      UnnamedStructIDs.clear();
      return false;
    }

    /*! Emit the complete function code and declaration */
    void emitFunction(Function &F);
    /*! Handle input and output function parameters */
    void emitFunctionPrototype(Function &F);
    /*! Emit the code for a basic block */
    void emitBasicBlock(BasicBlock *BB);

    /*! Alocate one or several registers (if vector) for the value */
    INLINE void newRegister(Value *value);
    /*! Return a valid register from an operand (can use LOADI to make one) */
    INLINE ir::Register getRegister(Value *value, uint32_t index = 0);
    /*! Create a new immediate from a constant */
    ir::ImmediateIndex newImmediate(Constant *CPV);
    /*! Insert a new label index when this is a scalar value */
    INLINE void newLabelIndex(const Value *value);
    /*! Helper function to emit loads and stores */
    template <bool isLoad, typename T> void emitLoadOrStore(T &I);

    // Currently supported instructions
#define DECL_VISIT_FN(NAME, TYPE)         \
    void regAllocate##NAME(TYPE &I);      \
    void emit##NAME(TYPE &I);             \
    void visit##NAME(TYPE &I) {           \
      if (pass == PASS_EMIT_INSTRUCTIONS) \
        emit##NAME(I);                    \
      else                                \
        regAllocate##NAME(I);             \
    }
    DECL_VISIT_FN(BinaryOperator, Instruction);
    DECL_VISIT_FN(CastInst, CastInst);
    DECL_VISIT_FN(ReturnInst, ReturnInst);
    DECL_VISIT_FN(LoadInst, LoadInst);
    DECL_VISIT_FN(StoreInst, StoreInst);
    DECL_VISIT_FN(CallInst, CallInst);
#undef DECL_VISIT_FN

    // Must be implemented later
    void visitInsertElementInst(InsertElementInst &I) {NOT_SUPPORTED;}
    void visitExtractElementInst(ExtractElementInst &I) {NOT_SUPPORTED;}
    void visitShuffleVectorInst(ShuffleVectorInst &SVI) {NOT_SUPPORTED;}
    void visitInsertValueInst(InsertValueInst &I) {NOT_SUPPORTED;}
    void visitExtractValueInst(ExtractValueInst &I) {NOT_SUPPORTED;}
    void visitPHINode(PHINode &I) {NOT_SUPPORTED;}
    void visitBranchInst(BranchInst &I) {NOT_SUPPORTED;}
    void visitICmpInst(ICmpInst &I) {NOT_SUPPORTED;}
    void visitFCmpInst(FCmpInst &I) {NOT_SUPPORTED;}
    void visitSelectInst(SelectInst &I) {NOT_SUPPORTED;}

    // These instructions are not supported at all
    void visitVAArgInst(VAArgInst &I) {NOT_SUPPORTED;}
    void visitSwitchInst(SwitchInst &I) {NOT_SUPPORTED;}
    void visitInvokeInst(InvokeInst &I) {NOT_SUPPORTED;}
    void visitUnwindInst(UnwindInst &I) {NOT_SUPPORTED;}
    void visitResumeInst(ResumeInst &I) {NOT_SUPPORTED;}
    void visitInlineAsm(CallInst &I) {NOT_SUPPORTED;}
    void visitIndirectBrInst(IndirectBrInst &I) {NOT_SUPPORTED;}
    void visitUnreachableInst(UnreachableInst &I) {NOT_SUPPORTED;}
    void visitGetElementPtrInst(GetElementPtrInst &I) {NOT_SUPPORTED;}
    void visitAllocaInst(AllocaInst &I) {NOT_SUPPORTED;}
    template <bool isLoad, typename T> void visitLoadOrStore(T &I);

    void visitInstruction(Instruction &I) {NOT_SUPPORTED;}
  };

  char GenWriter::ID = 0;

  bool GenWriter::doInitialization(Module &M) {
    FunctionPass::doInitialization(M);

    // Initialize
    TheModule = &M;

    TAsm = new CBEMCAsmInfo();
    TD = new TargetData(&M);
    MRI  = new MCRegisterInfo();
    TCtx = new MCContext(*TAsm, *MRI, NULL);
    Mang = new Mangler(*TCtx, *TD);

    return false;
  }

  ir::ImmediateIndex GenWriter::newImmediate(Constant *CPV) {
    if (dyn_cast<ConstantExpr>(CPV))
      GBE_ASSERTM(false, "Unsupported constant expression");
    else if (isa<UndefValue>(CPV) && CPV->getType()->isSingleValueType())
      GBE_ASSERTM(false, "Unsupported constant expression");

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
        GBE_ASSERTM(false, "Unsupported integer size");
        return ctx.newImmediate(uint64_t(0));
      }
    }

    // Floats and doubles
    switch (CPV->getType()->getTypeID()) {
      case Type::FloatTyID:
      case Type::DoubleTyID:
      {
        ConstantFP *FPC = cast<ConstantFP>(CPV);
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
    }
    return ctx.newImmediate(uint64_t(0));
  }

  void GenWriter::newRegister(Value *value) {
    auto type = value->getType();
    auto typeID = type->getTypeID();
    switch (typeID) {
      case Type::IntegerTyID:
      case Type::FloatTyID:
      case Type::DoubleTyID:
      case Type::PointerTyID:
        regTranslator.newScalar(value);
        break;
      case Type::VectorTyID:
      {
        auto vectorType = cast<VectorType>(type);
        const uint32_t elemNum = vectorType->getNumElements();
        for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
          regTranslator.newScalar(value, elemID);
        break;
      }
      default: NOT_SUPPORTED;
    };
  }

  ir::Register GenWriter::getRegister(Value *value, uint32_t index) {
    Constant *CPV = dyn_cast<Constant>(value);
    if (CPV && !isa<GlobalValue>(CPV)) {
      const ir::ImmediateIndex index = this->newImmediate(CPV);
      const ir::Immediate imm = ctx.getImmediate(index);
      const ir::Register reg = ctx.reg(getFamily(imm.type));
      ctx.LOADI(imm.type, reg, index);
      return reg;
    }
    else
      return regTranslator.getScalar(value, index);
  }

  void GenWriter::newLabelIndex(const Value *value) {
    if (labelMap.find(value) == labelMap.end()) {
      const ir::LabelIndex label = ctx.label();
      labelMap[value] = label;
    }
  }

  void GenWriter::emitBasicBlock(BasicBlock *BB) {
    GBE_ASSERT(labelMap.find(BB) != labelMap.end());
    ctx.LABEL(labelMap[BB]);
    for (auto II = BB->begin(), E = BB->end(); II != E; ++II) {
      const Type *Ty = II->getType();
      GBE_ASSERT(!Ty->isIntegerTy() ||
          (Ty==Type::getInt1Ty(II->getContext())  ||
           Ty==Type::getInt8Ty(II->getContext())  ||
           Ty==Type::getInt16Ty(II->getContext()) ||
           Ty==Type::getInt32Ty(II->getContext()) ||
           Ty==Type::getInt64Ty(II->getContext())));
      visit(*II);
    }
  }

  void GenWriter::emitFunctionPrototype(Function &F)
  {
    const bool returnStruct = F.hasStructRetAttr();

    // Loop over the arguments and output registers for them
    if (!F.arg_empty()) {
      Function::arg_iterator I = F.arg_begin(), E = F.arg_end();

      // When a struct is returned, first argument is pointer to the structure
      if (returnStruct)
        ctx.getFunction().setStructReturned(true);

      // Insert a new register for each function argument
      for (; I != E; ++I) {
        const Type *type = I->getType();
        GBE_ASSERT(isScalarType(type) == true);
        const ir::Register reg = regTranslator.newScalar(I);
        ctx.input(reg);
      }
    }

    // When returning a structure, first input register is the pointer to the
    // structure
    if (!returnStruct) {
      const Type *type = F.getReturnType();
      if (type->isVoidTy() == false) {
        const ir::RegisterData::Family family = getFamily(ctx, type);
        const ir::Register reg = ctx.reg(family);
        ctx.output(reg);
      }
    }

#if GBE_DEBUG
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

  void GenWriter::emitFunction(Function &F)
  {
    ctx.startFunction(F.getName());
    this->regTranslator.clear();
    this->labelMap.clear();
    this->emitFunctionPrototype(F);

    // Visit all the instructions and emit the IR registers or the value to
    // value mapping
    pass = PASS_EMIT_REGISTERS;
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
      visit(*I);

    // First create all the labels (one per block)
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      this->newLabelIndex(BB);

    // ... then, emit the instructions for all basic blocks
    pass = PASS_EMIT_INSTRUCTIONS;
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      emitBasicBlock(BB);
    ctx.endFunction();
  }

  void GenWriter::regAllocateReturnInst(ReturnInst &I) {}

  void GenWriter::emitReturnInst(ReturnInst &I) {
    const ir::Function &fn = ctx.getFunction();
    GBE_ASSERTM(fn.outputNum() <= 1, "no more than one value can be returned");
    if (fn.outputNum() == 1 && I.getNumOperands() > 0) {
      const ir::Register dst = fn.getOutput(0);
      const ir::Register src = this->getRegister(I.getOperand(0));
      const ir::RegisterData::Family family = fn.getRegisterFamiy(dst);;
      ctx.MOV(ir::getType(family), dst, src);
    }
    ctx.RET();
  }

  void GenWriter::regAllocateBinaryOperator(Instruction &I) {
    this->newRegister(&I);
  }

  void GenWriter::emitBinaryOperator(Instruction &I) {
    GBE_ASSERT(I.getType()->isPointerTy() == false);

    // Get the element type for a vector
    ir::Type type;
    uint32_t elemNum;
    Type *llvmType = I.getType();
    if (llvmType->isVectorTy() == true) {
      VectorType *vectorType = cast<VectorType>(llvmType);
      Type *elementType = vectorType->getElementType();
      elemNum = vectorType->getNumElements();
      type = getType(ctx, elementType);
    } else {
      elemNum = 1;
      type = getType(ctx, llvmType);
    }

    // Emit the instructions in a row
    for (uint32_t elemID = 0; elemID < elemNum; ++elemID) {
      const ir::Register dst = this->getRegister(&I, elemID);
      const ir::Register src0 = this->getRegister(I.getOperand(0), elemID);
      const ir::Register src1 = this->getRegister(I.getOperand(1), elemID);

      switch (I.getOpcode()) {
        case Instruction::Add:
        case Instruction::FAdd: ctx.ADD(type, dst, src0, src1); break;
        case Instruction::Sub:
        case Instruction::FSub: ctx.SUB(type, dst, src0, src1); break;
        case Instruction::Mul:
        case Instruction::FMul: ctx.MUL(type, dst, src0, src1); break;
        case Instruction::URem:
        case Instruction::SRem:
        case Instruction::FRem: ctx.REM(type, dst, src0, src1); break;
        case Instruction::UDiv:
        case Instruction::SDiv:
        case Instruction::FDiv: ctx.DIV(type, dst, src0, src1); break;
        case Instruction::And:  ctx.AND(type, dst, src0, src1); break;
        case Instruction::Or:   ctx.OR(type, dst, src0, src1); break;
        case Instruction::Xor:  ctx.XOR(type, dst, src0, src1); break;
        case Instruction::Shl:  ctx.SHL(type, dst, src0, src1); break;
        case Instruction::LShr: ctx.SHR(type, dst, src0, src1); break;
        case Instruction::AShr: ctx.ASR(type, dst, src0, src1); break;
        default: NOT_SUPPORTED;
      };
    }
  }

  void GenWriter::regAllocateCastInst(CastInst &I)
  {
    if (I.getOpcode() == Instruction::PtrToInt ||
        I.getOpcode() == Instruction::IntToPtr) {
      Value *dstValue = &I;
      Value *srcValue = I.getOperand(0);
      Constant *CPV = dyn_cast<Constant>(srcValue);
      if (CPV == NULL) {
        Type *dstType = dstValue->getType();
        Type *srcType = srcValue->getType();
        GBE_ASSERT(getTypeByteSize(unit, dstType) == getTypeByteSize(unit, srcType));
        regTranslator.newValueProxy(srcValue, dstValue);
      } else
        this->newRegister(dstValue);
    }
    else
      NOT_SUPPORTED;
  }

  void GenWriter::emitCastInst(CastInst &I) {
    if (I.getOpcode() == Instruction::PtrToInt ||
        I.getOpcode() == Instruction::IntToPtr) {
      Value *srcValue = &I;
      Value *dstValue = I.getOperand(0);
      Constant *CPV = dyn_cast<Constant>(srcValue);
      if (CPV != NULL) {
        const ir::ImmediateIndex index = ctx.newImmediate(CPV);
        const ir::Immediate imm = ctx.getImmediate(index);
        const ir::Register reg = this->getRegister(dstValue);
        ctx.LOADI(imm.type, reg, index);
      }
    }
  }

#ifndef NDEBUG
  static bool isSupportedIntegerSize(IntegerType &T) {
    return T.getBitWidth() == 8 || T.getBitWidth() == 16 ||
           T.getBitWidth() == 32 || T.getBitWidth() == 64;
  }
#endif

  void GenWriter::emitCallInst(CallInst &I) {}
  void GenWriter::regAllocateCallInst(CallInst &I) {
    Value *dst = &I;
    Value *Callee = I.getCalledValue();
    GBE_ASSERT(ctx.getFunction().getProfile() == ir::PROFILE_OCL);
    GBE_ASSERT(isa<InlineAsm>(I.getCalledValue()) == false);
    GBE_ASSERT(I.hasStructRetAttr() == false);
#if GBE_DEBUG
    if (Function *F = I.getCalledFunction())
      GBE_ASSERT(F->getIntrinsicID() == 0);
#endif /* GBE_DEBUG */
    // With OCL there is no side effect for any called functions. So do nothing
    // when there is no returned value
    if (I.getType() == Type::getVoidTy(I.getContext()))
      NOT_SUPPORTED;

    // Get the name of the called function and handle it. We should use a hash
    // map later
    const std::string fnName = Callee->getName();
    if (fnName == "__gen_ocl_get_global_id0")
      regTranslator.newScalarProxy(ir::ocl::gid0, dst);
    else if (fnName == "__gen_ocl_get_global_id1")
      regTranslator.newScalarProxy(ir::ocl::gid1, dst);
    else if (fnName == "__gen_ocl_get_global_id2")
      regTranslator.newScalarProxy(ir::ocl::gid2, dst);
    else if (fnName == "__gen_ocl_get_local_id0")
      regTranslator.newScalarProxy(ir::ocl::lid0, dst);
    else if (fnName == "__gen_ocl_get_local_id1")
      regTranslator.newScalarProxy(ir::ocl::lid1, dst);
    else if (fnName == "__gen_ocl_get_local_id2")
      regTranslator.newScalarProxy(ir::ocl::lid2, dst);
  }

  static INLINE ir::MemorySpace addressSpaceLLVMToGen(unsigned llvmMemSpace) {
    switch (llvmMemSpace) {
      case 0: return ir::MEM_GLOBAL;
      case 4: return ir::MEM_LOCAL;
    }
    GBE_ASSERT(false);
    return ir::MEM_GLOBAL;
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

  template <bool isLoad, typename T>
  INLINE void GenWriter::emitLoadOrStore(T &I)
  {
    GBE_ASSERTM(I.isVolatile() == false, "Volatile pointer is not supported");
    unsigned int llvmSpace = I.getPointerAddressSpace();
    Value *llvmPtr = I.getPointerOperand();
    Value *llvmValues = getLoadOrStoreValue(I);
    Type *llvmType = llvmValues->getType();
    const bool dwAligned = (I.getAlignment() % 4) == 0;
    const ir::MemorySpace memSpace = addressSpaceLLVMToGen(llvmSpace);
    const ir::Register ptr = this->getRegister(llvmPtr);

    // Scalar is easy. We neednot build register tuples
    if (isScalarType(llvmType) == true) {
      const ir::Type type = getType(ctx, llvmType);
      const ir::Register values = this->getRegister(llvmValues);
      if (isLoad)
        ctx.LOAD(type, ptr, memSpace, dwAligned, values);
      else
        ctx.STORE(type, ptr, memSpace, dwAligned, values);
    }
    // A vector type requires to build a tuple
    else {
      VectorType *vectorType = cast<VectorType>(llvmType);
      Type *elemType = vectorType->getElementType();

      // Build the tuple data in the vector
      vector<ir::Register> tupleData; // put registers here
      const uint32_t elemNum = vectorType->getNumElements();
      for (uint32_t elemID = 0; elemID < elemNum; ++elemID) {
        const ir::Register reg = this->getRegister(llvmValues, elemID);
        tupleData.push_back(reg);
      }
      const ir::Tuple tuple = ctx.arrayTuple(&tupleData[0], elemNum);

      // Emit the instruction
      const ir::Type type = getType(ctx, elemType);
      if (isLoad)
        ctx.LOAD(type, tuple, ptr, memSpace, elemNum, dwAligned);
      else
        ctx.STORE(type, tuple, ptr, memSpace, elemNum, dwAligned);
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

