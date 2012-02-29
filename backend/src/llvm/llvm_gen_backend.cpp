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

#include "ir/context.hpp"
#include "ir/unit.hpp"
#include "sys/map.hpp"
#include <algorithm>

using namespace llvm;

namespace gbe
{
  class CBEMCAsmInfo : public MCAsmInfo {
  public:
    CBEMCAsmInfo() {
      GlobalPrefix = "";
      PrivateGlobalPrefix = "";
    }
  };

  /// GenWriter - This class is the main chunk of code that converts an LLVM
  /// module to a C translation unit.
  class GenWriter : public FunctionPass, public InstVisitor<GenWriter>
  {
    ir::Unit &unit;
    ir::Context ctx;
    std::string FDOutErr;
    tool_output_file *FDOut;
    formatted_raw_ostream Out;
    IntrinsicLowering *IL;
    Mangler *Mang;
    LoopInfo *LI;
    const Module *TheModule;
    const MCAsmInfo* TAsm;
    const MCRegisterInfo *MRI;
    const MCObjectFileInfo *MOFI;
    MCContext *TCtx;
    const TargetData* TD;

    /*! Map value to ir::Register*/
    map<const Value*, ir::Register> registerMap;

    /*! Map value to ir::LabelIndex */
    map<const Value*, ir::LabelIndex> labelMap;

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
        FDOut(new llvm::tool_output_file("-", FDOutErr, 0)),
        Out(FDOut->os()),
        IL(0), Mang(0), LI(0),
        TheModule(0), TAsm(0), MRI(0), MOFI(0), TCtx(0), TD(0),
        OpaqueCounter(0), NextAnonValueNumber(0)
    {
      initializeLoopInfoPass(*PassRegistry::getPassRegistry());
      FPCounter = 0;
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

      // Get rid of intrinsics we can't handle.
      lowerIntrinsics(F);

      // Output all floating point constants that cannot be printed accurately.
      printFloatingPointConstants(F);

      emitFunction(F);
      return false;
    }

    virtual bool doFinalization(Module &M) {
      // Free memory...
      delete IL;
      delete TD;
      delete Mang;
      delete TCtx;
      delete TAsm;
      delete MRI;
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
    void writeOperandWithCast(Value* Operand, unsigned Opcode);
    void writeOperandWithCast(Value* Operand, const ICmpInst &I);
    bool writeInstructionCast(const Instruction &I);

  private :
    std::string InterpretASMConstraint(InlineAsm::ConstraintInfo& c);

    void lowerIntrinsics(Function &F);
    /// Prints the definition of the intrinsic function F. Supports the 
    /// intrinsics which need to be explicitly defined in the CBackend.
    void printIntrinsicDefinition(const Function &F, raw_ostream &Out);

    void printModuleTypes();
    void printContainedStructs(Type *Ty, SmallPtrSet<Type *, 16> &);
    void printFloatingPointConstants(Function &F);
    void printFloatingPointConstants(const Constant *C);
    void emitFunctionSignature(const Function *F, bool Prototype);

    /*! Emit the complete function code and declaration */
    void emitFunction(Function &F);
    /*! Handle input and output function parameters */
    void emitFunctionPrototype(const Function *F);
    /*! Emit the code for a basic block */
    void emitBasicBlock(BasicBlock *BB);

    /*! Get the register family from the given type */
    INLINE ir::RegisterData::Family getArgumentFamily(const Type*) const;
    /*! Insert a new register when this is a scalar value */
    INLINE void newRegister(const Value *value);
    /*! Return a valid register from an operand (can use LOADI to make one) */
    INLINE ir::Register getRegister(Value *value);
    /*! Insert a new label index when this is a scalar value */
    INLINE void newLabelIndex(const Value *value);
    /*! int / float / double / bool are scalars */
    INLINE bool isScalarType(const Type *type) const;
    /*! Get the Gen IR type from the LLVM type */
    INLINE ir::Type getType(const Type *type) const;

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

    // Instruction visitation functions
    friend class InstVisitor<GenWriter>;

    void visitReturnInst(ReturnInst &I);
    void visitBranchInst(BranchInst &I);

    void visitVAArgInst(VAArgInst &I) {GBE_ASSERTM(false, "Not supported");}
    void visitSwitchInst(SwitchInst &I) {GBE_ASSERTM(false, "Not supported");}
    void visitInvokeInst(InvokeInst &I) {GBE_ASSERTM(false, "Not supported");}
    void visitUnwindInst(UnwindInst &I) {GBE_ASSERTM(false, "Not supported");}
    void visitResumeInst(ResumeInst &I) {GBE_ASSERTM(false, "Not supported");}
    void visitInlineAsm(CallInst &I) {GBE_ASSERTM(false, "Not supported");}
    void visitIndirectBrInst(IndirectBrInst &I) {GBE_ASSERTM(false, "Not supported");}
    void visitUnreachableInst(UnreachableInst &I) {GBE_ASSERTM(false, "Not supported");}


    void visitPHINode(PHINode &I);
    void visitBinaryOperator(Instruction &I);
    void visitICmpInst(ICmpInst &I);
    void visitFCmpInst(FCmpInst &I);

    void visitCastInst (CastInst &I);
    void visitSelectInst(SelectInst &I);
    void visitCallInst (CallInst &I);
    bool visitBuiltinCall(CallInst &I, Intrinsic::ID ID, bool &WroteCallee);

    void visitAllocaInst(AllocaInst &I);
    template <bool isLoad, typename T> void visitLoadOrStore(T &I);
    void visitLoadInst  (LoadInst   &I);
    void visitStoreInst (StoreInst  &I);
    void visitGetElementPtrInst(GetElementPtrInst &I);

    void visitInsertElementInst(InsertElementInst &I);
    void visitExtractElementInst(ExtractElementInst &I);
    void visitShuffleVectorInst(ShuffleVectorInst &SVI);

    void visitInsertValueInst(InsertValueInst &I);
    void visitExtractValueInst(ExtractValueInst &I);

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
    void printGEPExpression(Value *Ptr, gep_type_iterator I,
                            gep_type_iterator E, bool Static);

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
        printGEPExpression(CE->getOperand(0), gep_type_begin(CPV),
                           gep_type_end(CPV), Static);
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

  // Some instructions need to have their result value casted back to the
  // original types because their operands were casted to the expected type.
  // This function takes care of detecting that case and printing the cast
  // for the Instruction.
  bool GenWriter::writeInstructionCast(const Instruction &I) {
    Type *Ty = I.getOperand(0)->getType();
    switch (I.getOpcode()) {
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::Mul:
      // We need to cast integer arithmetic so that it is always performed
      // as unsigned, to avoid undefined behavior on overflow.
    case Instruction::LShr:
    case Instruction::URem:
    case Instruction::UDiv:
      Out << "((";
      printSimpleType(Out, Ty, false);
      Out << ")(";
      return true;
    case Instruction::AShr:
    case Instruction::SRem:
    case Instruction::SDiv:
      Out << "((";
      printSimpleType(Out, Ty, true);
      Out << ")(";
      return true;
    default: break;
    }
    return false;
  }

  // Write the operand with a cast to another type based on the Opcode being used.
  // This will be used in cases where an instruction has specific type
  // requirements (usually signedness) for its operands.
  void GenWriter::writeOperandWithCast(Value* Operand, unsigned Opcode) {

    // Extract the operand's type, we'll need it.
    Type* OpTy = Operand->getType();

    // Indicate whether to do the cast or not.
    bool shouldCast = false;

    // Indicate whether the cast should be to a signed type or not.
    bool castIsSigned = false;

    // Based on the Opcode for which this Operand is being written, determine
    // the new type to which the operand should be casted by setting the value
    // of OpTy. If we change OpTy, also set shouldCast to true.
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
      case Instruction::URem: // Cast to unsigned first
        shouldCast = true;
        castIsSigned = false;
        break;
      case Instruction::GetElementPtr:
      case Instruction::AShr:
      case Instruction::SDiv:
      case Instruction::SRem: // Cast to signed first
        shouldCast = true;
        castIsSigned = true;
        break;
    }

    // Write out the casted operand if we should, otherwise just write the
    // operand.
    if (shouldCast) {
      Out << "((";
      printSimpleType(Out, OpTy, castIsSigned);
      Out << ")";
      writeOperand(Operand);
      Out << ")";
    } else
      writeOperand(Operand);
  }

  // Write the operand with a cast to another type based on the icmp predicate
  // being used.
  void GenWriter::writeOperandWithCast(Value* Operand, const ICmpInst &Cmp) {
    // This has to do a cast to ensure the operand has the right signedness.
    // Also, if the operand is a pointer, we make sure to cast to an integer when
    // doing the comparison both for signedness and so that the C compiler doesn't
    // optimize things like "p < NULL" to false (p may contain an integer value
    // f.e.).
    bool shouldCast = Cmp.isRelational();

    // Write out the casted operand if we should, otherwise just write the
    // operand.
    if (!shouldCast) {
      writeOperand(Operand);
      return;
    }

    // Should this be a signed comparison?  If so, convert to signed.
    bool castIsSigned = Cmp.isSigned();

    // If the operand was a pointer, convert to a large integer type.
    Type* OpTy = Operand->getType();
    if (OpTy->isPointerTy())
      OpTy = TD->getIntPtrType(Operand->getContext());

    Out << "((";
    printSimpleType(Out, OpTy, castIsSigned);
    Out << ")";
    writeOperand(Operand);
    Out << ")";
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

    TD = new TargetData(&M);
    IL = new IntrinsicLowering(*TD);
    IL->AddPrototypes(M);

    TAsm = new CBEMCAsmInfo();
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


  /// printSymbolTable - Run through symbol table looking for type names.  If a
  /// type name is found, emit its declaration...
  ///
  void GenWriter::printModuleTypes() {
    Out << "/* Helper union for bitcasts */\n";
    Out << "typedef union {\n";
    Out << "  unsigned int Int32;\n";
    Out << "  unsigned long long Int64;\n";
    Out << "  float Float;\n";
    Out << "  double Double;\n";
    Out << "} llvmBitCastUnion;\n";

    // Get all of the struct types used in the module.
    std::vector<StructType*> StructTypes;
    TheModule->findUsedStructTypes(StructTypes);

    if (StructTypes.empty()) return;

    Out << "/* Structure forward decls */\n";

    unsigned NextTypeID = 0;
    
    // If any of them are missing names, add a unique ID to UnnamedStructIDs.
    // Print out forward declarations for structure types.
    for (unsigned i = 0, e = StructTypes.size(); i != e; ++i) {
      StructType *ST = StructTypes[i];

      if (ST->isLiteral() || ST->getName().empty())
        UnnamedStructIDs[ST] = NextTypeID++;

      std::string Name = getStructName(ST);

      Out << "typedef struct " << Name << ' ' << Name << ";\n";
    }

    Out << '\n';

    // Keep track of which structures have been printed so far.
    SmallPtrSet<Type *, 16> StructPrinted;

    // Loop over all structures then push them into the stack so they are
    // printed in the correct order.
    //
    Out << "/* Structure contents */\n";
    for (unsigned i = 0, e = StructTypes.size(); i != e; ++i)
      if (StructTypes[i]->isStructTy())
        // Only print out used types!
        printContainedStructs(StructTypes[i], StructPrinted);
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

  INLINE bool GenWriter::isScalarType(const Type *type) const
  {
    return type->isFloatTy() ||
           type->isIntegerTy() ||
           type->isDoubleTy() ||
           type->isPointerTy();
  }

  INLINE ir::Type GenWriter::getType(const Type *type) const
  {
    GBE_ASSERT(this->isScalarType(type));
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

  INLINE ir::RegisterData::Family GenWriter::getArgumentFamily(const Type *type) const
  {
    GBE_ASSERT(this->isScalarType(type) == true); 
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

  void GenWriter::newRegister(const Value *value) {
    if (registerMap.find(value) == registerMap.end()) {
      const Type *type = value->getType();
      const ir::RegisterData::Family family = getArgumentFamily(type);
      const ir::Register reg = ctx.reg(family);
      ctx.input(reg);
      registerMap[value] = reg;
    }
  }

  ir::Register GenWriter::getRegister(Value *value) {
    Constant *CPV = dyn_cast<Constant>(value);
    if (CPV && !isa<GlobalValue>(CPV)) {
      GBE_ASSERT(0);
      // printConstant(CPV, Static);
    } else {
      GBE_ASSERT(this->registerMap.find(value) != this->registerMap.end());
      return this->registerMap[value];
    }
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

  void GenWriter::emitFunctionPrototype(const Function *F)
  {
    const bool returnStruct = F->hasStructRetAttr();

    // Loop over the arguments and output registers for them
    if (!F->arg_empty()) {
      Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();

      // When a struct is returned, first argument is pointer to the structure
      if (returnStruct) {
        ir::Function &fn = ctx.getFunction();
        fn.setStructReturned(true);
      }

      // Insert a new register if we need to
      for (; I != E; ++I) this->newRegister(I);
    }

    // When returning a structure, first input register is the pointer to the
    // structure
    if (!returnStruct) {
      const Type *type = F->getReturnType();
      if (type->isVoidTy() == false) {
        const ir::RegisterData::Family family = getArgumentFamily(type);
        const ir::Register reg = ctx.reg(family);
        ctx.output(reg);
      }
    }

#if GBE_DEBUG
    // Variable number of arguments is not supported
    FunctionType *FT = cast<FunctionType>(F->getFunctionType());
    GBE_ASSERT(FT->isVarArg() == false);
#endif /* GBE_DEBUG */
  }

  void GenWriter::emitFunctionSignature(const Function *F, bool Prototype)
  {
    /// isStructReturn - Should this function actually return a struct by-value?
    bool isStructReturn = F->hasStructRetAttr();

    // Loop over the arguments, printing them...
    FunctionType *FT = cast<FunctionType>(F->getFunctionType());
    const AttrListPtr &PAL = F->getAttributes();

    std::string tstr;
    raw_string_ostream FunctionInnards(tstr);

    // Print out the name...
    FunctionInnards << GetValueName(F) << '(';

    bool PrintedArg = false;
    if (!F->isDeclaration()) {
      if (!F->arg_empty()) {
        Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
        unsigned Idx = 1;

        // If this is a struct-return function, don't print the hidden
        // struct-return argument.
        if (isStructReturn) {
          assert(I != E && "Invalid struct return function!");
          ++I;
          ++Idx;
        }

        std::string ArgName;
        for (; I != E; ++I) {
          if (PrintedArg) FunctionInnards << ", ";
          if (I->hasName() || !Prototype) {
            ArgName = GetValueName(I);
          } else {
            GBE_ASSERT(0);
            ArgName = "";
          }
          Type *ArgTy = I->getType();
          if (PAL.paramHasAttr(Idx, Attribute::ByVal)) {
            ArgTy = cast<PointerType>(ArgTy)->getElementType();
            ByValParams.insert(I);
          }
          printType(FunctionInnards, ArgTy,
              /*isSigned=*/PAL.paramHasAttr(Idx, Attribute::SExt),
              ArgName);
          PrintedArg = true;
          ++Idx;
        }
      }
    } else {
      GBE_ASSERT(0);

      // Loop over the arguments, printing them.
      FunctionType::param_iterator I = FT->param_begin(), E = FT->param_end();
      unsigned Idx = 1;

      // If this is a struct-return function, don't print the hidden
      // struct-return argument.
      if (isStructReturn) {
        assert(I != E && "Invalid struct return function!");
        ++I;
        ++Idx;
      }

      for (; I != E; ++I) {
        if (PrintedArg) FunctionInnards << ", ";
        Type *ArgTy = *I;
        if (PAL.paramHasAttr(Idx, Attribute::ByVal)) {
          assert(ArgTy->isPointerTy());
          ArgTy = cast<PointerType>(ArgTy)->getElementType();
        }
        printType(FunctionInnards, ArgTy,
               /*isSigned=*/PAL.paramHasAttr(Idx, Attribute::SExt));
        PrintedArg = true;
        ++Idx;
      }
    }

    if (!PrintedArg && FT->isVarArg()) {
      FunctionInnards << "int vararg_dummy_arg";
      PrintedArg = true;
    }

    // Finish printing arguments... if this is a vararg function, print the ...,
    // unless there are no known types, in which case, we just emit ().
    //
    if (FT->isVarArg() && PrintedArg) {
      FunctionInnards << ",...";  // Output varargs portion of signature!
    } else if (!FT->isVarArg() && !PrintedArg) {
      FunctionInnards << "void"; // ret() -> ret(void) in C.
    }
    FunctionInnards << ')';

    // Get the return tpe for the function.
    Type *RetTy;
    if (!isStructReturn)
      RetTy = F->getReturnType();
    else {
      // If this is a struct-return function, print the struct-return type.
      RetTy = cast<PointerType>(FT->getParamType(0))->getElementType();
    }

    // Print out the return type and the signature built above.
    printType(Out, RetTy,
              /*isSigned=*/PAL.paramHasAttr(0, Attribute::SExt),
              FunctionInnards.str());
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
    this->registerMap.clear();
    this->labelMap.clear();
    this->emitFunctionPrototype(&F);

    // We create all the register variables
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
      if (I->getType() != Type::getVoidTy(F.getContext()))
        this->newRegister(&*I);

    // First create all the labels (one per block)
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      this->newLabelIndex(BB);

    // ... then, emit the code for all basic blocks
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


  // Specific Instruction type classes... note that all of the casts are
  // necessary because we use the instruction classes as opaque types...
  //
  void GenWriter::visitReturnInst(ReturnInst &I) {
    // If this is a struct return function, return the temporary struct.
    const ir::Function &fn = ctx.getFunction();
    GBE_ASSERTM(fn.outputNum() <= 1, "no more than one value can be returned");
    if (fn.outputNum() == 1 && I.getNumOperands() > 0) {
      const ir::Register dst = fn.getOutput(0);
      const ir::Register src = this->getRegister(I.getOperand(0));
      const ir::RegisterData::Family family = fn.getRegisterFamiy(dst);;
      ctx.MOV(ir::getType(family), dst, src);
    }
    ctx.RET();

    bool isStructReturn = I.getParent()->getParent()->hasStructRetAttr();
    if (isStructReturn) {
      Out << "  return StructReturn;\n";
      return;
    }

    // Don't output a void return if this is the last basic block in the function
    if (I.getNumOperands() == 0 &&
        &*--I.getParent()->getParent()->end() == I.getParent() &&
        !I.getParent()->size() == 1) {
      return;
    }
#if 0
    Out << "  return";
    if (I.getNumOperands()) {
      Out << ' ';
      writeOperand(I.getOperand(0));
    }
    Out << ";\n";
#endif
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

  void GenWriter::printPHICopiesForSuccessor (BasicBlock *CurBlock,
                                            BasicBlock *Successor,
                                            unsigned Indent) {
    for (BasicBlock::iterator I = Successor->begin(); isa<PHINode>(I); ++I) {
      PHINode *PN = cast<PHINode>(I);
      // Now we have to do the printing.
      Value *IV = PN->getIncomingValueForBlock(CurBlock);
      if (!isa<UndefValue>(IV)) {
        Out << std::string(Indent, ' ');
        Out << "  " << GetValueName(I) << "__PHI_TEMPORARY = ";
        writeOperand(IV);
        Out << ";   /* for PHI node */\n";
      }
    }
  }

  void GenWriter::printBranchToBlock(BasicBlock *CurBB, BasicBlock *Succ,
                                   unsigned Indent) {
    if (isGotoCodeNecessary(CurBB, Succ)) {
      Out << std::string(Indent, ' ') << "  goto ";
      writeOperand(Succ);
      Out << ";\n";
    }
  }

  // Branch instruction printing - Avoid printing out a branch to a basic block
  // that immediately succeeds the current one.
  //
  void GenWriter::visitBranchInst(BranchInst &I) {

    if (I.isConditional()) {
      if (isGotoCodeNecessary(I.getParent(), I.getSuccessor(0))) {
        Out << "  if (";
        writeOperand(I.getCondition());
        Out << ") {\n";

        printPHICopiesForSuccessor (I.getParent(), I.getSuccessor(0), 2);
        printBranchToBlock(I.getParent(), I.getSuccessor(0), 2);

        if (isGotoCodeNecessary(I.getParent(), I.getSuccessor(1))) {
          Out << "  } else {\n";
          printPHICopiesForSuccessor (I.getParent(), I.getSuccessor(1), 2);
          printBranchToBlock(I.getParent(), I.getSuccessor(1), 2);
        }
      } else {
        // First goto not necessary, assume second one is...
        Out << "  if (!";
        writeOperand(I.getCondition());
        Out << ") {\n";

        printPHICopiesForSuccessor (I.getParent(), I.getSuccessor(1), 2);
        printBranchToBlock(I.getParent(), I.getSuccessor(1), 2);
      }

      Out << "  }\n";
    } else {
      printPHICopiesForSuccessor (I.getParent(), I.getSuccessor(0), 0);
      printBranchToBlock(I.getParent(), I.getSuccessor(0), 0);
    }
    Out << "\n";
  }

  // PHI nodes get copied into temporary values at the end of predecessor basic
  // blocks.  We now need to copy these temporary values into the REAL value for
  // the PHI.
  void GenWriter::visitPHINode(PHINode &I) {
    writeOperand(&I);
    Out << "__PHI_TEMPORARY";
  }


  void GenWriter::visitBinaryOperator(Instruction &I)
  {
    GBE_ASSERT(!I.getType()->isPointerTy());
    GBE_ASSERT(this->registerMap.find(&I) != this->registerMap.end());
    const ir::Register dst = this->registerMap[&I];
    const ir::Register src0 = this->getRegister(I.getOperand(0));
    const ir::Register src1 = this->getRegister(I.getOperand(1));
    const ir::Type type = this->getType(I.getType());

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

#if 0
    // binary instructions, shift instructions, setCond instructions.
    assert(!I.getType()->isPointerTy());
    // We must cast the results of binary operations which might be promoted.
    bool needsCast = false;
    if ((I.getType() == Type::getInt8Ty(I.getContext())) ||
        (I.getType() == Type::getInt16Ty(I.getContext()))
        || (I.getType() == Type::getFloatTy(I.getContext()))) {
      needsCast = true;
      Out << "((";
      printType(Out, I.getType(), false);
      Out << ")(";
    }

    // If this is a negation operation, print it out as such.  For FP, we don't
    // want to print "-0.0 - X".
    if (BinaryOperator::isNeg(&I)) {
      Out << "-(";
      writeOperand(BinaryOperator::getNegArgument(cast<BinaryOperator>(&I)));
      Out << ")";
    } else if (BinaryOperator::isFNeg(&I)) {
      Out << "-(";
      writeOperand(BinaryOperator::getFNegArgument(cast<BinaryOperator>(&I)));
      Out << ")";
    } else if (I.getOpcode() == Instruction::FRem) {
      // Output a call to fmod/fmodf instead of emitting a%b
      if (I.getType() == Type::getFloatTy(I.getContext()))
        Out << "fmodf(";
      else if (I.getType() == Type::getDoubleTy(I.getContext()))
        Out << "fmod(";
      else  // all 3 flavors of long double
        Out << "fmodl(";
      writeOperand(I.getOperand(0));
      Out << ", ";
      writeOperand(I.getOperand(1));
      Out << ")";
    } else {

      // Write out the cast of the instruction's value back to the proper type
      // if necessary.
      bool NeedsClosingParens = writeInstructionCast(I);

      // Certain instructions require the operand to be forced to a specific type
      // so we use writeOperandWithCast here instead of writeOperand. Similarly
      // below for operand 1
      writeOperandWithCast(I.getOperand(0), I.getOpcode());

      switch (I.getOpcode()) {
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
      case Instruction::And:  Out << " & "; break;
      case Instruction::Or:   Out << " | "; break;
      case Instruction::Xor:  Out << " ^ "; break;
      case Instruction::Shl : Out << " << "; break;
      case Instruction::LShr:
      case Instruction::AShr: Out << " >> "; break;
      default:
#ifndef NDEBUG
         errs() << "Invalid operator type!" << I;
#endif
         llvm_unreachable(0);
      }

      writeOperandWithCast(I.getOperand(1), I.getOpcode());
      if (NeedsClosingParens)
        Out << "))";
    }

    if (needsCast) {
      Out << "))";
    }
#endif
  }

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

  static const char * getFloatBitCastField(Type *Ty) {
    switch (Ty->getTypeID()) {
      default: llvm_unreachable("Invalid Type");
      case Type::FloatTyID:  return "Float";
      case Type::DoubleTyID: return "Double";
      case Type::IntegerTyID: {
        unsigned NumBits = cast<IntegerType>(Ty)->getBitWidth();
        if (NumBits <= 32)
          return "Int32";
        else
          return "Int64";
      }
    }
  }

  void GenWriter::visitCastInst(CastInst &I) {
    Type *DstTy = I.getType();
    Type *SrcTy = I.getOperand(0)->getType();
    if (isFPIntBitCast(I)) {
      Out << '(';
      // These int<->float and long<->double casts need to be handled specially
      Out << GetValueName(&I) << "__BITCAST_TEMPORARY."
          << getFloatBitCastField(I.getOperand(0)->getType()) << " = ";
      writeOperand(I.getOperand(0));
      Out << ", " << GetValueName(&I) << "__BITCAST_TEMPORARY."
          << getFloatBitCastField(I.getType());
      Out << ')';
      return;
    }

    Out << '(';
    printCast(I.getOpcode(), SrcTy, DstTy);

    // Make a sext from i1 work by subtracting the i1 from 0 (an int).
    if (SrcTy == Type::getInt1Ty(I.getContext()) &&
        I.getOpcode() == Instruction::SExt)
      Out << "0-";

    writeOperand(I.getOperand(0));

    if (DstTy == Type::getInt1Ty(I.getContext()) &&
        (I.getOpcode() == Instruction::Trunc ||
         I.getOpcode() == Instruction::FPToUI ||
         I.getOpcode() == Instruction::FPToSI ||
         I.getOpcode() == Instruction::PtrToInt)) {
      // Make sure we really get a trunc to bool by anding the operand with 1
      Out << "&1u";
    }
    Out << ')';
  }

  void GenWriter::visitSelectInst(SelectInst &I) {
    Out << "((";
    writeOperand(I.getCondition());
    Out << ") ? (";
    writeOperand(I.getTrueValue());
    Out << ") : (";
    writeOperand(I.getFalseValue());
    Out << "))";
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

  void GenWriter::lowerIntrinsics(Function &F) {
    // This is used to keep track of intrinsics that get generated to a lowered
    // function. We must generate the prototypes before the function body which
    // will only be expanded on first use (by the loop below).
    std::vector<Function*> prototypesToGen;

    // Examine all the instructions in this function to find the intrinsics that
    // need to be lowered.
    for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE; ++BB)
      for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; )
        if (CallInst *CI = dyn_cast<CallInst>(I++))
          if (Function *F = CI->getCalledFunction())
            switch (F->getIntrinsicID()) {
            case Intrinsic::not_intrinsic:
            case Intrinsic::vastart:
            case Intrinsic::vacopy:
            case Intrinsic::vaend:
            case Intrinsic::returnaddress:
            case Intrinsic::frameaddress:
            case Intrinsic::setjmp:
            case Intrinsic::longjmp:
            case Intrinsic::prefetch:
            case Intrinsic::powi:
            case Intrinsic::x86_sse_cmp_ss:
            case Intrinsic::x86_sse_cmp_ps:
            case Intrinsic::x86_sse2_cmp_sd:
            case Intrinsic::x86_sse2_cmp_pd:
            case Intrinsic::ppc_altivec_lvsl:
            case Intrinsic::uadd_with_overflow:
            case Intrinsic::sadd_with_overflow:
                // We directly implement these intrinsics
              break;
            default:
              // If this is an intrinsic that directly corresponds to a GCC
              // builtin, we handle it.
              const char *BuiltinName = "";
#define GET_GCC_BUILTIN_NAME
#include "llvm/Intrinsics.gen"
#undef GET_GCC_BUILTIN_NAME
              // If we handle it, don't lower it.
              if (BuiltinName[0]) break;

              // All other intrinsic calls we must lower.
              Instruction *Before = 0;
              if (CI != &BB->front())
                Before = prior(BasicBlock::iterator(CI));

              IL->LowerIntrinsicCall(CI);
              if (Before) {        // Move iterator to instruction after call
                I = Before; ++I;
              } else {
                I = BB->begin();
              }
              // If the intrinsic got lowered to another call, and that call has
              // a definition then we need to make sure its prototype is emitted
              // before any calls to it.
              if (CallInst *Call = dyn_cast<CallInst>(I))
                if (Function *NewF = Call->getCalledFunction())
                  if (!NewF->isDeclaration())
                    prototypesToGen.push_back(NewF);

              break;
            }

    // We may have collected some prototypes to emit in the loop above.
    // Emit them now, before the function that uses them is emitted. But,
    // be careful not to emit them twice.
    std::vector<Function*>::iterator I = prototypesToGen.begin();
    std::vector<Function*>::iterator E = prototypesToGen.end();
    for ( ; I != E; ++I) {
      if (intrinsicPrototypesAlreadyGenerated.insert(*I).second) {
        Out << '\n';
        emitFunctionSignature(*I, true);
        Out << ";\n";
      }
    }
  }

  void GenWriter::visitCallInst(CallInst &I) {
    if (isa<InlineAsm>(I.getCalledValue()))
      return visitInlineAsm(I);

    bool WroteCallee = false;

    // Handle intrinsic function calls first...
    if (Function *F = I.getCalledFunction())
      if (Intrinsic::ID ID = (Intrinsic::ID)F->getIntrinsicID())
        if (visitBuiltinCall(I, ID, WroteCallee))
          return;

    Value *Callee = I.getCalledValue();

    PointerType  *PTy   = cast<PointerType>(Callee->getType());
    FunctionType *FTy   = cast<FunctionType>(PTy->getElementType());

    // If this is a call to a struct-return function, assign to the first
    // parameter instead of passing it to the call.
    const AttrListPtr &PAL = I.getAttributes();
    bool hasByVal = I.hasByValArgument();
    bool isStructRet = I.hasStructRetAttr();
    if (isStructRet) {
      writeOperandDeref(I.getArgOperand(0));
      Out << " = ";
    }

    if (I.isTailCall()) Out << " /*tail*/ ";

    if (!WroteCallee) {
      // If this is an indirect call to a struct return function, we need to cast
      // the pointer. Ditto for indirect calls with byval arguments.
      bool NeedsCast = (hasByVal || isStructRet) && !isa<Function>(Callee);

      // GCC is a real PITA.  It does not permit codegening casts of functions to
      // function pointers if they are in a call (it generates a trap instruction
      // instead!).  We work around this by inserting a cast to void* in between
      // the function and the function pointer cast.  Unfortunately, we can't just
      // form the constant expression here, because the folder will immediately
      // nuke it.
      //
      // Note finally, that this is completely unsafe.  ANSI C does not guarantee
      // that void* and function pointers have the same size. :( To deal with this
      // in the common case, we handle casts where the number of arguments passed
      // match exactly.
      //
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(Callee))
        if (CE->isCast())
          if (Function *RF = dyn_cast<Function>(CE->getOperand(0))) {
            NeedsCast = true;
            Callee = RF;
          }

      if (NeedsCast) {
        // Ok, just cast the pointer type.
        Out << "((";
        if (isStructRet)
          printStructReturnPointerFunctionType(Out, PAL,
                               cast<PointerType>(I.getCalledValue()->getType()));
        else if (hasByVal)
          printType(Out, I.getCalledValue()->getType(), false, "", true, PAL);
        else
          printType(Out, I.getCalledValue()->getType());
        Out << ")(void*)";
      }
      writeOperand(Callee);
      if (NeedsCast) Out << ')';
    }

    Out << '(';

    bool PrintedArg = false;
    if(FTy->isVarArg() && !FTy->getNumParams()) {
      Out << "0 /*dummy arg*/";
      PrintedArg = true;
    }

    unsigned NumDeclaredParams = FTy->getNumParams();
    CallSite CS(&I);
    CallSite::arg_iterator AI = CS.arg_begin(), AE = CS.arg_end();
    unsigned ArgNo = 0;
    if (isStructRet) {   // Skip struct return argument.
      ++AI;
      ++ArgNo;
    }


    for (; AI != AE; ++AI, ++ArgNo) {
      if (PrintedArg) Out << ", ";
      if (ArgNo < NumDeclaredParams &&
          (*AI)->getType() != FTy->getParamType(ArgNo)) {
        Out << '(';
        printType(Out, FTy->getParamType(ArgNo),
              /*isSigned=*/PAL.paramHasAttr(ArgNo+1, Attribute::SExt));
        Out << ')';
      }
      // Check if the argument is expected to be passed by value.
      if (I.paramHasAttr(ArgNo+1, Attribute::ByVal))
        writeOperandDeref(*AI);
      else
        writeOperand(*AI);
      PrintedArg = true;
    }
    Out << ')';
  }

  /// visitBuiltinCall - Handle the call to the specified builtin.  Returns true
  /// if the entire call is handled, return false if it wasn't handled, and
  /// optionally set 'WroteCallee' if the callee has already been printed out.
  bool GenWriter::visitBuiltinCall(CallInst &I, Intrinsic::ID ID, bool &WroteCallee) {
    switch (ID) {
    default: {
      // If this is an intrinsic that directly corresponds to a GCC
      // builtin, we emit it here.
      const char *BuiltinName = "";
      Function *F = I.getCalledFunction();
#define GET_GCC_BUILTIN_NAME
#include "llvm/Intrinsics.gen"
#undef GET_GCC_BUILTIN_NAME
      assert(BuiltinName[0] && "Unknown LLVM intrinsic!");

      Out << BuiltinName;
      WroteCallee = true;
      return false;
    }
    case Intrinsic::vastart:
      Out << "0; ";

      Out << "va_start(*(va_list*)";
      writeOperand(I.getArgOperand(0));
      Out << ", ";
      // Output the last argument to the enclosing function.
      if (I.getParent()->getParent()->arg_empty())
        Out << "vararg_dummy_arg";
      else
        writeOperand(--I.getParent()->getParent()->arg_end());
      Out << ')';
      return true;
    case Intrinsic::vaend:
      if (!isa<ConstantPointerNull>(I.getArgOperand(0))) {
        Out << "0; va_end(*(va_list*)";
        writeOperand(I.getArgOperand(0));
        Out << ')';
      } else {
        Out << "va_end(*(va_list*)0)";
      }
      return true;
    case Intrinsic::vacopy:
      Out << "0; ";
      Out << "va_copy(*(va_list*)";
      writeOperand(I.getArgOperand(0));
      Out << ", *(va_list*)";
      writeOperand(I.getArgOperand(1));
      Out << ')';
      return true;
    case Intrinsic::returnaddress:
      Out << "__builtin_return_address(";
      writeOperand(I.getArgOperand(0));
      Out << ')';
      return true;
    case Intrinsic::frameaddress:
      Out << "__builtin_frame_address(";
      writeOperand(I.getArgOperand(0));
      Out << ')';
      return true;
    case Intrinsic::powi:
      Out << "__builtin_powi(";
      writeOperand(I.getArgOperand(0));
      Out << ", ";
      writeOperand(I.getArgOperand(1));
      Out << ')';
      return true;
    case Intrinsic::setjmp:
      Out << "setjmp(*(jmp_buf*)";
      writeOperand(I.getArgOperand(0));
      Out << ')';
      return true;
    case Intrinsic::longjmp:
      Out << "longjmp(*(jmp_buf*)";
      writeOperand(I.getArgOperand(0));
      Out << ", ";
      writeOperand(I.getArgOperand(1));
      Out << ')';
      return true;
    case Intrinsic::prefetch:
      Out << "LLVM_PREFETCH((const void *)";
      writeOperand(I.getArgOperand(0));
      Out << ", ";
      writeOperand(I.getArgOperand(1));
      Out << ", ";
      writeOperand(I.getArgOperand(2));
      Out << ")";
      return true;
    case Intrinsic::stacksave:
      // Emit this as: Val = 0; *((void**)&Val) = __builtin_stack_save()
      // to work around GCC bugs (see PR1809).
      Out << "0; *((void**)&" << GetValueName(&I)
          << ") = __builtin_stack_save()";
      return true;
    case Intrinsic::x86_sse_cmp_ss:
    case Intrinsic::x86_sse_cmp_ps:
    case Intrinsic::x86_sse2_cmp_sd:
    case Intrinsic::x86_sse2_cmp_pd:
      Out << '(';
      printType(Out, I.getType());
      Out << ')';
      // Multiple GCC builtins multiplex onto this intrinsic.
      switch (cast<ConstantInt>(I.getArgOperand(2))->getZExtValue()) {
      default: llvm_unreachable("Invalid llvm.x86.sse.cmp!");
      case 0: Out << "__builtin_ia32_cmpeq"; break;
      case 1: Out << "__builtin_ia32_cmplt"; break;
      case 2: Out << "__builtin_ia32_cmple"; break;
      case 3: Out << "__builtin_ia32_cmpunord"; break;
      case 4: Out << "__builtin_ia32_cmpneq"; break;
      case 5: Out << "__builtin_ia32_cmpnlt"; break;
      case 6: Out << "__builtin_ia32_cmpnle"; break;
      case 7: Out << "__builtin_ia32_cmpord"; break;
      }
      if (ID == Intrinsic::x86_sse_cmp_ps || ID == Intrinsic::x86_sse2_cmp_pd)
        Out << 'p';
      else
        Out << 's';
      if (ID == Intrinsic::x86_sse_cmp_ss || ID == Intrinsic::x86_sse_cmp_ps)
        Out << 's';
      else
        Out << 'd';

      Out << "(";
      writeOperand(I.getArgOperand(0));
      Out << ", ";
      writeOperand(I.getArgOperand(1));
      Out << ")";
      return true;
    case Intrinsic::ppc_altivec_lvsl:
      Out << '(';
      printType(Out, I.getType());
      Out << ')';
      Out << "__builtin_altivec_lvsl(0, (void*)";
      writeOperand(I.getArgOperand(0));
      Out << ")";
      return true;
    case Intrinsic::uadd_with_overflow:
    case Intrinsic::sadd_with_overflow:
      Out << GetValueName(I.getCalledFunction()) << "(";
      writeOperand(I.getArgOperand(0));
      Out << ", ";
      writeOperand(I.getArgOperand(1));
      Out << ")";
      return true;
    }
  }

  //This converts the llvm constraint string to something gcc is expecting.
  //TODO: work out platform independent constraints and factor those out
  //      of the per target tables
  //      handle multiple constraint codes
  std::string GenWriter::InterpretASMConstraint(InlineAsm::ConstraintInfo& c) {
    assert(c.Codes.size() == 1 && "Too many asm constraint codes to handle");

    // Grab the translation table from MCAsmInfo if it exists.
    const MCAsmInfo *TargetAsm;
    std::string Triple = TheModule->getTargetTriple();
    if (Triple.empty())
      Triple = llvm::sys::getHostTriple();

    std::string E;
    if (const Target *Match = TargetRegistry::lookupTarget(Triple, E))
      TargetAsm = Match->createMCAsmInfo(Triple);
    else
      return c.Codes[0];

    const char *const *table = TargetAsm->getAsmCBE();

    // Search the translation table if it exists.
    for (int i = 0; table && table[i]; i += 2)
      if (c.Codes[0] == table[i]) {
        delete TargetAsm;
        return table[i+1];
      }

    // Default is identity.
    delete TargetAsm;
    return c.Codes[0];
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

  void GenWriter::printGEPExpression(Value *Ptr, gep_type_iterator I,
                                     gep_type_iterator E, bool Static) {

    // If there are no indices, just print out the pointer.
    if (I == E) {
      writeOperand(Ptr);
      return;
    }

    // Find out if the last index is into a vector.  If so, we have to print this
    // specially.  Since vectors can't have elements of indexable type, only the
    // last index could possibly be of a vector element.
    VectorType *LastIndexIsVector = 0;
    {
      for (gep_type_iterator TmpI = I; TmpI != E; ++TmpI)
        LastIndexIsVector = dyn_cast<VectorType>(*TmpI);
    }

    Out << "(";

    // If the last index is into a vector, we can't print it as &a[i][j] because
    // we can't index into a vector with j in GCC.  Instead, emit this as
    // (((float*)&a[i])+j)
    if (LastIndexIsVector) {
      Out << "((";
      printType(Out, PointerType::getUnqual(LastIndexIsVector->getElementType()));
      Out << ")(";
    }

    Out << '&';

    // If the first index is 0 (very typical) we can do a number of
    // simplifications to clean up the code.
    Value *FirstOp = I.getOperand();
    if (!isa<Constant>(FirstOp) || !cast<Constant>(FirstOp)->isNullValue()) {
      // First index isn't simple, print it the hard way.
      writeOperand(Ptr);
    } else {
      ++I;  // Skip the zero index.

      // Okay, emit the first operand. If Ptr is something that is already address
      // exposed, like a global, avoid emitting (&foo)[0], just emit foo instead.
      if (isAddressExposed(Ptr)) {
        writeOperandInternal(Ptr, Static);
      } else if (I != E && (*I)->isStructTy()) {
        // If we didn't already emit the first operand, see if we can print it as
        // P->f instead of "P[0].f"
        writeOperand(Ptr);
        Out << "->field" << cast<ConstantInt>(I.getOperand())->getZExtValue();
        ++I;  // eat the struct index as well.
      } else {
        // Instead of emitting P[0][1], emit (*P)[1], which is more idiomatic.
        Out << "(*";
        writeOperand(Ptr);
        Out << ")";
      }
    }

    for (; I != E; ++I) {
      if ((*I)->isStructTy()) {
        Out << ".field" << cast<ConstantInt>(I.getOperand())->getZExtValue();
      } else if ((*I)->isArrayTy()) {
        Out << ".array[";
        writeOperandWithCast(I.getOperand(), Instruction::GetElementPtr);
        Out << ']';
      } else if (!(*I)->isVectorTy()) {
        Out << '[';
        writeOperandWithCast(I.getOperand(), Instruction::GetElementPtr);
        Out << ']';
      } else {
        // If the last index is into a vector, then print it out as "+j)".  This
        // works with the 'LastIndexIsVector' code above.
        if (isa<Constant>(I.getOperand()) &&
            cast<Constant>(I.getOperand())->isNullValue()) {
          Out << "))";  // avoid "+0".
        } else {
          Out << ")+(";
          writeOperandWithCast(I.getOperand(), Instruction::GetElementPtr);
          Out << "))";
        }
      }
    }
    Out << ")";
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

  template <bool isLoad, typename T>
  INLINE void GenWriter::visitLoadOrStore(T &I)
  {
    GBE_ASSERTM(I.isVolatile() == false, "Volatile pointer is not supported");
    unsigned int llvmSpace = I.getPointerAddressSpace();
    Value *llvmPtr = I.getPointerOperand();
    Value *llvmValues = getLoadOrStoreValue(I);
    Type *llvmType = llvmValues->getType();
    const bool dwAligned = (I.getAlignment() % 4) == 0;
    const ir::MemorySpace memSpace = addressSpaceLLVMToGen(llvmSpace);
    const ir::Type type = getType(llvmType);
    const ir::Register values = getRegister(llvmValues);
    const ir::Register ptr = getRegister(llvmPtr);
    if (isLoad)
      ctx.LOAD(type, ptr, memSpace, dwAligned, values);
    else
      ctx.STORE(type, ptr, memSpace, dwAligned, values);
  }

  void GenWriter::visitLoadInst(LoadInst &I) {
    this->visitLoadOrStore<true>(I);
  }

  void GenWriter::visitStoreInst(StoreInst &I) {
    this->visitLoadOrStore<false>(I);
  }

  void GenWriter::visitGetElementPtrInst(GetElementPtrInst &I) {
    printGEPExpression(I.getPointerOperand(), gep_type_begin(I),
                       gep_type_end(I), false);
  }

  void GenWriter::visitInsertElementInst(InsertElementInst &I) {
    Type *EltTy = I.getType()->getElementType();
    writeOperand(I.getOperand(0));
    Out << ";\n  ";
    Out << "((";
    printType(Out, PointerType::getUnqual(EltTy));
    Out << ")(&" << GetValueName(&I) << "))[";
    writeOperand(I.getOperand(2));
    Out << "] = (";
    writeOperand(I.getOperand(1));
    Out << ")";
  }

  void GenWriter::visitExtractElementInst(ExtractElementInst &I) {
    // We know that our operand is not inlined.
    Out << "((";
    Type *EltTy = cast<VectorType>(I.getOperand(0)->getType())->getElementType();
    printType(Out, PointerType::getUnqual(EltTy));
    Out << ")(&" << GetValueName(I.getOperand(0)) << "))[";
    writeOperand(I.getOperand(1));
    Out << "]";
  }

  void GenWriter::visitShuffleVectorInst(ShuffleVectorInst &SVI) {
    Out << "(";
    printType(Out, SVI.getType());
    Out << "){ ";
    VectorType *VT = SVI.getType();
    unsigned NumElts = VT->getNumElements();
    Type *EltTy = VT->getElementType();

    for (unsigned i = 0; i != NumElts; ++i) {
      if (i) Out << ", ";
      int SrcVal = SVI.getMaskValue(i);
      if ((unsigned)SrcVal >= NumElts*2) {
        Out << " 0/*undef*/ ";
      } else {
        Value *Op = SVI.getOperand((unsigned)SrcVal >= NumElts);
        if (isa<Instruction>(Op)) {
          // Do an extractelement of this value from the appropriate input.
          Out << "((";
          printType(Out, PointerType::getUnqual(EltTy));
          Out << ")(&" << GetValueName(Op)
              << "))[" << (SrcVal & (NumElts-1)) << "]";
        } else if (isa<ConstantAggregateZero>(Op) || isa<UndefValue>(Op)) {
          Out << "0";
        } else {
          printConstant(cast<ConstantVector>(Op)->getOperand(SrcVal &
                                                             (NumElts-1)),
                        false);
        }
      }
    }
    Out << "}";
  }

  void GenWriter::visitInsertValueInst(InsertValueInst &IVI) {
    // Start by copying the entire aggregate value into the result variable.
        writeOperand(IVI.getOperand(0));
    Out << ";\n  ";

    // Then do the insert to update the field.
    Out << GetValueName(&IVI);
    for (const unsigned *b = IVI.idx_begin(), *i = b, *e = IVI.idx_end();
         i != e; ++i) {
      Type *IndexedTy =
        ExtractValueInst::getIndexedType(IVI.getOperand(0)->getType(),
                                         makeArrayRef(b, i+1));
      if (IndexedTy->isArrayTy())
        Out << ".array[" << *i << "]";
      else
        Out << ".field" << *i;
    }
    Out << " = ";
    writeOperand(IVI.getOperand(1));
  }

  void GenWriter::visitExtractValueInst(ExtractValueInst &EVI) {
    Out << "(";
    if (isa<UndefValue>(EVI.getOperand(0))) {
      Out << "(";
      printType(Out, EVI.getType());
      Out << ") 0/*UNDEF*/";
    } else {
      Out << GetValueName(EVI.getOperand(0));
      for (const unsigned *b = EVI.idx_begin(), *i = b, *e = EVI.idx_end();
           i != e; ++i) {
        Type *IndexedTy =
          ExtractValueInst::getIndexedType(EVI.getOperand(0)->getType(),
                                           makeArrayRef(b, i+1));
        if (IndexedTy->isArrayTy())
          Out << ".array[" << *i << "]";
        else
          Out << ".field" << *i;
      }
    }
    Out << ")";
  }

  llvm::FunctionPass *createGenPass(ir::Unit &unit) {
    return new GenWriter(unit);
  }

} /* namespace gbe */

