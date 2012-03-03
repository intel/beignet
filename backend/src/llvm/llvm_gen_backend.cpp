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

    std::string FDOutErr;
    tool_output_file *FDOut;
    formatted_raw_ostream Out;
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
        FDOut(new llvm::tool_output_file("-", FDOutErr, 0)),
        Out(FDOut->os()),
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

      // Output all floating point constants that cannot be printed accurately.
      printFloatingPointConstants(F);

      emitFunction(F);
      return false;
    }

    virtual bool doFinalization(Module &M) {
      // Free memory...
      delete TD;
      delete Mang;
      delete MOFI;
      FPConstantMap.clear();
      ByValParams.clear();
      intrinsicPrototypesAlreadyGenerated.clear();
      UnnamedStructIDs.clear();
      FDOut->keep();
      return false;
    }

    raw_ostream &printType(raw_ostream &Out, Type *Ty,
                           bool isSigned = false,
                           const std::string &VariableName = "",
                           bool IgnoreName = false,
                           const AttrListPtr &PAL = AttrListPtr());
    raw_ostream &printSimpleType(raw_ostream &Out, Type *Ty,
                                 bool isSigned,
                                 const std::string &NameSoFar = "");

    void printStructReturnPointerFunctionType(raw_ostream &Out,
                                              const AttrListPtr &PAL,
                                              PointerType *Ty);

    std::string getStructName(StructType *ST);

    /// writeOperandDeref - Print the result of dereferencing the specified
    /// operand with '*'.  This is equivalent to printing '*' then using
    /// writeOperand, but avoids excess syntax in some cases.
    void writeOperandDeref(Value *Operand) {
      if (isAddressExposed(Operand)) {
        // Already something with an address exposed.
        writeOperandInternal(Operand);
      } else {
        Out << "*(";
        writeOperand(Operand);
        Out << ")";
      }
    }

    void writeOperand(Value *Operand, bool Static = false);
    void writeInstComputationInline(Instruction &I);
    void writeOperandInternal(Value *Operand, bool Static = false);

    /// Prints the definition of the intrinsic function F. Supports the 
    /// intrinsics which need to be explicitly defined in the CBackend.
    void printIntrinsicDefinition(const Function &F, raw_ostream &Out);

    void printContainedStructs(Type *Ty, SmallPtrSet<Type *, 16> &);
    void printFloatingPointConstants(Function &F);
    void printFloatingPointConstants(const Constant *C);

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

    void printBasicBlock(BasicBlock *BB);

    void printCast(unsigned opcode, Type *SrcTy, Type *DstTy);
    void printConstant(Constant *CPV, bool Static);
    void printConstantWithCast(Constant *CPV, unsigned Opcode);
    bool printConstExprCast(const ConstantExpr *CE, bool Static);
    void printConstantArray(ConstantArray *CPA, bool Static);
    void printConstantVector(ConstantVector *CV, bool Static);

    /// isAddressExposed - Return true if the specified value's name needs to
    /// have its address taken in order to get a C value of the correct type.
    /// This happens for global variables, byval parameters, and direct allocas.
    bool isAddressExposed(const Value *V) const {
      if (const Argument *A = dyn_cast<Argument>(V))
        return ByValParams.count(A);
      return isa<GlobalVariable>(V) || isDirectAlloca(V);
    }

    // isInlinableInst - Attempt to inline instructions into their uses to build
    // trees as much as possible.  To do this, we have to consistently decide
    // what is acceptable to inline, so that variable declarations don't get
    // printed and an extra copy of the expr is not emitted.
    //
    static bool isInlinableInst(const Instruction &I) {
      // Always inline cmp instructions, even if they are shared by multiple
      // expressions.  GCC generates horrible code if we don't.
      if (isa<CmpInst>(I))
        return true;

      // Must be an expression, must be used exactly once.  If it is dead, we
      // emit it inline where it would go.
      if (I.getType() == Type::getVoidTy(I.getContext()) || !I.hasOneUse() ||
          isa<TerminatorInst>(I) || isa<CallInst>(I) || isa<PHINode>(I) ||
          isa<LoadInst>(I) || isa<VAArgInst>(I) || isa<InsertElementInst>(I) ||
          isa<InsertValueInst>(I))
        // Don't inline a load across a store or other bad things!
        return false;

      // Must not be used in inline asm, extractelement, or shufflevector.
      if (I.hasOneUse()) {
        const Instruction &User = cast<Instruction>(*I.use_back());
        if (isInlineAsm(User) || isa<ExtractElementInst>(User) ||
            isa<ShuffleVectorInst>(User))
          return false;
      }

      // Only inline instruction it if it's use is in the same BB as the inst.
      return I.getParent() == cast<Instruction>(I.use_back())->getParent();
    }

    // isDirectAlloca - Define fixed sized allocas in the entry block as direct
    // variables which are accessed with the & operator.  This causes GCC to
    // generate significantly better code than to emit alloca calls directly.
    //
    static const AllocaInst *isDirectAlloca(const Value *V) {
      const AllocaInst *AI = dyn_cast<AllocaInst>(V);
      if (!AI) return 0;
      if (AI->isArrayAllocation())
        return 0;   // FIXME: we can also inline fixed size array allocas!
      if (AI->getParent() != &AI->getParent()->getParent()->getEntryBlock())
        return 0;
      return AI;
    }

    // isInlineAsm - Check if the instruction is a call to an inline asm chunk.
    static bool isInlineAsm(const Instruction& I) {
      if (const CallInst *CI = dyn_cast<CallInst>(&I))
        return isa<InlineAsm>(CI->getCalledValue());
      return false;
    }

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


    void visitAllocaInst(AllocaInst &I);
    template <bool isLoad, typename T> void visitLoadOrStore(T &I);


    void visitInstruction(Instruction &I) {
#ifndef NDEBUG
      errs() << "C Writer does not know about " << I;
#endif
      llvm_unreachable(0);
    }

    void outputLValue(Instruction *I) {
      Out << "  " << GetValueName(I) << " = ";
    }

    bool isGotoCodeNecessary(BasicBlock *From, BasicBlock *To);
    void printPHICopiesForSuccessor(BasicBlock *CurBlock,
                                    BasicBlock *Successor, unsigned Indent);
    void printBranchToBlock(BasicBlock *CurBlock, BasicBlock *SuccBlock,
                            unsigned Indent);

    std::string GetValueName(const Value *Operand);
  };

char GenWriter::ID = 0;
#define PRINT_CODE 1

static std::string CBEMangle(const std::string &S) {
  std::string Result;

  for (unsigned i = 0, e = S.size(); i != e; ++i)
    if (isalnum(S[i]) || S[i] == '_') {
      Result += S[i];
    } else {
      Result += '_';
      Result += 'A'+(S[i]&15);
      Result += 'A'+((S[i]>>4)&15);
      Result += '_';
    }
  return Result;
}

  std::string GenWriter::getStructName(StructType *ST) {
    if (!ST->isLiteral() && !ST->getName().empty())
      return CBEMangle("l_"+ST->getName().str());
    return "l_unnamed_" + utostr(UnnamedStructIDs[ST]);
  }


  /// printStructReturnPointerFunctionType - This is like printType for a struct
  /// return type, except, instead of printing the type as void (*)(Struct*, ...)
  /// print it as "Struct (*)(...)", for struct return functions.
  void GenWriter::printStructReturnPointerFunctionType(raw_ostream &Out,
                                                     const AttrListPtr &PAL,
                                                     PointerType *TheTy) {
    FunctionType *FTy = cast<FunctionType>(TheTy->getElementType());
    std::string tstr;
    raw_string_ostream FunctionInnards(tstr);
    FunctionInnards << " (*) (";
    bool PrintedType = false;

    FunctionType::param_iterator I = FTy->param_begin(), E = FTy->param_end();
    Type *RetTy = cast<PointerType>(*I)->getElementType();
    unsigned Idx = 1;
    for (++I, ++Idx; I != E; ++I, ++Idx) {
      if (PrintedType)
        FunctionInnards << ", ";
      Type *ArgTy = *I;
      if (PAL.paramHasAttr(Idx, Attribute::ByVal)) {
        assert(ArgTy->isPointerTy());
        ArgTy = cast<PointerType>(ArgTy)->getElementType();
      }
      printType(FunctionInnards, ArgTy,
          /*isSigned=*/PAL.paramHasAttr(Idx, Attribute::SExt), "");
      PrintedType = true;
    }
    if (FTy->isVarArg()) {
      if (!PrintedType)
        FunctionInnards << " int"; //dummy argument for empty vararg functs
      FunctionInnards << ", ...";
    } else if (!PrintedType) {
      FunctionInnards << "void";
    }
    FunctionInnards << ')';
    printType(Out, RetTy,
        /*isSigned=*/PAL.paramHasAttr(0, Attribute::SExt), FunctionInnards.str());
  }

  raw_ostream &
  GenWriter::printSimpleType(raw_ostream &Out, Type *Ty, bool isSigned,
                             const std::string &NameSoFar) {
    assert((Ty->isPrimitiveType() || Ty->isIntegerTy() || Ty->isVectorTy()) &&
           "Invalid type for printSimpleType");
    switch (Ty->getTypeID()) {
    case Type::VoidTyID:   return Out << "void " << NameSoFar;
    case Type::IntegerTyID: {
      unsigned NumBits = cast<IntegerType>(Ty)->getBitWidth();
      if (NumBits == 1)
        return Out << "bool " << NameSoFar;
      else if (NumBits <= 8)
        return Out << (isSigned?"signed":"unsigned") << " char " << NameSoFar;
      else if (NumBits <= 16)
        return Out << (isSigned?"signed":"unsigned") << " short " << NameSoFar;
      else if (NumBits <= 32)
        return Out << (isSigned?"signed":"unsigned") << " int " << NameSoFar;
      else if (NumBits <= 64)
        return Out << (isSigned?"signed":"unsigned") << " long long "<< NameSoFar;
      else {
        assert(NumBits <= 128 && "Bit widths > 128 not implemented yet");
        return Out << (isSigned?"llvmInt128":"llvmUInt128") << " " << NameSoFar;
      }
    }
    case Type::FloatTyID:  return Out << "float "   << NameSoFar;
    case Type::DoubleTyID: return Out << "double "  << NameSoFar;
    // Lacking emulation of FP80 on PPC, etc., we assume whichever of these is
    // present matches host 'long double'.
    case Type::X86_FP80TyID:
    case Type::PPC_FP128TyID:
    case Type::FP128TyID:  return Out << "long double " << NameSoFar;

    case Type::X86_MMXTyID:
      return printSimpleType(Out, Type::getInt32Ty(Ty->getContext()), isSigned,
                       " __attribute__((vector_size(64))) " + NameSoFar);

    case Type::VectorTyID: {
      VectorType *VTy = cast<VectorType>(Ty);
      return printSimpleType(Out, VTy->getElementType(), isSigned,
                       " __attribute__((vector_size(" +
                       utostr(TD->getTypeAllocSize(VTy)) + " ))) " + NameSoFar);
    }

    default:
#ifndef NDEBUG
      errs() << "Unknown primitive type: " << *Ty << "\n";
#endif
      llvm_unreachable(0);
    }
  }

  // Pass the Type* and the variable name and this prints out the variable
  // declaration.
  //
  raw_ostream &GenWriter::printType(raw_ostream &Out, Type *Ty,
                                  bool isSigned, const std::string &NameSoFar,
                                  bool IgnoreName, const AttrListPtr &PAL) {
    if (Ty->isPrimitiveType() || Ty->isIntegerTy() || Ty->isVectorTy()) {
      printSimpleType(Out, Ty, isSigned, NameSoFar);
      return Out;
    }

    switch (Ty->getTypeID()) {
    case Type::FunctionTyID: {
      FunctionType *FTy = cast<FunctionType>(Ty);
      std::string tstr;
      raw_string_ostream FunctionInnards(tstr);
      FunctionInnards << " (" << NameSoFar << ") (";
      unsigned Idx = 1;
      for (FunctionType::param_iterator I = FTy->param_begin(),
             E = FTy->param_end(); I != E; ++I) {
        Type *ArgTy = *I;
        if (PAL.paramHasAttr(Idx, Attribute::ByVal)) {
          assert(ArgTy->isPointerTy());
          ArgTy = cast<PointerType>(ArgTy)->getElementType();
        }
        if (I != FTy->param_begin())
          FunctionInnards << ", ";
        printType(FunctionInnards, ArgTy,
          /*isSigned=*/PAL.paramHasAttr(Idx, Attribute::SExt), "");
        ++Idx;
      }
      if (FTy->isVarArg()) {
        if (!FTy->getNumParams())
          FunctionInnards << " int"; //dummy argument for empty vaarg functs
        FunctionInnards << ", ...";
      } else if (!FTy->getNumParams()) {
        FunctionInnards << "void";
      }
      FunctionInnards << ')';
      printType(Out, FTy->getReturnType(),
        /*isSigned=*/PAL.paramHasAttr(0, Attribute::SExt), FunctionInnards.str());
      return Out;
    }
    case Type::StructTyID: {
      StructType *STy = cast<StructType>(Ty);
      
      // Check to see if the type is named.
      if (!IgnoreName)
        return Out << getStructName(STy) << ' ' << NameSoFar;
      
      Out << NameSoFar + " {\n";
      unsigned Idx = 0;
      for (StructType::element_iterator I = STy->element_begin(),
             E = STy->element_end(); I != E; ++I) {
        Out << "  ";
        printType(Out, *I, false, "field" + utostr(Idx++));
        Out << ";\n";
      }
      Out << '}';
      if (STy->isPacked())
        Out << " __attribute__ ((packed))";
      return Out;
    }

    case Type::PointerTyID: {
      PointerType *PTy = cast<PointerType>(Ty);
      std::string ptrName = "*" + NameSoFar;

      if (PTy->getElementType()->isArrayTy() ||
          PTy->getElementType()->isVectorTy())
        ptrName = "(" + ptrName + ")";

      if (!PAL.isEmpty())
        // Must be a function ptr cast!
        return printType(Out, PTy->getElementType(), false, ptrName, true, PAL);
      return printType(Out, PTy->getElementType(), false, ptrName);
    }

    case Type::ArrayTyID: {
      ArrayType *ATy = cast<ArrayType>(Ty);
      unsigned NumElements = ATy->getNumElements();
      if (NumElements == 0) NumElements = 1;
      // Arrays are wrapped in structs to allow them to have normal
      // value semantics (avoiding the array "decay").
      Out << NameSoFar << " { ";
      printType(Out, ATy->getElementType(), false,
                "array[" + utostr(NumElements) + "]");
      return Out << "; }";
    }

    default:
      llvm_unreachable("Unhandled case in getTypeProps!");
    }

    return Out;
  }

  void GenWriter::printConstantArray(ConstantArray *CPA, bool Static) {

    // As a special case, print the array as a string if it is an array of
    // ubytes or an array of sbytes with positive values.
    //
    Type *ETy = CPA->getType()->getElementType();
    bool isString = (ETy == Type::getInt8Ty(CPA->getContext()) ||
                     ETy == Type::getInt8Ty(CPA->getContext()));

    // Make sure the last character is a null char, as automatically added by C
    if (isString && (CPA->getNumOperands() == 0 ||
                     !cast<Constant>(*(CPA->op_end()-1))->isNullValue()))
      isString = false;

    if (isString) {
      Out << '\"';
      // Keep track of whether the last number was a hexadecimal escape.
      bool LastWasHex = false;

      // Do not include the last character, which we know is null
      for (unsigned i = 0, e = CPA->getNumOperands()-1; i != e; ++i) {
        unsigned char C = cast<ConstantInt>(CPA->getOperand(i))->getZExtValue();

        // Print it out literally if it is a printable character.  The only thing
        // to be careful about is when the last letter output was a hex escape
        // code, in which case we have to be careful not to print out hex digits
        // explicitly (the C compiler thinks it is a continuation of the previous
        // character, sheesh...)
        //
        if (isprint(C) && (!LastWasHex || !isxdigit(C))) {
          LastWasHex = false;
          if (C == '"' || C == '\\')
            Out << "\\" << (char)C;
          else
            Out << (char)C;
        } else {
          LastWasHex = false;
          switch (C) {
          case '\n': Out << "\\n"; break;
          case '\t': Out << "\\t"; break;
          case '\r': Out << "\\r"; break;
          case '\v': Out << "\\v"; break;
          case '\a': Out << "\\a"; break;
          case '\"': Out << "\\\""; break;
          case '\'': Out << "\\\'"; break;
          default:
            Out << "\\x";
            Out << (char)(( C/16  < 10) ? ( C/16 +'0') : ( C/16 -10+'A'));
            Out << (char)(((C&15) < 10) ? ((C&15)+'0') : ((C&15)-10+'A'));
            LastWasHex = true;
            break;
          }
        }
      }
      Out << '\"';
    } else {
      Out << '{';
      if (CPA->getNumOperands()) {
        Out << ' ';
        printConstant(cast<Constant>(CPA->getOperand(0)), Static);
        for (unsigned i = 1, e = CPA->getNumOperands(); i != e; ++i) {
          Out << ", ";
          printConstant(cast<Constant>(CPA->getOperand(i)), Static);
        }
      }
      Out << " }";
    }
  }

  void GenWriter::printConstantVector(ConstantVector *CP, bool Static) {
    Out << '{';
    if (CP->getNumOperands()) {
      Out << ' ';
      printConstant(cast<Constant>(CP->getOperand(0)), Static);
      for (unsigned i = 1, e = CP->getNumOperands(); i != e; ++i) {
        Out << ", ";
        printConstant(cast<Constant>(CP->getOperand(i)), Static);
      }
    }
    Out << " }";
  }

  // isFPCSafeToPrint - Returns true if we may assume that CFP may be written out
  // textually as a double (rather than as a reference to a stack-allocated
  // variable). We decide this by converting CFP to a string and back into a
  // double, and then checking whether the conversion results in a bit-equal
  // double to the original value of CFP. This depends on us and the target C
  // compiler agreeing on the conversion process (which is pretty likely since we
  // only deal in IEEE FP).
  //
  static bool isFPCSafeToPrint(const ConstantFP *CFP) {
    bool ignored;
    // Do long doubles in hex for now.
    if (CFP->getType() != Type::getFloatTy(CFP->getContext()) &&
        CFP->getType() != Type::getDoubleTy(CFP->getContext()))
      return false;
    APFloat APF = APFloat(CFP->getValueAPF());  // copy
    if (CFP->getType() == Type::getFloatTy(CFP->getContext()))
      APF.convert(APFloat::IEEEdouble, APFloat::rmNearestTiesToEven, &ignored);
#if HAVE_PRINTF_A && ENABLE_CBE_PRINTF_A
    char Buffer[100];
    sprintf(Buffer, "%a", APF.convertToDouble());
    if (!strncmp(Buffer, "0x", 2) ||
        !strncmp(Buffer, "-0x", 3) ||
        !strncmp(Buffer, "+0x", 3))
      return APF.bitwiseIsEqual(APFloat(atof(Buffer)));
    return false;
#else
    std::string StrVal = ftostr(APF);

    while (StrVal[0] == ' ')
      StrVal.erase(StrVal.begin());

    // Check to make sure that the stringized number is not some string like "Inf"
    // or NaN.  Check that the string matches the "[-+]?[0-9]" regex.
    if ((StrVal[0] >= '0' && StrVal[0] <= '9') ||
        ((StrVal[0] == '-' || StrVal[0] == '+') &&
         (StrVal[1] >= '0' && StrVal[1] <= '9')))
      // Reparse stringized version!
      return APF.bitwiseIsEqual(APFloat(atof(StrVal.c_str())));
    return false;
#endif
  }

  /// Print out the casting for a cast operation. This does the double casting
  /// necessary for conversion to the destination type, if necessary.
  /// @brief Print a cast
  void GenWriter::printCast(unsigned opc, Type *SrcTy, Type *DstTy) {
    // Print the destination type cast
    switch (opc) {
      case Instruction::UIToFP:
      case Instruction::SIToFP:
      case Instruction::IntToPtr:
      case Instruction::Trunc:
      case Instruction::BitCast:
      case Instruction::FPExt:
      case Instruction::FPTrunc: // For these the DstTy sign doesn't matter
        Out << '(';
        printType(Out, DstTy);
        Out << ')';
        break;
      case Instruction::ZExt:
      case Instruction::PtrToInt:
      case Instruction::FPToUI: // For these, make sure we get an unsigned dest
        Out << '(';
        printSimpleType(Out, DstTy, false);
        Out << ')';
        break;
      case Instruction::SExt:
      case Instruction::FPToSI: // For these, make sure we get a signed dest
        Out << '(';
        printSimpleType(Out, DstTy, true);
        Out << ')';
        break;
      default:
        llvm_unreachable("Invalid cast opcode");
    }

    // Print the source type cast
    switch (opc) {
      case Instruction::UIToFP:
      case Instruction::ZExt:
        Out << '(';
        printSimpleType(Out, SrcTy, false);
        Out << ')';
        break;
      case Instruction::SIToFP:
      case Instruction::SExt:
        Out << '(';
        printSimpleType(Out, SrcTy, true);
        Out << ')';
        break;
      case Instruction::IntToPtr:
      case Instruction::PtrToInt:
        // Avoid "cast to pointer from integer of different size" warnings
        Out << "(unsigned long)";
        break;
      case Instruction::Trunc:
      case Instruction::BitCast:
      case Instruction::FPExt:
      case Instruction::FPTrunc:
      case Instruction::FPToSI:
      case Instruction::FPToUI:
        break; // These don't need a source cast.
      default:
        llvm_unreachable("Invalid cast opcode");
        break;
    }
  }

  // printConstant - The LLVM Constant to C Constant converter.
  void GenWriter::printConstant(Constant *CPV, bool Static) {
    if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(CPV)) {
      switch (CE->getOpcode()) {
      case Instruction::Trunc:
      case Instruction::ZExt:
      case Instruction::SExt:
      case Instruction::FPTrunc:
      case Instruction::FPExt:
      case Instruction::UIToFP:
      case Instruction::SIToFP:
      case Instruction::FPToUI:
      case Instruction::FPToSI:
      case Instruction::PtrToInt:
      case Instruction::IntToPtr:
      case Instruction::BitCast:
        Out << "(";
        printCast(CE->getOpcode(), CE->getOperand(0)->getType(), CE->getType());
        if (CE->getOpcode() == Instruction::SExt &&
            CE->getOperand(0)->getType() == Type::getInt1Ty(CPV->getContext())) {
          // Make sure we really sext from bool here by subtracting from 0
          Out << "0-";
        }
        printConstant(CE->getOperand(0), Static);
        if (CE->getType() == Type::getInt1Ty(CPV->getContext()) &&
            (CE->getOpcode() == Instruction::Trunc ||
             CE->getOpcode() == Instruction::FPToUI ||
             CE->getOpcode() == Instruction::FPToSI ||
             CE->getOpcode() == Instruction::PtrToInt)) {
          // Make sure we really truncate to bool here by anding with 1
          Out << "&1u";
        }
        Out << ')';
        return;

      case Instruction::GetElementPtr:
        Out << "(";
        //printGEPExpression(CE->getOperand(0), gep_type_begin(CPV),
         //                  gep_type_end(CPV), Static);
        Out << ")";
        return;
      case Instruction::Select:
        Out << '(';
        printConstant(CE->getOperand(0), Static);
        Out << '?';
        printConstant(CE->getOperand(1), Static);
        Out << ':';
        printConstant(CE->getOperand(2), Static);
        Out << ')';
        return;
      case Instruction::Add:
      case Instruction::FAdd:
      case Instruction::Sub:
      case Instruction::FSub:
      case Instruction::Mul:
      case Instruction::FMul:
      case Instruction::SDiv:
      case Instruction::UDiv:
      case Instruction::FDiv:
      case Instruction::URem:
      case Instruction::SRem:
      case Instruction::FRem:
      case Instruction::And:
      case Instruction::Or:
      case Instruction::Xor:
      case Instruction::ICmp:
      case Instruction::Shl:
      case Instruction::LShr:
      case Instruction::AShr:
      {
        Out << '(';
        bool NeedsClosingParens = printConstExprCast(CE, Static);
        printConstantWithCast(CE->getOperand(0), CE->getOpcode());
        switch (CE->getOpcode()) {
        case Instruction::Add:
        case Instruction::FAdd: Out << " + "; break;
        case Instruction::Sub:
        case Instruction::FSub: Out << " - "; break;
        case Instruction::Mul:
        case Instruction::FMul: Out << " * "; break;
        case Instruction::URem:
        case Instruction::SRem:
        case Instruction::FRem: Out << " % "; break;
        case Instruction::UDiv:
        case Instruction::SDiv:
        case Instruction::FDiv: Out << " / "; break;
        case Instruction::And: Out << " & "; break;
        case Instruction::Or:  Out << " | "; break;
        case Instruction::Xor: Out << " ^ "; break;
        case Instruction::Shl: Out << " << "; break;
        case Instruction::LShr:
        case Instruction::AShr: Out << " >> "; break;
        case Instruction::ICmp:
          switch (CE->getPredicate()) {
            case ICmpInst::ICMP_EQ: Out << " == "; break;
            case ICmpInst::ICMP_NE: Out << " != "; break;
            case ICmpInst::ICMP_SLT:
            case ICmpInst::ICMP_ULT: Out << " < "; break;
            case ICmpInst::ICMP_SLE:
            case ICmpInst::ICMP_ULE: Out << " <= "; break;
            case ICmpInst::ICMP_SGT:
            case ICmpInst::ICMP_UGT: Out << " > "; break;
            case ICmpInst::ICMP_SGE:
            case ICmpInst::ICMP_UGE: Out << " >= "; break;
            default: llvm_unreachable("Illegal ICmp predicate");
          }
          break;
        default: llvm_unreachable("Illegal opcode here!");
        }
        printConstantWithCast(CE->getOperand(1), CE->getOpcode());
        if (NeedsClosingParens)
          Out << "))";
        Out << ')';
        return;
      }
      case Instruction::FCmp: {
        Out << '(';
        bool NeedsClosingParens = printConstExprCast(CE, Static);
        if (CE->getPredicate() == FCmpInst::FCMP_FALSE)
          Out << "0";
        else if (CE->getPredicate() == FCmpInst::FCMP_TRUE)
          Out << "1";
        else {
          const char* op = 0;
          switch (CE->getPredicate()) {
          default: llvm_unreachable("Illegal FCmp predicate");
          case FCmpInst::FCMP_ORD: op = "ord"; break;
          case FCmpInst::FCMP_UNO: op = "uno"; break;
          case FCmpInst::FCMP_UEQ: op = "ueq"; break;
          case FCmpInst::FCMP_UNE: op = "une"; break;
          case FCmpInst::FCMP_ULT: op = "ult"; break;
          case FCmpInst::FCMP_ULE: op = "ule"; break;
          case FCmpInst::FCMP_UGT: op = "ugt"; break;
          case FCmpInst::FCMP_UGE: op = "uge"; break;
          case FCmpInst::FCMP_OEQ: op = "oeq"; break;
          case FCmpInst::FCMP_ONE: op = "one"; break;
          case FCmpInst::FCMP_OLT: op = "olt"; break;
          case FCmpInst::FCMP_OLE: op = "ole"; break;
          case FCmpInst::FCMP_OGT: op = "ogt"; break;
          case FCmpInst::FCMP_OGE: op = "oge"; break;
          }
          Out << "llvm_fcmp_" << op << "(";
          printConstantWithCast(CE->getOperand(0), CE->getOpcode());
          Out << ", ";
          printConstantWithCast(CE->getOperand(1), CE->getOpcode());
          Out << ")";
        }
        if (NeedsClosingParens)
          Out << "))";
        Out << ')';
        return;
      }
      default:
#ifndef NDEBUG
        errs() << "GenWriter Error: Unhandled constant expression: "
             << *CE << "\n";
#endif
        llvm_unreachable(0);
      }
    } else if (isa<UndefValue>(CPV) && CPV->getType()->isSingleValueType()) {
      Out << "((";
      printType(Out, CPV->getType()); // sign doesn't matter
      Out << ")/*UNDEF*/";
      if (!CPV->getType()->isVectorTy()) {
        Out << "0)";
      } else {
        Out << "{})";
      }
      return;
    }

    if (ConstantInt *CI = dyn_cast<ConstantInt>(CPV)) {
      Type* Ty = CI->getType();
      if (Ty == Type::getInt1Ty(CPV->getContext()))
        Out << (CI->getZExtValue() ? '1' : '0');
      else if (Ty == Type::getInt32Ty(CPV->getContext()))
        Out << CI->getZExtValue() << 'u';
      else if (Ty->getPrimitiveSizeInBits() > 32)
        Out << CI->getZExtValue() << "ull";
      else {
        Out << "((";
        printSimpleType(Out, Ty, false) << ')';
        if (CI->isMinValue(true))
          Out << CI->getZExtValue() << 'u';
        else
          Out << CI->getSExtValue();
        Out << ')';
      }
      return;
    }

    switch (CPV->getType()->getTypeID()) {
    case Type::FloatTyID:
    case Type::DoubleTyID:
    case Type::X86_FP80TyID:
    case Type::PPC_FP128TyID:
    case Type::FP128TyID: {
      ConstantFP *FPC = cast<ConstantFP>(CPV);
      std::map<const ConstantFP*, unsigned>::iterator I = FPConstantMap.find(FPC);
      if (I != FPConstantMap.end()) {
        // Because of FP precision problems we must load from a stack allocated
        // value that holds the value in hex.
        Out << "(*(" << (FPC->getType() == Type::getFloatTy(CPV->getContext()) ?
                         "float" :
                         FPC->getType() == Type::getDoubleTy(CPV->getContext()) ?
                         "double" :
                         "long double")
            << "*)&FPConstant" << I->second << ')';
      } else {
        double V;
        if (FPC->getType() == Type::getFloatTy(CPV->getContext()))
          V = FPC->getValueAPF().convertToFloat();
        else if (FPC->getType() == Type::getDoubleTy(CPV->getContext()))
          V = FPC->getValueAPF().convertToDouble();
        else {
          // Long double.  Convert the number to double, discarding precision.
          // This is not awesome, but it at least makes the CBE output somewhat
          // useful.
          APFloat Tmp = FPC->getValueAPF();
          bool LosesInfo;
          Tmp.convert(APFloat::IEEEdouble, APFloat::rmTowardZero, &LosesInfo);
          V = Tmp.convertToDouble();
        }

        if (IsNAN(V)) {
          // The value is NaN

          // FIXME the actual NaN bits should be emitted.
          // The prefix for a quiet NaN is 0x7FF8. For a signalling NaN,
          // it's 0x7ff4.
          const unsigned long QuietNaN = 0x7ff8UL;
          //const unsigned long SignalNaN = 0x7ff4UL;

          // We need to grab the first part of the FP #
          char Buffer[100];

          uint64_t ll = DoubleToBits(V);
          sprintf(Buffer, "0x%llx", static_cast<long long>(ll));

          std::string Num(&Buffer[0], &Buffer[6]);
          unsigned long Val = strtoul(Num.c_str(), 0, 16);

          if (FPC->getType() == Type::getFloatTy(FPC->getContext()))
            Out << "LLVM_NAN" << (Val == QuietNaN ? "" : "S") << "F(\""
                << Buffer << "\") /*nan*/ ";
          else
            Out << "LLVM_NAN" << (Val == QuietNaN ? "" : "S") << "(\""
                << Buffer << "\") /*nan*/ ";
        } else if (IsInf(V)) {
          // The value is Inf
          if (V < 0) Out << '-';
          Out << "LLVM_INF" <<
              (FPC->getType() == Type::getFloatTy(FPC->getContext()) ? "F" : "")
              << " /*inf*/ ";
        } else {
          std::string Num;
#if HAVE_PRINTF_A && ENABLE_CBE_PRINTF_A
          // Print out the constant as a floating point number.
          char Buffer[100];
          sprintf(Buffer, "%a", V);
          Num = Buffer;
#else
          Num = ftostr(FPC->getValueAPF());
#endif
         Out << Num;
        }
      }
      break;
    }

    case Type::ArrayTyID:
      // Use C99 compound expression literal initializer syntax.
      if (!Static) {
        Out << "(";
        printType(Out, CPV->getType());
        Out << ")";
      }
      Out << "{ "; // Arrays are wrapped in struct types.
      if (ConstantArray *CA = dyn_cast<ConstantArray>(CPV)) {
        printConstantArray(CA, Static);
      } else {
        assert(isa<ConstantAggregateZero>(CPV) || isa<UndefValue>(CPV));
        ArrayType *AT = cast<ArrayType>(CPV->getType());
        Out << '{';
        if (AT->getNumElements()) {
          Out << ' ';
          Constant *CZ = Constant::getNullValue(AT->getElementType());
          printConstant(CZ, Static);
          for (unsigned i = 1, e = AT->getNumElements(); i != e; ++i) {
            Out << ", ";
            printConstant(CZ, Static);
          }
        }
        Out << " }";
      }
      Out << " }"; // Arrays are wrapped in struct types.
      break;

    case Type::VectorTyID:
      // Use C99 compound expression literal initializer syntax.
      if (!Static) {
        Out << "(";
        printType(Out, CPV->getType());
        Out << ")";
      }
      if (ConstantVector *CV = dyn_cast<ConstantVector>(CPV)) {
        printConstantVector(CV, Static);
      } else {
        assert(isa<ConstantAggregateZero>(CPV) || isa<UndefValue>(CPV));
        VectorType *VT = cast<VectorType>(CPV->getType());
        Out << "{ ";
        Constant *CZ = Constant::getNullValue(VT->getElementType());
        printConstant(CZ, Static);
        for (unsigned i = 1, e = VT->getNumElements(); i != e; ++i) {
          Out << ", ";
          printConstant(CZ, Static);
        }
        Out << " }";
      }
      break;

    case Type::StructTyID:
      // Use C99 compound expression literal initializer syntax.
      if (!Static) {
        Out << "(";
        printType(Out, CPV->getType());
        Out << ")";
      }
      if (isa<ConstantAggregateZero>(CPV) || isa<UndefValue>(CPV)) {
        StructType *ST = cast<StructType>(CPV->getType());
        Out << '{';
        if (ST->getNumElements()) {
          Out << ' ';
          printConstant(Constant::getNullValue(ST->getElementType(0)), Static);
          for (unsigned i = 1, e = ST->getNumElements(); i != e; ++i) {
            Out << ", ";
            printConstant(Constant::getNullValue(ST->getElementType(i)), Static);
          }
        }
        Out << " }";
      } else {
        Out << '{';
        if (CPV->getNumOperands()) {
          Out << ' ';
          printConstant(cast<Constant>(CPV->getOperand(0)), Static);
          for (unsigned i = 1, e = CPV->getNumOperands(); i != e; ++i) {
            Out << ", ";
            printConstant(cast<Constant>(CPV->getOperand(i)), Static);
          }
        }
        Out << " }";
      }
      break;

    case Type::PointerTyID:
      if (isa<ConstantPointerNull>(CPV)) {
        Out << "((";
        printType(Out, CPV->getType()); // sign doesn't matter
        Out << ")/*NULL*/0)";
        break;
      } else if (GlobalValue *GV = dyn_cast<GlobalValue>(CPV)) {
        writeOperand(GV, Static);
        break;
      }
      // FALL THROUGH
    default:
#ifndef NDEBUG
      errs() << "Unknown constant type: " << *CPV << "\n";
#endif
      llvm_unreachable(0);
    }
  }

  // Some constant expressions need to be casted back to the original types
  // because their operands were casted to the expected type. This function takes
  // care of detecting that case and printing the cast for the ConstantExpr.
  bool GenWriter::printConstExprCast(const ConstantExpr* CE, bool Static) {
    bool NeedsExplicitCast = false;
    Type *Ty = CE->getOperand(0)->getType();
    bool TypeIsSigned = false;
    switch (CE->getOpcode()) {
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::Mul:
      // We need to cast integer arithmetic so that it is always performed
      // as unsigned, to avoid undefined behavior on overflow.
    case Instruction::LShr:
    case Instruction::URem:
    case Instruction::UDiv: NeedsExplicitCast = true; break;
    case Instruction::AShr:
    case Instruction::SRem:
    case Instruction::SDiv: NeedsExplicitCast = true; TypeIsSigned = true; break;
    case Instruction::SExt:
      Ty = CE->getType();
      NeedsExplicitCast = true;
      TypeIsSigned = true;
      break;
    case Instruction::ZExt:
    case Instruction::Trunc:
    case Instruction::FPTrunc:
    case Instruction::FPExt:
    case Instruction::UIToFP:
    case Instruction::SIToFP:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::PtrToInt:
    case Instruction::IntToPtr:
    case Instruction::BitCast:
      Ty = CE->getType();
      NeedsExplicitCast = true;
      break;
    default: break;
    }
    if (NeedsExplicitCast) {
      Out << "((";
      if (Ty->isIntegerTy() && Ty != Type::getInt1Ty(Ty->getContext()))
        printSimpleType(Out, Ty, TypeIsSigned);
      else
        printType(Out, Ty); // not integer, sign doesn't matter
      Out << ")(";
    }
    return NeedsExplicitCast;
  }

  //  Print a constant assuming that it is the operand for a given Opcode. The
  //  opcodes that care about sign need to cast their operands to the expected
  //  type before the operation proceeds. This function does the casting.
  void GenWriter::printConstantWithCast(Constant* CPV, unsigned Opcode) {

    // Extract the operand's type, we'll need it.
    Type* OpTy = CPV->getType();

    // Indicate whether to do the cast or not.
    bool shouldCast = false;
    bool typeIsSigned = false;

    // Based on the Opcode for which this Constant is being written, determine
    // the new type to which the operand should be casted by setting the value
    // of OpTy. If we change OpTy, also set shouldCast to true so it gets
    // casted below.
    switch (Opcode) {
      default:
        // for most instructions, it doesn't matter
        break;
      case Instruction::Add:
      case Instruction::Sub:
      case Instruction::Mul:
        // We need to cast integer arithmetic so that it is always performed
        // as unsigned, to avoid undefined behavior on overflow.
      case Instruction::LShr:
      case Instruction::UDiv:
      case Instruction::URem:
        shouldCast = true;
        break;
      case Instruction::AShr:
      case Instruction::SDiv:
      case Instruction::SRem:
        shouldCast = true;
        typeIsSigned = true;
        break;
    }

    // Write out the casted constant if we should, otherwise just write the
    // operand.
    if (shouldCast) {
      Out << "((";
      printSimpleType(Out, OpTy, typeIsSigned);
      Out << ")";
      printConstant(CPV, false);
      Out << ")";
    } else
      printConstant(CPV, false);
  }

  std::string GenWriter::GetValueName(const Value *Operand) {

    // Resolve potential alias.
    if (const GlobalAlias *GA = dyn_cast<GlobalAlias>(Operand)) {
      if (const Value *V = GA->resolveAliasedGlobal(false))
        Operand = V;
    }

    // Mangle globals with the standard mangler interface for LLC compatibility.
    if (const GlobalValue *GV = dyn_cast<GlobalValue>(Operand)) {
      SmallString<128> Str;
      Mang->getNameWithPrefix(Str, GV, false);
      return CBEMangle(Str.str().str());
    }

    std::string Name = Operand->getName();

    if (Name.empty()) { // Assign unique names to local temporaries.
      unsigned &No = AnonValueNumbers[Operand];
      if (No == 0)
        No = ++NextAnonValueNumber;
      Name = "tmp__" + utostr(No);
    }

    std::string VarName;
    VarName.reserve(Name.capacity());

    for (std::string::iterator I = Name.begin(), E = Name.end();
         I != E; ++I) {
      char ch = *I;

      if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') || ch == '_')) {
        char buffer[5];
        sprintf(buffer, "_%x_", ch);
        VarName += buffer;
      } else
        VarName += ch;
    }

    return "llvm_gen_" + VarName;
  }

  /// writeInstComputationInline - Emit the computation for the specified
  /// instruction inline, with no destination provided.
  void GenWriter::writeInstComputationInline(Instruction &I) {
    // We can't currently support integer types other than 1, 8, 16, 32, 64.
    // Validate this.
    Type *Ty = I.getType();
    if (Ty->isIntegerTy() && (Ty!=Type::getInt1Ty(I.getContext()) &&
          Ty!=Type::getInt8Ty(I.getContext()) &&
          Ty!=Type::getInt16Ty(I.getContext()) &&
          Ty!=Type::getInt32Ty(I.getContext()) &&
          Ty!=Type::getInt64Ty(I.getContext()))) {
        report_fatal_error("The C backend does not currently support integer "
                          "types of widths other than 1, 8, 16, 32, 64.\n"
                          "This is being tracked as PR 4158.");
    }

    // If this is a non-trivial bool computation, make sure to truncate down to
    // a 1 bit value.  This is important because we want "add i1 x, y" to return
    // "0" when x and y are true, not "2" for example.
    bool NeedBoolTrunc = false;
    if (I.getType() == Type::getInt1Ty(I.getContext()) &&
        !isa<ICmpInst>(I) && !isa<FCmpInst>(I))
      NeedBoolTrunc = true;

    if (NeedBoolTrunc)
      Out << "((";

    visit(I);

    if (NeedBoolTrunc)
      Out << ")&1)";
  }


  void GenWriter::writeOperandInternal(Value *Operand, bool Static) {
    if (Instruction *I = dyn_cast<Instruction>(Operand))
      // Should we inline this instruction to build a tree?
      if (isInlinableInst(*I) && !isDirectAlloca(I)) {
        Out << '(';
        writeInstComputationInline(*I);
        Out << ')';
        return;
      }

    Constant* CPV = dyn_cast<Constant>(Operand);

    if (CPV && !isa<GlobalValue>(CPV))
      printConstant(CPV, Static);
    else
      Out << GetValueName(Operand);
  }

  void GenWriter::writeOperand(Value *Operand, bool Static) {
    bool isAddressImplicit = isAddressExposed(Operand);
    if (isAddressImplicit)
      Out << "(&";  // Global variables are referenced as their addresses by llvm

    writeOperandInternal(Operand, Static);

    if (isAddressImplicit)
      Out << ')';
  }

  enum SpecialGlobalClass {
    NotSpecial = 0,
    GlobalCtors, GlobalDtors,
    NotPrinted
  };

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

  /// Output all floating point constants that cannot be printed accurately...
  void GenWriter::printFloatingPointConstants(Function &F) {
    // Scan the module for floating point constants.  If any FP constant is used
    // in the function, we want to redirect it here so that we do not depend on
    // the precision of the printed form, unless the printed form preserves
    // precision.
    //
    for (constant_iterator I = constant_begin(&F), E = constant_end(&F);
         I != E; ++I)
      printFloatingPointConstants(*I);

    Out << '\n';
  }

  void GenWriter::printFloatingPointConstants(const Constant *C) {
    // If this is a constant expression, recursively check for constant fp values.
    if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(C)) {
      for (unsigned i = 0, e = CE->getNumOperands(); i != e; ++i)
        printFloatingPointConstants(CE->getOperand(i));
      return;
    }

    // Otherwise, check for a FP constant that we need to print.
    const ConstantFP *FPC = dyn_cast<ConstantFP>(C);
    if (FPC == 0 ||
        // Do not put in FPConstantMap if safe.
        isFPCSafeToPrint(FPC) ||
        // Already printed this constant?
        FPConstantMap.count(FPC))
      return;

    FPConstantMap[FPC] = FPCounter;  // Number the FP constants

    if (FPC->getType() == Type::getDoubleTy(FPC->getContext())) {
      double Val = FPC->getValueAPF().convertToDouble();
      uint64_t i = FPC->getValueAPF().bitcastToAPInt().getZExtValue();
      Out << "static const ConstantDoubleTy FPConstant" << FPCounter++
      << " = 0x" << utohexstr(i)
      << "ULL;    /* " << Val << " */\n";
    } else if (FPC->getType() == Type::getFloatTy(FPC->getContext())) {
      float Val = FPC->getValueAPF().convertToFloat();
      uint32_t i = (uint32_t)FPC->getValueAPF().bitcastToAPInt().
      getZExtValue();
      Out << "static const ConstantFloatTy FPConstant" << FPCounter++
      << " = 0x" << utohexstr(i)
      << "U;    /* " << Val << " */\n";
    } else if (FPC->getType() == Type::getX86_FP80Ty(FPC->getContext())) {
      // api needed to prevent premature destruction
      APInt api = FPC->getValueAPF().bitcastToAPInt();
      const uint64_t *p = api.getRawData();
      Out << "static const ConstantFP80Ty FPConstant" << FPCounter++
      << " = { 0x" << utohexstr(p[0])
      << "ULL, 0x" << utohexstr((uint16_t)p[1]) << ",{0,0,0}"
      << "}; /* Long double constant */\n";
    } else if (FPC->getType() == Type::getPPC_FP128Ty(FPC->getContext()) ||
               FPC->getType() == Type::getFP128Ty(FPC->getContext())) {
      APInt api = FPC->getValueAPF().bitcastToAPInt();
      const uint64_t *p = api.getRawData();
      Out << "static const ConstantFP128Ty FPConstant" << FPCounter++
      << " = { 0x"
      << utohexstr(p[0]) << ", 0x" << utohexstr(p[1])
      << "}; /* Long double constant */\n";

    } else {
      llvm_unreachable("Unknown float type!");
    }
  }

  // Push the struct onto the stack and recursively push all structs
  // this one depends on.
  //
  // TODO:  Make this work properly with vector types
  //
  void GenWriter::printContainedStructs(Type *Ty,
                                  SmallPtrSet<Type *, 16> &StructPrinted) {
    // Don't walk through pointers.
    if (Ty->isPointerTy() || Ty->isPrimitiveType() || Ty->isIntegerTy())
      return;

    // Print all contained types first.
    for (Type::subtype_iterator I = Ty->subtype_begin(),
         E = Ty->subtype_end(); I != E; ++I)
      printContainedStructs(*I, StructPrinted);

    if (StructType *ST = dyn_cast<StructType>(Ty)) {
      // Check to see if we have already printed this struct.
      if (!StructPrinted.insert(Ty)) return;
      
      // Print structure type out.
      printType(Out, ST, false, getStructName(ST), true);
      Out << ";\n\n";
    }
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
    ctx.startFunction(GetValueName(&F));
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

  void GenWriter::printBasicBlock(BasicBlock *BB) {

    // Don't print the label for the basic block if there are no uses, or if
    // the only terminator use is the predecessor basic block's terminator.
    // We have to scan the use list because PHI nodes use basic blocks too but
    // do not require a label to be generated.
    //
    bool NeedsLabel = false;
    for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI)
      if (isGotoCodeNecessary(*PI, BB)) {
        NeedsLabel = true;
        break;
      }

    if (NeedsLabel) Out << GetValueName(BB) << ":\n";

    // Output all of the instructions in the basic block...
    for (BasicBlock::iterator II = BB->begin(), E = --BB->end(); II != E;
         ++II) {
      if (!isInlinableInst(*II) && !isDirectAlloca(II)) {
        if (II->getType() != Type::getVoidTy(BB->getContext()) &&
            !isInlineAsm(*II))
          outputLValue(II);
        else
          Out << "  ";
        writeInstComputationInline(*II);
        Out << ";\n";
      }
    }

    // Don't emit prefix or suffix for the terminator.
    visit(*BB->getTerminator());
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

  bool GenWriter::isGotoCodeNecessary(BasicBlock *From, BasicBlock *To) {
    /// FIXME: This should be reenabled, but loop reordering safe!!
    return true;

    if (llvm::next(Function::iterator(From)) != Function::iterator(To))
      return true;  // Not the direct successor, we need a goto.

    //isa<SwitchInst>(From->getTerminator())

    if (LI->getLoopFor(From) != LI->getLoopFor(To))
      return true;
    return false;
  }

  void GenWriter::regAllocateBinaryOperator(Instruction &I)
  {
    this->newRegister(&I);
  }

  void GenWriter::emitBinaryOperator(Instruction &I)
  {
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
        case Instruction::Shl : ctx.SHL(type, dst, src0, src1); break;
        case Instruction::LShr: ctx.SHR(type, dst, src0, src1); break;
        case Instruction::AShr: ctx.ASR(type, dst, src0, src1); break;
        default:
           GBE_ASSERT(0);
      };
    }
  }

#if 0
  void GenWriter::visitICmpInst(ICmpInst &I) {
    // We must cast the results of icmp which might be promoted.
    bool needsCast = false;

    // Write out the cast of the instruction's value back to the proper type
    // if necessary.
    bool NeedsClosingParens = writeInstructionCast(I);

    // Certain icmp predicate require the operand to be forced to a specific type
    // so we use writeOperandWithCast here instead of writeOperand. Similarly
    // below for operand 1
    writeOperandWithCast(I.getOperand(0), I);

    switch (I.getPredicate()) {
    case ICmpInst::ICMP_EQ:  Out << " == "; break;
    case ICmpInst::ICMP_NE:  Out << " != "; break;
    case ICmpInst::ICMP_ULE:
    case ICmpInst::ICMP_SLE: Out << " <= "; break;
    case ICmpInst::ICMP_UGE:
    case ICmpInst::ICMP_SGE: Out << " >= "; break;
    case ICmpInst::ICMP_ULT:
    case ICmpInst::ICMP_SLT: Out << " < "; break;
    case ICmpInst::ICMP_UGT:
    case ICmpInst::ICMP_SGT: Out << " > "; break;
    default:
#ifndef NDEBUG
      errs() << "Invalid icmp predicate!" << I;
#endif
      llvm_unreachable(0);
    }

    writeOperandWithCast(I.getOperand(1), I);
    if (NeedsClosingParens)
      Out << "))";

    if (needsCast) {
      Out << "))";
    }
  }

  void GenWriter::visitFCmpInst(FCmpInst &I) {
    if (I.getPredicate() == FCmpInst::FCMP_FALSE) {
      Out << "0";
      return;
    }
    if (I.getPredicate() == FCmpInst::FCMP_TRUE) {
      Out << "1";
      return;
    }

    const char* op = 0;
    switch (I.getPredicate()) {
    default: llvm_unreachable("Illegal FCmp predicate");
    case FCmpInst::FCMP_ORD: op = "ord"; break;
    case FCmpInst::FCMP_UNO: op = "uno"; break;
    case FCmpInst::FCMP_UEQ: op = "ueq"; break;
    case FCmpInst::FCMP_UNE: op = "une"; break;
    case FCmpInst::FCMP_ULT: op = "ult"; break;
    case FCmpInst::FCMP_ULE: op = "ule"; break;
    case FCmpInst::FCMP_UGT: op = "ugt"; break;
    case FCmpInst::FCMP_UGE: op = "uge"; break;
    case FCmpInst::FCMP_OEQ: op = "oeq"; break;
    case FCmpInst::FCMP_ONE: op = "one"; break;
    case FCmpInst::FCMP_OLT: op = "olt"; break;
    case FCmpInst::FCMP_OLE: op = "ole"; break;
    case FCmpInst::FCMP_OGT: op = "ogt"; break;
    case FCmpInst::FCMP_OGE: op = "oge"; break;
    }

    Out << "llvm_fcmp_" << op << "(";
    // Write the first operand
    writeOperand(I.getOperand(0));
    Out << ", ";
    // Write the second operand
    writeOperand(I.getOperand(1));
    Out << ")";
  }
#endif

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

  void GenWriter::emitCastInst(CastInst &I)
  {
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

  void GenWriter::printIntrinsicDefinition(const Function &F, raw_ostream &Out) {
#ifndef NDEBUG
    FunctionType *funT = F.getFunctionType();
    Type *retT = F.getReturnType();
    IntegerType *elemT = cast<IntegerType>(funT->getParamType(1));

    assert(isSupportedIntegerSize(*elemT) &&
           "CBackend does not support arbitrary size integers.");
    assert(cast<StructType>(retT)->getElementType(0) == elemT &&
           elemT == funT->getParamType(0) && funT->getNumParams() == 2);

    switch (F.getIntrinsicID()) {
    default:
      llvm_unreachable("Unsupported Intrinsic.");
    }
#endif
  }

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

  void GenWriter::visitAllocaInst(AllocaInst &I) {
    Out << '(';
    printType(Out, I.getType());
    Out << ") alloca(sizeof(";
    printType(Out, I.getType()->getElementType());
    Out << ')';
    if (I.isArrayAllocation()) {
      Out << " * " ;
      writeOperand(I.getOperand(0));
    }
    Out << ')';
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

