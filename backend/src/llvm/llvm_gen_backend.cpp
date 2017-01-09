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

#include "llvm_includes.hpp"

#include "llvm/llvm_gen_backend.hpp"
#include "ir/context.hpp"
#include "ir/unit.hpp"
#include "ir/half.hpp"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "sys/set.hpp"
#include "sys/cvar.hpp"
#include "backend/program.h"
#include <sstream>
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfo.h"

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
  extern bool OCL_DEBUGINFO; // first defined by calling BVAR in program.cpp
  /*! Gen IR manipulates only scalar types */
  static bool isScalarType(const Type *type)
  {
    return type->isFloatTy()   ||
           type->isHalfTy()    ||
           type->isIntegerTy() ||
           type->isDoubleTy()  ||
           type->isPointerTy();
  }

  static std::string getTypeName(ir::Context &ctx, const Type *type, int sign)
  {
    GBE_ASSERT(isScalarType(type));
    if (type->isFloatTy() == true)
      return "float";
    if (type->isHalfTy() == true)
      return "half";
    if (type->isDoubleTy() == true)
      return "double";

    GBE_ASSERT(type->isIntegerTy() == true);
    if(sign) {
      if (type == Type::getInt1Ty(type->getContext()))
        return "char";
      if (type == Type::getInt8Ty(type->getContext()))
        return "char";
      if (type == Type::getInt16Ty(type->getContext()))
        return "short";
      if (type == Type::getInt32Ty(type->getContext()))
        return "int";
      if (type == Type::getInt64Ty(type->getContext()))
        return "long";
    }
    else
    {
      if (type == Type::getInt1Ty(type->getContext()))
        return "uchar";
      if (type == Type::getInt8Ty(type->getContext()))
        return "uchar";
      if (type == Type::getInt16Ty(type->getContext()))
        return "ushort";
      if (type == Type::getInt32Ty(type->getContext()))
        return "uint";
      if (type == Type::getInt64Ty(type->getContext()))
        return "ulong";
    }
    GBE_ASSERTM(false, "Unsupported type.");
    return "";
  }

  /*! LLVM IR Type to Gen IR type translation */
  static ir::Type getType(ir::Context &ctx, const Type *type)
  {
    GBE_ASSERT(isScalarType(type));
    if (type->isFloatTy() == true)
      return ir::TYPE_FLOAT;
    if (type->isHalfTy() == true)
      return ir::TYPE_HALF;
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
    if (type == Type::getInt16Ty(type->getContext()) || type->isHalfTy())
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
  static ir::Type getVectorInfo(ir::Context &ctx, Value *value, uint32_t &elemNum, bool useUnsigned = false)
  {
    ir::Type type;
    Type *llvmType = value->getType();
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
      case 4: return ir::MEM_GENERIC;
    }
    GBE_ASSERT(false);
    return ir::MEM_GLOBAL;
  }

  static INLINE ir::AddressSpace btiToGen(const unsigned bti) {
    switch (bti) {
      case BTI_CONSTANT: return ir::MEM_CONSTANT;
      case BTI_PRIVATE: return  ir::MEM_PRIVATE;
      case BTI_LOCAL: return ir::MEM_LOCAL;
      default: return ir::MEM_GLOBAL;
    }
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

#define TYPESIZE(TYPE,VECT,SZ) else if( name == std::string(#TYPE).append(" __attribute__((ext_vector_type("#VECT")))") ) return VECT*SZ;
#define TYPESIZEVEC(TYPE,SZ)\
  else if(name == #TYPE) return SZ;\
  TYPESIZE(TYPE,2,SZ)\
  TYPESIZE(TYPE,3,SZ)\
  TYPESIZE(TYPE,4,SZ)\
  TYPESIZE(TYPE,8,SZ)\
  TYPESIZE(TYPE,16,SZ)

  static uint32_t getTypeSize(Module* M, const ir::Unit &unit, std::string& name) {
      if(name == "size_t") return sizeof(size_t);
      TYPESIZEVEC(char,1)
      TYPESIZEVEC(unsigned char,1)
      TYPESIZEVEC(short,2)
      TYPESIZEVEC(unsigned short,2)
      TYPESIZEVEC(half,2)
      TYPESIZEVEC(int,4)
      TYPESIZEVEC(unsigned int,4)
      TYPESIZEVEC(float,4)
      TYPESIZEVEC(double,8)
      TYPESIZEVEC(long,8)
      TYPESIZEVEC(unsigned long,8)
      else{
        StructType *StrTy = M->getTypeByName("struct."+name);
        if(StrTy)
          return getTypeByteSize(unit,StrTy);
      }
      GBE_ASSERTM(false, "Unspported type name");
      return 0;
  }
#undef TYPESIZEVEC
#undef TYPESIZE
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
        case Type::HalfTyID:
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
              elementTypeID != Type::HalfTyID &&
              elementTypeID != Type::DoubleTyID)
            GBE_ASSERTM(false, "Vectors of elements are not supported");
            return this->_newScalar(value, key, elementType, index, uniform);
          break;
        }
        case Type::StructTyID:
        {
          auto structType = cast<StructType>(type);
          auto elementType = structType->getElementType(index);
          auto elementTypeID = elementType->getTypeID();
          if (elementTypeID != Type::IntegerTyID &&
              elementTypeID != Type::FloatTyID &&
              elementTypeID != Type::HalfTyID &&
              elementTypeID != Type::DoubleTyID)
            GBE_ASSERTM(false, "Strcuts of elements are not supported");
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

  class GenWriter;
  class MemoryInstHelper {
    public:
      MemoryInstHelper(ir::Context &c, ir::Unit &u, GenWriter *w, bool l)
                : ctx(c),
                  unit(u),
                  writer(w),
                  legacyMode(l)
                  { }
      void         emitUnalignedDQLoadStore(Value *llvmValues);
      ir::Tuple    getValueTuple(llvm::Value *llvmValues, llvm::Type *elemType, unsigned start, unsigned elemNum);
      void         emitBatchLoadOrStore(const ir::Type type, const uint32_t elemNum, Value *llvmValues, Type * elemType);
      ir::Register getOffsetAddress(ir::Register basePtr, unsigned offset);
      void         shootMessage(ir::Type type, ir::Register offset, ir::Tuple value, unsigned elemNum);
      template <bool IsLoad, typename T>
      void         emitLoadOrStore(T &I);
    private:
      ir::Context             &ctx;
      ir::Unit               &unit;
      GenWriter            *writer;
      bool              legacyMode;
      ir::AddressSpace   addrSpace;
      ir::Register            mBTI;
      ir::Register            mPtr;
      ir::AddressMode mAddressMode;
      unsigned        SurfaceIndex;
      bool                  isLoad;
      bool               dwAligned;
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
    typedef map<const Value*, int>::iterator GlobalPtrIter;

    /*!
     *  <phi,phiCopy> node information for later optimization
     */
    map<const ir::Register, const ir::Register> phiMap;

    map<Value *, SmallVector<Value *, 4>> pointerOrigMap;
    typedef map<Value *, SmallVector<Value *, 4>>::iterator PtrOrigMapIter;
    // map pointer source to bti
    map<Value *, unsigned> BtiMap;
    // map printf pointer source to bti
    int printfBti;
    uint32_t printfNum;
    // map ptr to its bti register
    map<Value *, Value *> BtiValueMap;
    // map ptr to it's base
    map<Value *, Value *> pointerBaseMap;
    std::set<Value *> addrStoreInst;
    typedef map<Value *, Value *>::iterator PtrBaseMapIter;
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
    Function *Func;
    const Module *TheModule;
    int btiBase;
    bool has_errors;
    /*! legacyMode is for hardware before BDW,
     * which do not support stateless memory access */
    bool legacyMode;
  public:
    static char ID;
    explicit GenWriter(ir::Unit &unit)
      : FunctionPass(ID),
        unit(unit),
        ctx(unit),
        regTranslator(ctx),
        printfBti(-1),
        printfNum(0),
        LI(0),
        TheModule(0),
        btiBase(BTI_RESERVED_NUM),
        has_errors(false),
        legacyMode(true)
    {
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >=7
      initializeLoopInfoWrapperPassPass(*PassRegistry::getPassRegistry());
#else
      initializeLoopInfoPass(*PassRegistry::getPassRegistry());
#endif
      pass = PASS_EMIT_REGISTERS;
    }

    virtual const char *getPassName() const { return "Gen Back-End"; }

    void getAnalysisUsage(AnalysisUsage &AU) const {
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >=7
      AU.addRequired<LoopInfoWrapperPass>();
#else
      AU.addRequired<LoopInfo>();
#endif
      AU.setPreservesAll();
    }

    virtual bool doInitialization(Module &M);
    /*! helper function for parsing global constant data */
    void getConstantData(const Constant * c, void* mem, uint32_t& offset, vector<ir::RelocEntry> &) const;
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

      Func = &F;
      assignBti(F);
      if (legacyMode)
        analyzePointerOrigin(F);

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >=7
      LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
#else
      LI = &getAnalysis<LoopInfo>();
#endif
      emitFunction(F);
      phiMap.clear();
      globalPointer.clear();
      pointerOrigMap.clear();
      BtiMap.clear();
      BtiValueMap.clear();
      pointerBaseMap.clear();
      addrStoreInst.clear();
      // Reset for next function
      btiBase = BTI_RESERVED_NUM;
      printfBti = -1;
      return false;
    }
    /*! Given a possible pointer value, find out the interested escape like
        load/store or atomic instruction */
    void findPointerEscape(Value *ptr, std::set<Value *> &mixedPtr, bool recordMixed, std::vector<Value *> &revisit);
    /*! For all possible pointers, GlobalVariable, function pointer argument,
        alloca instruction, find their pointer escape points */
    void analyzePointerOrigin(Function &F);
    unsigned getNewBti(Value *origin, bool force);
    void assignBti(Function &F);
    bool isSingleBti(Value *Val);
    Value *getBtiRegister(Value *v);
    /*! get the pointer origin */
    Value *getSinglePointerOrigin(Value *ptr);
    /*! get the bti base address */
    Value *getPointerBase(Value *ptr);
    void processPointerArray(Value *ptr, Value *bti, Value *base);
    void handleStoreLoadAddress(Function &F);

    MDNode *getKernelFunctionMetadata(Function *F);
    virtual bool doFinalization(Module &M) { return false; }
    /*! handle global variable register allocation (local, constant space) */
    void allocateGlobalVariableRegister(Function &F);
    /*! gather all the loops in the function and add them to ir::Function */
    void gatherLoopInfo(ir::Function &fn);
    /*! do topological sorting of basicblocks */
    void sortBasicBlock(Function &F);
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
    template <bool IsLoad, typename T> void emitLoadOrStore(T &I);
    /*! Will try to remove MOVs due to PHI resolution */
    void removeMOVs(const ir::Liveness &liveness, ir::Function &fn);
    /*! Optimize phi move based on liveness information */
    void optimizePhiCopy(ir::Liveness &liveness, ir::Function &fn,
                         map <ir::Register, ir::Register> &replaceMap,
                         map <ir::Register, ir::Register> &redundantPhiCopyMap);
    /*! further optimization after phi copy optimization.
     *  Global liveness interefering checking based redundant phy value
     *  elimination. */
    void postPhiCopyOptimization(ir::Liveness &liveness, ir::Function &fn,
                                 map <ir::Register, ir::Register> &replaceMap,
                                 map <ir::Register, ir::Register> &redundantPhiCopyMap);
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
    DECL_VISIT_FN(ExtractValue, ExtractValueInst);
    DECL_VISIT_FN(ShuffleVectorInst, ShuffleVectorInst);
    DECL_VISIT_FN(SelectInst, SelectInst);
    DECL_VISIT_FN(BranchInst, BranchInst);
    DECL_VISIT_FN(PHINode, PHINode);
    DECL_VISIT_FN(AllocaInst, AllocaInst);
    DECL_VISIT_FN(AtomicRMWInst, AtomicRMWInst);
    DECL_VISIT_FN(AtomicCmpXchgInst, AtomicCmpXchgInst);
#undef DECL_VISIT_FN

    // Emit unary instructions from gen native function
    void emitUnaryCallInst(CallInst &I, CallSite &CS, ir::Opcode opcode, ir::Type = ir::TYPE_FLOAT);
    // Emit unary instructions from gen native function
    void emitAtomicInst(CallInst &I, CallSite &CS, ir::AtomicOps opcode);
    // Emit workgroup instructions
    void emitWorkGroupInst(CallInst &I, CallSite &CS, ir::WorkGroupOps opcode);
    // Emit subgroup instructions
    void emitSubGroupInst(CallInst &I, CallSite &CS, ir::WorkGroupOps opcode);
    // Emit subgroup instructions
    void emitBlockReadWriteMemInst(CallInst &I, CallSite &CS, bool isWrite, uint8_t vec_size, ir::Type = ir::TYPE_U32);
    void emitBlockReadWriteImageInst(CallInst &I, CallSite &CS, bool isWrite, uint8_t vec_size, ir::Type = ir::TYPE_U32);

    uint8_t appendSampler(CallSite::arg_iterator AI);
    uint8_t getImageID(CallInst &I);

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
    void visitUnreachableInst(UnreachableInst &I) {;}
    void visitGetElementPtrInst(GetElementPtrInst &I) {NOT_SUPPORTED;}
    void visitInsertValueInst(InsertValueInst &I) {NOT_SUPPORTED;}
    template <bool IsLoad, typename T> void visitLoadOrStore(T &I);

    INLINE void gatherBTI(Value *pointer, ir::BTI &bti);
    // batch vec4/8/16 load/store
    INLINE void emitBatchLoadOrStore(const ir::Type type, const uint32_t elemNum,
                  Value *llvmValue, const ir::Register ptr,
                  const ir::AddressSpace addrSpace, Type * elemType, bool isLoad, ir::Register bti,
                  bool dwAligned, bool fixedBTI);
    // handle load of dword/qword with unaligned address
    void emitUnalignedDQLoadStore(ir::Register ptr, Value *llvmValues, ir::AddressSpace addrSpace, ir::Register bti, bool isLoad, bool dwAligned, bool fixedBTI);
    void visitInstruction(Instruction &I) {NOT_SUPPORTED;}
    ir::PrintfSet::PrintfFmt* getPrintfInfo(CallInst* inst) {
      if (unit.printfs.find(inst) == unit.printfs.end())
        return NULL;
      return unit.printfs[inst];
    }
    void emitAtomicInstHelper(const ir::AtomicOps opcode,const ir::Type type, const ir::Register dst, llvm::Value* llvmPtr, const ir::Tuple payloadTuple);
    private:
      void setDebugInfo_CTX(llvm::Instruction * insn); // store the debug infomation in context for subsequently passing to Gen insn
      ir::ImmediateIndex processConstantImmIndexImpl(Constant *CPV, int32_t index = 0u);
      template <typename T, typename P = T>
      ir::ImmediateIndex processSeqConstant(ConstantDataSequential *seq,
                                            int index, ConstTypeId tid);
      ir::ImmediateIndex processConstantVector(ConstantVector *cv, int index);
      friend class MemoryInstHelper;
  };

  char GenWriter::ID = 0;

  static void updatePointerSource(Value *parent, Value *theUser, Value *source, SmallVector<Value *, 4> &pointers) {
    if (isa<SelectInst>(theUser)) {
      SelectInst *si = dyn_cast<SelectInst>(theUser);
      if (si && si->getTrueValue() == parent)
        pointers[0] = source;
      else
        pointers[1] = source;
    } else if (isa<PHINode>(theUser)) {
      PHINode *phi = dyn_cast<PHINode>(theUser);
      unsigned opNum = phi ? phi->getNumIncomingValues() : 0;
      for (unsigned j = 0; j < opNum; j++) {
        if (phi->getIncomingValue(j) == parent) {
          pointers[j] = source;
        }
      }
    } else {
      pointers[0] = source;
    }
  }

  bool isMixedPoint(Value *val, SmallVector<Value *, 4> &pointers) {
    Value *validSrc = NULL;
    unsigned i = 0;
    if (pointers.size() < 2) return false;
    while(i < pointers.size()) {
      if (pointers[i] != NULL && validSrc != NULL && pointers[i] != validSrc)
        return true;
      // when source is same as itself, we don't treat it as a new source
      // this often occurs for PHINode
      if (pointers[i] != NULL && validSrc == NULL && pointers[i] != val) {
        validSrc = pointers[i];
      }
      i++;
    }
    return false;
  }

  void GenWriter::findPointerEscape(Value *ptr,  std::set<Value *> &mixedPtr, bool bFirstPass, std::vector<Value *> &revisit) {
    std::vector<Value*> workList;
    std::set<Value *> visited;
    // loadInst result maybe used as pointer
    std::set<LoadInst *> ptrCandidate;

    bool isPointerArray = false;
    if (ptr->use_empty()) return;

    workList.push_back(ptr);

    for (unsigned i = 0; i < workList.size(); i++) {
      Value *work = workList[i];
      if (work->use_empty()) continue;
      for (Value::use_iterator iter = work->use_begin(); iter != work->use_end(); ++iter) {
      // After LLVM 3.5, use_iterator points to 'Use' instead of 'User',
      // which is more straightforward.
  #if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 5)
        User *theUser = *iter;
  #else
        User *theUser = iter->getUser();
  #endif
        // becareful with sub operation
        if (isa<BinaryOperator>(theUser) && dyn_cast<BinaryOperator>(theUser)->getOpcode() == Instruction::Sub) {
          // check both comes from ptrtoInt, don't need to traverse ptrdiff
          Value *op0 = theUser->getOperand(0);
          Value *op1 = theUser->getOperand(1);
          if ((isa<Instruction>(op0) && dyn_cast<Instruction>(op0)->getOpcode() == Instruction::PtrToInt)
              &&(isa<Instruction>(op1) && dyn_cast<Instruction>(op1)->getOpcode() == Instruction::PtrToInt)) {
            continue;
          }
        }

        if (isa<Instruction>(theUser)) {
          // some GlobalVariable maybe used in the function which is not current processed.
          // such kind of user should be skipped
          if (dyn_cast<Instruction>(theUser)->getParent()->getParent() != Func)
            continue;
        }

        bool visitedInThisSource = visited.find(theUser) != visited.end();

        if (isa<SelectInst>(theUser) || isa<PHINode>(theUser))
        {
          // reached from another source, update pointer source
          PtrOrigMapIter ptrIter = pointerOrigMap.find(theUser);
          if (ptrIter == pointerOrigMap.end()) {
            // create new one
            unsigned capacity = 1;
            if (isa<SelectInst>(theUser)) capacity = 2;
            if (isa<PHINode>(theUser)) {
              PHINode *phi = dyn_cast<PHINode>(theUser);
              capacity = phi ? phi->getNumIncomingValues() : 1;
            }

            SmallVector<Value *, 4> pointers;

            unsigned k = 0;
            while (k++ < capacity) {
              pointers.push_back(NULL);
            }

            updatePointerSource(work, theUser, ptr, pointers);
            pointerOrigMap.insert(std::make_pair(theUser, pointers));
          } else {
            // update pointer source
            updatePointerSource(work, theUser, ptr, (*ptrIter).second);
          }
          ptrIter = pointerOrigMap.find(theUser);

          if (isMixedPoint(theUser, (*ptrIter).second)) {
            // for the first pass, we need to record the mixed point instruction.
            // for the second pass, we don't need to go further, the reason is:
            // we always use it's 'direct mixed pointer parent' as origin, if we don't
            // stop here, we may set wrong pointer origin.
            if (bFirstPass)
              mixedPtr.insert(theUser);
            else
              continue;
          }
          // don't fall into dead loop,
          if (visitedInThisSource || theUser == ptr) {
            continue;
          }
        }

        // pointer address is used as the ValueOperand in store instruction, should be skipped
        if (StoreInst *store = dyn_cast<StoreInst>(theUser)) {
          if (store->getValueOperand() == work) {
            addrStoreInst.insert(store);
            Value * pointerOperand = store->getPointerOperand();
            // check whether the pointerOperand already visited or not,
            // if not visited, then we need to record all the loadInst
            // on the origin of pointerOperand
            // if visited, that is the origin of the pointerOperand already
            // traversed, we need to the traverse again to record all the LoadInst
            PtrOrigMapIter pointerOpIter = pointerOrigMap.find(pointerOperand);
            bool pointerVisited = pointerOpIter != pointerOrigMap.end();
            if (pointerVisited) {
              revisit.push_back((*pointerOpIter).second[0]);
            }

            PtrOrigMapIter ptrIter = pointerOrigMap.find(work);
            if (ptrIter == pointerOrigMap.end()) {
              // create new one
              SmallVector<Value *, 4> pointers;
              pointers.push_back(ptr);
              pointerOrigMap.insert(std::make_pair(work, pointers));
            } else {
              // update the pointer source here,
              if ((!isa<SelectInst>(work) && !isa<PHINode>(work)))
                (*ptrIter).second[0] = ptr;
            }

            continue;
          }
        }

        visited.insert(theUser);

        if (isa<LoadInst>(theUser) || isa<StoreInst>(theUser) || isa<CallInst>(theUser)) {
          if (isa<CallInst>(theUser)) {
            Function *F = dyn_cast<CallInst>(theUser)->getCalledFunction();
            if (!F || F->getIntrinsicID() != 0) continue;
          }
          Value *pointer = NULL;
          if (isa<LoadInst>(theUser)) {
            ptrCandidate.insert(cast<LoadInst>(theUser));
            pointer = dyn_cast<LoadInst>(theUser)->getPointerOperand();
          } else if (isa<StoreInst>(theUser)) {
            pointer = dyn_cast<StoreInst>(theUser)->getPointerOperand();
            // Check whether we have stored a address to this pointer
            // if yes, we need to traverse the ptrCandidate, as they are loaded pointers
            if (addrStoreInst.find(theUser) != addrStoreInst.end()) {
              isPointerArray = true;
            }
          } else if (isa<CallInst>(theUser)) {
            // atomic/read(write)image
            CallInst *ci = dyn_cast<CallInst>(theUser);
            pointer = ci ? ci->getArgOperand(0) : NULL;
          } else {
            theUser->dump();
            GBE_ASSERT(0 && "Unknown instruction operating on pointers\n");
          }

          // the pointer operand is same as pointer origin, don't add to pointerOrigMap
          if (ptr == pointer) continue;

          // load/store/atomic instruction, we have reached the end, stop further traversing
          PtrOrigMapIter ptrIter = pointerOrigMap.find(pointer);
          if (ptrIter == pointerOrigMap.end()) {
            // create new one
            SmallVector<Value *, 4> pointers;
            pointers.push_back(ptr);
            pointerOrigMap.insert(std::make_pair(pointer, pointers));
          } else {
            // update the pointer source here,
            if ((!isa<SelectInst>(pointer) && !isa<PHINode>(pointer)))
              (*ptrIter).second[0] = ptr;
          }
        } else {
          workList.push_back(theUser);
        }
      }
    }

    if (isPointerArray) {
      GBE_ASSERT((isa<AllocaInst>(ptr) || ptrCandidate.empty())
                && "storing/loading pointers only support private array");
      for (auto x : ptrCandidate) {
        revisit.push_back(x);
      }
    }
    ptrCandidate.clear();
  }

  bool GenWriter::isSingleBti(Value *Val) {
    // self + others same --> single
    // all same  ---> single
    if (!isa<SelectInst>(Val) && !isa<PHINode>(Val)) {
      return true;
    } else {
      PtrOrigMapIter iter = pointerOrigMap.find(Val);
      SmallVector<Value *, 4> &pointers = (*iter).second;
      unsigned srcNum = pointers.size();
      Value *source = NULL;
      for (unsigned x = 0; x < srcNum; x++) {
        // often happend in phiNode where one source is same as PHINode itself, skip it
        if (pointers[x] == Val) continue;

        if (source == NULL) source = pointers[x];
        else {
          if (source != pointers[x])
            return false;
        }
      }
      return true;
    }
  }
  Value *GenWriter::getPointerBase(Value *ptr) {
    PtrBaseMapIter baseIter = pointerBaseMap.find(ptr);
    if (baseIter != pointerBaseMap.end()) {
      return baseIter->second;
    }

    if (isa<ConstantPointerNull>(ptr)) {
      PointerType *ty = PointerType::get(ptr->getType(), 0);
      return ConstantPointerNull::get(ty);
    }

    typedef std::map<Value *, unsigned>::iterator BtiIter;
    // for pointers that already assigned a bti, it is the base pointer,
    BtiIter found = BtiMap.find(ptr);
    if (found != BtiMap.end()) {
      if (isa<PointerType>(ptr->getType())) {
        PointerType *ty = cast<PointerType>(ptr->getType());
        // only global pointer will have starting address
        if (ty->getAddressSpace() == 1) {
          return ptr;
        } else {
          return ConstantPointerNull::get(ty);
        }
      } else {
          PointerType *ty = PointerType::get(ptr->getType(), 0);
          return ConstantPointerNull::get(ty);
      }
    }

    PtrOrigMapIter iter = pointerOrigMap.find(ptr);

    // we may not find the ptr, as it may be uninitialized
    if (iter == pointerOrigMap.end()) {
      PointerType *ty = PointerType::get(ptr->getType(), 0);
      return ConstantPointerNull::get(ty);
    }

    SmallVector<Value *, 4> &pointers = (*iter).second;
    if (isSingleBti(ptr)) {
      Value *base = getPointerBase(pointers[0]);
      pointerBaseMap.insert(std::make_pair(ptr, base));
      return base;
    } else {
      if (isa<SelectInst>(ptr)) {
          SelectInst *si = dyn_cast<SelectInst>(ptr);
          IRBuilder<> Builder(si->getParent());

          Value *trueVal = getPointerBase((*iter).second[0]);
          Value *falseVal = getPointerBase((*iter).second[1]);
          Builder.SetInsertPoint(si);
          Value *base = Builder.CreateSelect(si->getCondition(), trueVal, falseVal);
          pointerBaseMap.insert(std::make_pair(ptr, base));
        return base;
      } else if (isa<PHINode>(ptr)) {
          PHINode *phi = dyn_cast<PHINode>(ptr);
          IRBuilder<> Builder(phi->getParent());
          Builder.SetInsertPoint(phi);

          PHINode *basePhi = Builder.CreatePHI(ptr->getType(), phi->getNumIncomingValues());
          unsigned srcNum = pointers.size();
          for (unsigned x = 0; x < srcNum; x++) {
            Value *base = NULL;
            if (pointers[x] != ptr) {
              base = getPointerBase(pointers[x]);
            } else {
              base = basePhi;
            }
            IRBuilder<> Builder2(phi->getIncomingBlock(x));
            BasicBlock *predBB = phi->getIncomingBlock(x);
            if (predBB->getTerminator())
              Builder2.SetInsertPoint(predBB->getTerminator());

#if (LLVM_VERSION_MAJOR== 3 && LLVM_VERSION_MINOR < 6)
  // llvm 3.5 and older version don't have CreateBitOrPointerCast() define
            Type *srcTy = base->getType();
            Type *dstTy = ptr->getType();
            if (srcTy->isPointerTy() && dstTy->isIntegerTy())
              base = Builder2.CreatePtrToInt(base, dstTy);
            else if (srcTy->isIntegerTy() && dstTy->isPointerTy())
              base = Builder2.CreateIntToPtr(base, dstTy);
            else if (srcTy != dstTy)
              base = Builder2.CreateBitCast(base, dstTy);
#else
            base = Builder2.CreateBitOrPointerCast(base, ptr->getType());
#endif
            basePhi->addIncoming(base, phi->getIncomingBlock(x));
          }
          pointerBaseMap.insert(std::make_pair(ptr, basePhi));
          return basePhi;
      } else {
        ptr->dump();
        GBE_ASSERT(0 && "Unhandled instruction in getPointerBase\n");
        return ptr;
      }
    }
  }

  Value *GenWriter::getSinglePointerOrigin(Value *ptr) {
    typedef std::map<Value *, unsigned>::iterator BtiIter;
    // for pointers that already assigned a bti, it is the pointer origin,
    BtiIter found = BtiMap.find(ptr);
    if (found != BtiMap.end())
      return ptr;
    PtrOrigMapIter iter = pointerOrigMap.find(ptr);
    GBE_ASSERT(iter != pointerOrigMap.end());
    return iter->second[0];
  }

  Value *GenWriter::getBtiRegister(Value *Val) {
    typedef std::map<Value *, unsigned>::iterator BtiIter;
    typedef std::map<Value *, Value *>::iterator BtiValueIter;
    BtiIter found = BtiMap.find(Val);
    BtiValueIter valueIter = BtiValueMap.find(Val);
    if (valueIter != BtiValueMap.end())
      return valueIter->second;

    if (isa<ConstantPointerNull>(Val)) {
      return ConstantInt::get(Type::getInt32Ty(Val->getContext()), BTI_PRIVATE);
    }

    if (found != BtiMap.end()) {
      // the Val already got assigned an BTI, return it
      Value *bti = ConstantInt::get(IntegerType::get(Val->getContext(), 32),
                                    found->second);
      BtiValueMap.insert(std::make_pair(Val, bti));
      return bti;
    } else {
      PtrOrigMapIter iter = pointerOrigMap.find(Val);
      // the pointer may access an uninitialized pointer,
      // in this case, we will not find it in pointerOrigMap
      if (iter == pointerOrigMap.end())
        return ConstantInt::get(Type::getInt32Ty(Val->getContext()), BTI_PRIVATE);

      if (isSingleBti(Val)) {
        Value * bti = getBtiRegister((*iter).second[0]);
        BtiValueMap.insert(std::make_pair(Val, bti));
        return bti;
      } else {
        if (isa<SelectInst>(Val)) {
          SelectInst *si = dyn_cast<SelectInst>(Val);

          IRBuilder<> Builder(si->getParent());
          Value *trueVal = getBtiRegister((*iter).second[0]);
          Value *falseVal = getBtiRegister((*iter).second[1]);
          Builder.SetInsertPoint(si);
          Value *bti = Builder.CreateSelect(si->getCondition(),
                                            trueVal, falseVal);
          BtiValueMap.insert(std::make_pair(Val, bti));
          return bti;
        } else if (isa<PHINode>(Val)) {
          PHINode *phi = dyn_cast<PHINode>(Val);
          IRBuilder<> Builder(phi->getParent());
          Builder.SetInsertPoint(phi);

          PHINode *btiPhi = Builder.CreatePHI(
                                    IntegerType::get(Val->getContext(), 32),
                                    phi->getNumIncomingValues());
          SmallVector<Value *, 4> &pointers = (*iter).second;
          unsigned srcNum = pointers.size();
          for (unsigned x = 0; x < srcNum; x++) {
            Value *bti = NULL;
            if (pointers[x] != Val) {
              bti = getBtiRegister(pointers[x]);
            } else {
              bti = btiPhi;
            }
            btiPhi->addIncoming(bti, phi->getIncomingBlock(x));
          }
          BtiValueMap.insert(std::make_pair(Val, btiPhi));
          return btiPhi;
        } else {
          Val->dump();
          GBE_ASSERT(0 && "Unhandled instruction in getBtiRegister\n");
          return Val;
        }
      }
    }
  }

  unsigned GenWriter::getNewBti(Value *origin, bool force) {
    unsigned new_bti = 0;
    if (force) {
      new_bti = btiBase;
      incBtiBase();
      return new_bti;
    }

    if (origin->getName().equals(StringRef("__gen_ocl_profiling_buf"))) {
      new_bti = btiBase;
      incBtiBase();
    }
    else if (isa<GlobalVariable>(origin)
        && dyn_cast<GlobalVariable>(origin)->isConstant()) {
      new_bti = BTI_CONSTANT;
    } else {
      unsigned space = origin->getType()->getPointerAddressSpace();
      switch (space) {
        case 0:
          new_bti = BTI_PRIVATE;
          break;
        case 1:
        {
          new_bti = btiBase;
          incBtiBase();
          break;
        }
        case 2:
          // ocl 2.0, constant pointer use separate bti
          if(legacyMode)
            new_bti = BTI_CONSTANT;//btiBase;
          else {
            new_bti = btiBase;//btiBase;
            incBtiBase();
          }
          break;
        case 3:
          new_bti = BTI_LOCAL;
          break;
        default:
          GBE_ASSERT(0);
          break;
      }
    }
    return new_bti;
  }

  MDNode *GenWriter::getKernelFunctionMetadata(Function *F) {
    NamedMDNode *clKernels = TheModule->getNamedMetadata("opencl.kernels");
     uint32_t ops = clKernels->getNumOperands();
      for(uint32_t x = 0; x < ops; x++) {
        MDNode* node = clKernels->getOperand(x);
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
        Value * op = node->getOperand(0);
#else
        auto *V = cast<ValueAsMetadata>(node->getOperand(0));
        Value *op = V ? V->getValue() : NULL;
#endif
        if(op == F) {
          return node;
        }
      }
    return NULL;
  }

  void GenWriter::assignBti(Function &F) {
    Module::GlobalListType &globalList = const_cast<Module::GlobalListType &> (TheModule->getGlobalList());
    for(auto i = globalList.begin(); i != globalList.end(); i ++) {
      GlobalVariable &v = *i;
      if(!v.isConstantUsed()) continue;

      BtiMap.insert(std::make_pair(&v, getNewBti(&v, false)));
    }
    MDNode *typeNameNode = NULL;
    MDNode *typeBaseNameNode = NULL;
    MDNode *typeQualNode = NULL;
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9
    typeNameNode = F.getMetadata("kernel_arg_type");
    typeBaseNameNode = F.getMetadata("kernel_arg_base_type");
    typeQualNode = F.getMetadata("kernel_arg_type_qual");
#else
    MDNode *node = getKernelFunctionMetadata(&F);
    for(uint j = 0;node && j < node->getNumOperands() - 1; j++) {
      MDNode *attrNode = dyn_cast_or_null<MDNode>(node->getOperand(1 + j));
      if (attrNode == NULL) break;
      MDString *attrName = dyn_cast_or_null<MDString>(attrNode->getOperand(0));
      if (!attrName) continue;
      if (attrName->getString() == "kernel_arg_type") {
        typeNameNode = attrNode;
      } else if (attrName->getString() == "kernel_arg_type_qual") {
        typeQualNode = attrNode;
      }
      if (attrName->getString() == "kernel_arg_base_type") {
        typeBaseNameNode = attrNode;
      }
    }
#endif

    unsigned argID = 0;
    ir::FunctionArgument::InfoFromLLVM llvmInfo;
    for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I, argID++) {
      unsigned opID = argID;
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR < 9
      opID += 1;
#endif

      if(typeNameNode) {
        llvmInfo.typeName= (cast<MDString>(typeNameNode->getOperand(opID)))->getString();
      }
      if(typeBaseNameNode) {
        llvmInfo.typeBaseName= (cast<MDString>(typeBaseNameNode->getOperand(opID)))->getString();
      }
      llvmInfo.typeName= (cast<MDString>(typeNameNode->getOperand(opID)))->getString();
      llvmInfo.typeQual = (cast<MDString>(typeQualNode->getOperand(opID)))->getString();
      bool isImage = llvmInfo.isImageType();
      bool isPipe = llvmInfo.isPipeType();
      if (I->getType()->isPointerTy() || isImage || isPipe) {
        BtiMap.insert(std::make_pair(&*I, getNewBti(&*I, isImage || isPipe)));
      }
    }

    BasicBlock &bb = F.getEntryBlock();
    for (BasicBlock::iterator iter = bb.begin(), iterE = bb.end(); iter != iterE; ++iter) {
      if (AllocaInst *ai = dyn_cast<AllocaInst>(iter)) {
        BtiMap.insert(std::make_pair(ai, BTI_PRIVATE));
      }
    }
  }

  void GenWriter::processPointerArray(Value *ptr, Value *bti, Value *base) {
    std::vector<Value*> workList;
    std::set<Value *> visited;

    if (ptr->use_empty()) return;

    workList.push_back(ptr);

    for (unsigned i = 0; i < workList.size(); i++) {
      Value *work = workList[i];
      if (work->use_empty()) continue;

      for (Value::use_iterator iter = work->use_begin(); iter != work->use_end(); ++iter) {
      // After LLVM 3.5, use_iterator points to 'Use' instead of 'User',
      // which is more straightforward.
  #if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 5)
        User *theUser = *iter;
  #else
        User *theUser = iter->getUser();
  #endif
        if(visited.find(theUser) != visited.end())
          continue;

        visited.insert(theUser);

        if (isa<LoadInst>(theUser) || isa<StoreInst>(theUser) || isa<CallInst>(theUser)) {
          if (isa<CallInst>(theUser)) {
            Function *F = dyn_cast<CallInst>(theUser)->getCalledFunction();
            if (!F || F->getIntrinsicID() != 0) continue;
          }
          bool isLoad; Value *pointerOp;

          IRBuilder<> Builder(cast<Instruction>(theUser)->getParent());
          if (isa<LoadInst>(theUser)) {
            pointerOp = dyn_cast<LoadInst>(theUser)->getPointerOperand();
            isLoad = true;
          } else {
            pointerOp = dyn_cast<StoreInst>(theUser)->getPointerOperand();
            isLoad = false;
          }
          Builder.SetInsertPoint(cast<Instruction>(theUser));

          Type *ptyTy = IntegerType::get(ptr->getContext(), getTypeBitSize(unit, ptr->getType()));
          Value *v1 = Builder.CreatePtrToInt(pointerOp, ptyTy);

          Value *v2 = Builder.CreatePtrToInt(getSinglePointerOrigin(pointerOp), ptyTy);
          Value *v3 = Builder.CreatePtrToInt(base, ptyTy);
          Value *v4 = Builder.CreatePtrToInt(bti, ptyTy);
          // newLocBase = (pointer - origin) + base_start
          Value *diff = Builder.CreateSub(v1, v2);
          Value *newLocBase = Builder.CreateAdd(v3, diff);
          newLocBase = Builder.CreateIntToPtr(newLocBase, Type::getInt32PtrTy(ptr->getContext()));
          // newLocBti = (pointer - origin) + bti_start
          Value *newLocBti = Builder.CreateAdd(v4, diff);
          newLocBti = Builder.CreateIntToPtr(newLocBti, Type::getInt32PtrTy(ptr->getContext()));

          // later GenWriter instruction translation needs this map info
          BtiValueMap.insert(std::make_pair(newLocBti, ConstantInt::get(Type::getInt32Ty(ptr->getContext()), BTI_PRIVATE)));
          pointerBaseMap.insert(std::make_pair(newLocBti, ConstantPointerNull::get(cast<PointerType>(pointerOp->getType()))));

          BtiValueMap.insert(std::make_pair(newLocBase, ConstantInt::get(Type::getInt32Ty(ptr->getContext()), BTI_PRIVATE)));
          pointerBaseMap.insert(std::make_pair(newLocBase, ConstantPointerNull::get(cast<PointerType>(pointerOp->getType()))));

          if (isLoad) {
            Value *loadedBase = Builder.CreateLoad(newLocBase);
            Value *loadedBti = Builder.CreateLoad(newLocBti);

            BtiValueMap.insert(std::make_pair(theUser, loadedBti));
            pointerBaseMap.insert(std::make_pair(theUser, loadedBase));
          } else {
            Value *valueOp = cast<StoreInst>(theUser)->getValueOperand();
            Value *tmp = Builder.CreatePtrToInt(getPointerBase(valueOp), Type::getInt32Ty(ptr->getContext()));
            Builder.CreateStore(tmp, newLocBase);
            Builder.CreateStore(getBtiRegister(valueOp), newLocBti);
          }
        } else {
          workList.push_back(theUser);
        }
      }
    }
  }

  void GenWriter::analyzePointerOrigin(Function &F) {
    // used to record where the pointers get mixed (i.e. select or phi instruction)
    std::set<Value *> mixedPtr;
    // This is a two-pass algorithm, the 1st pass will try to update the pointer sources for
    // every instruction reachable from pointers and record mix-point in this pass.
    // The second pass will start from really mixed-pointer instruction like select or phinode.
    // and update the sources correctly. For pointers reachable from mixed-pointer, we will set
    // its direct mixed-pointer parent as it's pointer origin.

    std::vector<Value *> revisit;
    // GlobalVariable
    Module::GlobalListType &globalList = const_cast<Module::GlobalListType &> (TheModule->getGlobalList());
    for(auto i = globalList.begin(); i != globalList.end(); i ++) {
      GlobalVariable &v = *i;
      if(!v.isConstantUsed()) continue;
      findPointerEscape(&v, mixedPtr, true, revisit);
    }
    // function argument
    for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I) {
      if (I->getType()->isPointerTy()) {
        findPointerEscape(&*I, mixedPtr, true, revisit);
      }
    }
    // alloca
    BasicBlock &bb = F.getEntryBlock();
    for (BasicBlock::iterator iter = bb.begin(), iterE = bb.end(); iter != iterE; ++iter) {
      if (AllocaInst *ai = dyn_cast<AllocaInst>(iter)) {
        findPointerEscape(ai, mixedPtr, true, revisit);
      }
    }
    // storing/loading pointer would introduce revisit
    for (size_t i = 0; i < revisit.size(); ++i) {
      findPointerEscape(revisit[i], mixedPtr, true, revisit);
    }

    // the second pass starts from mixed pointer
    for (std::set<Value *>::iterator iter = mixedPtr.begin(); iter != mixedPtr.end(); ++iter) {
      findPointerEscape(*iter, mixedPtr, false, revisit);
    }

    for (std::set<Value *>::iterator iter = mixedPtr.begin(); iter != mixedPtr.end(); ++iter) {
      getBtiRegister(*iter);
    }

    for (std::set<Value *>::iterator iter = mixedPtr.begin(); iter != mixedPtr.end(); ++iter) {
      getPointerBase(*iter);
    }
    handleStoreLoadAddress(F);
  }
  void GenWriter::handleStoreLoadAddress(Function &F) {
    std::set<Value *> processed;
    for (std::set<Value *>::iterator iter = addrStoreInst.begin(); iter != addrStoreInst.end(); ++iter) {
      StoreInst *store = cast<StoreInst>(*iter);
      Value *pointerOp = store->getPointerOperand();
      Value *base = getSinglePointerOrigin(pointerOp);
      if (processed.find(base) != processed.end()) {
        continue;
      }
      processed.insert(base);

      if (!isa<AllocaInst>(base)) continue;

      Value *ArraySize = cast<AllocaInst>(base)->getArraySize();

      BasicBlock &entry = F.getEntryBlock();
      BasicBlock::iterator bbIter = entry.begin();
      while (isa<AllocaInst>(bbIter)) ++bbIter;

      IRBuilder<> Builder(&entry);
      Builder.SetInsertPoint(&*bbIter);

      PointerType * AITy = cast<AllocaInst>(base)->getType();
      Value * btiArray = Builder.CreateAlloca(AITy->getElementType(), ArraySize, base->getName() + ".bti");
      Value * pointerBaseArray = Builder.CreateAlloca(AITy->getElementType(), ArraySize, base->getName() + ".pointer-base");

      processPointerArray(base, btiArray, pointerBaseArray);
    }
  }

  void getSequentialData(const ConstantDataSequential *cda, void *ptr, uint32_t &offset) {
    StringRef data = cda->getRawDataValues();
    memcpy((char*)ptr+offset, data.data(), data.size());
    offset += data.size();
    return;
  }

  void GenWriter::getConstantData(const Constant * c, void* mem, uint32_t& offset, vector<ir::RelocEntry> &relocs) const {
    Type * type = c->getType();
    Type::TypeID id = type->getTypeID();

    GBE_ASSERT(c);
    if (isa<ConstantExpr>(c)) {
      const ConstantExpr *expr = dyn_cast<ConstantExpr>(c);
      Value *pointer = expr->getOperand(0);
      if (expr->getOpcode() == Instruction::GetElementPtr) {
        uint32_t constantOffset = 0;
        CompositeType* CompTy = cast<CompositeType>(pointer->getType());
        for(uint32_t op=1; op<expr->getNumOperands(); ++op) {
            int32_t TypeIndex;
            ConstantInt* ConstOP = dyn_cast<ConstantInt>(expr->getOperand(op));
            GBE_ASSERTM(ConstOP != NULL, "must be constant index");
            TypeIndex = ConstOP->getZExtValue();
            GBE_ASSERT(TypeIndex >= 0);
            constantOffset += getGEPConstOffset(unit, CompTy, TypeIndex);
            CompTy = dyn_cast<CompositeType>(CompTy->getTypeAtIndex(TypeIndex));
        }

        ir::Constant cc = unit.getConstantSet().getConstant(pointer->getName());
        unsigned int defOffset = cc.getOffset();
        relocs.push_back(ir::RelocEntry(offset, defOffset + constantOffset));

        uint32_t size = getTypeByteSize(unit, type);
        memset((char*)mem+offset, 0, size);
        offset += size;
      } else if (expr->isCast()) {
        Constant *constPtr = cast<Constant>(pointer);
        getConstantData(constPtr, mem, offset, relocs);
        offset += getTypeByteSize(unit, type);
      }
      return;
    }
    if (isa<GlobalVariable>(c)) {
      ir::Constant cc = unit.getConstantSet().getConstant(c->getName());
      unsigned int defOffset = cc.getOffset();

      relocs.push_back(ir::RelocEntry(offset, defOffset));
      uint32_t size = getTypeByteSize(unit, type);
      memset((char*)mem+offset, 0, size);
      offset += size;
      return;
    }
    if(isa<UndefValue>(c)) {
      uint32_t size = getTypeByteSize(unit, type);
      offset += size;
      return;
    } else if(isa<ConstantAggregateZero>(c) || isa<ConstantPointerNull>(c)) {
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

          for(uint32_t op=0; strTy && op < strTy->getNumElements(); op++)
          {
            Type* elementType = strTy->getElementType(op);
            uint32_t align = 8 * getAlignmentByte(unit, elementType);
            uint32_t padding = getPadding(size, align);
            size += padding;
            size += getTypeBitSize(unit, elementType);

            offset += padding/8;
            const Constant* sub = cast<Constant>(c->getOperand(op));
            GBE_ASSERT(sub);
            getConstantData(sub, mem, offset, relocs);
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
            if(!ca)
              return;
            const ArrayType *arrTy = ca->getType();
            Type* elemTy = arrTy->getElementType();
            uint32_t elemSize = getTypeBitSize(unit, elemTy);
            uint32_t padding = getPadding(elemSize, 8 * getAlignmentByte(unit, elemTy));
            padding /= 8;
            uint32_t ops = c->getNumOperands();
            for(uint32_t op = 0; op < ops; ++op) {
              Constant * ca = dyn_cast<Constant>(c->getOperand(op));
              getConstantData(ca, mem, offset, relocs);
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
      case Type::TypeID::HalfTyID:
        {
          const ConstantFP *cf = dyn_cast<ConstantFP>(c);
          llvm::APFloat apf = cf->getValueAPF();
          llvm::APInt api = apf.bitcastToAPInt();
          uint64_t v64 = api.getZExtValue();
          uint16_t v16 = static_cast<uint16_t>(v64);
          *(unsigned short *)((char*)mem+offset) = v16;
          offset += sizeof(short);
          break;
        }
      case Type::TypeID::PointerTyID:
        {
          break;
        }
      default:
        {
          c->dump();
          NOT_IMPLEMENTED;
        }
    }
  }
  static bool isProgramGlobal(const GlobalVariable &v) {
    unsigned addrSpace = v.getType()->getAddressSpace();
    // private/global/constant
    return (addrSpace == 2 || addrSpace == 1 || addrSpace == 0);
  }
  void GenWriter::collectGlobalConstant(void) const {
    const Module::GlobalListType &globalList = TheModule->getGlobalList();
    // The first pass just create the global variable constants
    for(auto i = globalList.begin(); i != globalList.end(); i ++) {
      const GlobalVariable &v = *i;
      const char *name = v.getName().data();

      vector<ir::RelocEntry> relocs;

      if(isProgramGlobal(v)) {
        Type * type = v.getType()->getPointerElementType();
        uint32_t size = getTypeByteSize(unit, type);
        uint32_t alignment = getAlignmentByte(unit, type);
        unit.newConstant(name, size, alignment);
      }
    }
    // the second pass to initialize the data
    for(auto i = globalList.begin(); i != globalList.end(); i ++) {
      const GlobalVariable &v = *i;
      const char *name = v.getName().data();

      if(isProgramGlobal(v)) {
        if (v.hasInitializer()) {
          vector<ir::RelocEntry> relocs;
          uint32_t offset = 0;
          ir::Constant &con = unit.getConstantSet().getConstant(name);
          void* mem = malloc(con.getSize());
          const Constant *c = v.getInitializer();
          getConstantData(c, mem, offset, relocs);
          unit.getConstantSet().setData((char*)mem, con.getOffset(), con.getSize());
          free(mem);

          if (!legacyMode) {
            uint32_t refOffset = unit.getConstantSet().getConstant(name).getOffset();
            for (uint32_t k = 0; k < relocs.size(); k++) {
              unit.getRelocTable().addEntry(
                  refOffset + relocs[k].refOffset,
                  relocs[k].defOffset
                  );
            }
          }
        }
      }
    }
  }

  bool GenWriter::doInitialization(Module &M) {
    FunctionPass::doInitialization(M);

    // Initialize
    TheModule = &M;
    uint32_t oclVersion = getModuleOclVersion(TheModule);
    legacyMode = oclVersion >= 200 ? false : true;
    unit.setOclVersion(oclVersion);
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
      for(uint32_t i = 0; i < seq->getNumElements(); i++)
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
      return ctx.newImmediate(immVector, getType(ctx, cv->getType()->getElementType()));
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
      } else if (Ty == Type::getHalfTy(CPV->getContext())) {
        GBE_ASSERTM(0, "Const data array never be half float\n");
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
      } else if (Ty == Type::getHalfTy(CPV->getContext())) {
        const ir::half f16 = 0;
        return ctx.newImmediate(f16);
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
        if (ctx.getPointerFamily() == ir::FAMILY_QWORD)
          return ctx.newImmediate(uint64_t(0));
        else
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
        if (Ty == Type::getHalfTy(CPV->getContext())) return ctx.newImmediate((ir::half)0);
        if (Ty == Type::getDoubleTy(CPV->getContext())) return ctx.newImmediate((double)0);
        GBE_ASSERT(0 && "Unsupported undef value type.\n");
      }

      // Floats and doubles
      switch (typeID) {
        case Type::FloatTyID:
        case Type::HalfTyID:
        case Type::DoubleTyID:
        {
          ConstantFP *FPC = cast<ConstantFP>(CPV);
          GBE_ASSERT(isa<UndefValue>(CPV) == false);

          if (FPC->getType() == Type::getFloatTy(CPV->getContext())) {
            const float f32 = FPC->getValueAPF().convertToFloat();
            return ctx.newImmediate(f32);
          } else if (FPC->getType() == Type::getDoubleTy(CPV->getContext())) {
            const double f64 = FPC->getValueAPF().convertToDouble();
            return ctx.newImmediate(f64);
          } else {
            llvm::APFloat apf = FPC->getValueAPF();
            llvm::APInt api = apf.bitcastToAPInt();
            uint64_t v64 = api.getZExtValue();
            uint16_t v16 = static_cast<uint16_t>(v64);
            const ir::half f16(v16);
            return ctx.newImmediate(f16);
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
    CPV->dump();
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
      case Type::HalfTyID:
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
      case Type::StructTyID:
      {
        auto structType = cast<StructType>(type);
        const uint32_t elemNum = structType->getNumElements();
        for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
          regTranslator.newScalar(value, key, elemID, uniform);
        break;
      }
      default: NOT_SUPPORTED;
    };
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
    Value *value = bb->getTerminator();
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
    for (auto II = BB->begin(), E = BB->end(); II != E; ++II) {
      if(OCL_DEBUGINFO) {
        llvm::Instruction * It = dyn_cast<llvm::Instruction>(II);
        setDebugInfo_CTX(It);
      }
      visit(*II);
    }
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

  /*! To track read image args and write args */
  struct ImageArgsInfo{
    uint32_t readImageArgs;
    uint32_t writeImageArgs;
  };

  static void collectImageArgs(std::string& accessQual, ImageArgsInfo& imageArgsInfo)
  {
    if(accessQual.find("read") != std::string::npos)
    {
      imageArgsInfo.readImageArgs++;
      GBE_ASSERT(imageArgsInfo.readImageArgs <= BTI_MAX_READ_IMAGE_ARGS);
    }
    else if(accessQual.find("write") != std::string::npos)
    {
      imageArgsInfo.writeImageArgs++;
      GBE_ASSERT(imageArgsInfo.writeImageArgs <= BTI_MAX_WRITE_IMAGE_ARGS);
    }
    else
    {
      //default is read_only per spec.
      imageArgsInfo.readImageArgs++;
      GBE_ASSERT(imageArgsInfo.readImageArgs <= BTI_MAX_READ_IMAGE_ARGS);
    }
  }

  void GenWriter::setDebugInfo_CTX(llvm::Instruction * insn)
  {
    llvm::DebugLoc dg = insn->getDebugLoc();
    DebugInfo dbginfo;
    dbginfo.line = dg.getLine();
    dbginfo.col = dg.getCol();
    ctx.setDBGInfo(dbginfo);
  }

  void GenWriter::emitFunctionPrototype(Function &F)
  {
    GBE_ASSERTM(F.hasStructRetAttr() == false,
                "Returned value for kernel functions is forbidden");

    // Loop over the kernel metadatas to set the required work group size.
    size_t reqd_wg_sz[3] = {0, 0, 0};
    size_t hint_wg_sz[3] = {0, 0, 0};
    ir::FunctionArgument::InfoFromLLVM llvmInfo;
    MDNode *addrSpaceNode = NULL;
    MDNode *typeNameNode = NULL;
    MDNode *typeBaseNameNode = NULL;
    MDNode *accessQualNode = NULL;
    MDNode *typeQualNode = NULL;
    MDNode *argNameNode = NULL;

    std::string functionAttributes;

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9
    /* LLVM 3.9 change kernel arg info as function metadata */
    addrSpaceNode = F.getMetadata("kernel_arg_addr_space");
    accessQualNode = F.getMetadata("kernel_arg_access_qual");
    typeNameNode = F.getMetadata("kernel_arg_type");
    typeBaseNameNode = F.getMetadata("kernel_arg_base_type");
    typeQualNode = F.getMetadata("kernel_arg_type_qual");
    argNameNode = F.getMetadata("kernel_arg_name");
    MDNode *attrNode;
    if ((attrNode = F.getMetadata("vec_type_hint"))) {
      GBE_ASSERT(attrNode->getNumOperands() == 2);
      functionAttributes += "vec_type_hint";
      auto *Op1 = cast<ValueAsMetadata>(attrNode->getOperand(0));
      Value *V = Op1 ? Op1->getValue() : NULL;
      ConstantInt *sign =
          mdconst::extract<ConstantInt>(attrNode->getOperand(1));
      size_t signValue = sign->getZExtValue();
      Type *vtype = V->getType();
      Type *stype = vtype;
      uint32_t elemNum = 0;
      if (vtype->isVectorTy()) {
        VectorType *vectorType = cast<VectorType>(vtype);
        stype = vectorType->getElementType();
        elemNum = vectorType->getNumElements();
      }

      std::string typeName = getTypeName(ctx, stype, signValue);

      std::stringstream param;
      char buffer[100] = {0};
      param << "(";
      param << typeName;
      if (vtype->isVectorTy())
        param << elemNum;
      param << ")";
      param >> buffer;
      functionAttributes += buffer;
      functionAttributes += " ";
    }
    if ((attrNode = F.getMetadata("reqd_work_group_size"))) {
      GBE_ASSERT(attrNode->getNumOperands() == 3);
      ConstantInt *x = mdconst::extract<ConstantInt>(attrNode->getOperand(0));
      ConstantInt *y = mdconst::extract<ConstantInt>(attrNode->getOperand(1));
      ConstantInt *z = mdconst::extract<ConstantInt>(attrNode->getOperand(2));
      GBE_ASSERT(x && y && z);
      reqd_wg_sz[0] = x->getZExtValue();
      reqd_wg_sz[1] = y->getZExtValue();
      reqd_wg_sz[2] = z->getZExtValue();
      functionAttributes += "reqd_work_group_size";
      std::stringstream param;
      char buffer[100] = {0};
      param << "(";
      param << reqd_wg_sz[0];
      param << ",";
      param << reqd_wg_sz[1];
      param << ",";
      param << reqd_wg_sz[2];
      param << ")";
      param >> buffer;
      functionAttributes += buffer;
      functionAttributes += " ";
    }
    if ((attrNode = F.getMetadata("work_group_size_hint"))) {
      GBE_ASSERT(attrNode->getNumOperands() == 3);
      ConstantInt *x = mdconst::extract<ConstantInt>(attrNode->getOperand(0));
      ConstantInt *y = mdconst::extract<ConstantInt>(attrNode->getOperand(1));
      ConstantInt *z = mdconst::extract<ConstantInt>(attrNode->getOperand(2));
      GBE_ASSERT(x && y && z);
      hint_wg_sz[0] = x->getZExtValue();
      hint_wg_sz[1] = y->getZExtValue();
      hint_wg_sz[2] = z->getZExtValue();
      functionAttributes += "work_group_size_hint";
      std::stringstream param;
      char buffer[100] = {0};
      param << "(";
      param << hint_wg_sz[0];
      param << ",";
      param << hint_wg_sz[1];
      param << ",";
      param << hint_wg_sz[2];
      param << ")";
      param >> buffer;
      functionAttributes += buffer;
      functionAttributes += " ";
    }
#else
    /* First find the meta data belong to this function. */
    MDNode *node = getKernelFunctionMetadata(&F);

    /* because "-cl-kernel-arg-info", should always have meta data. */
    if (!F.arg_empty())
      assert(node);


    for(uint j = 0; node && j < node->getNumOperands() - 1; j++) {
      MDNode *attrNode = dyn_cast_or_null<MDNode>(node->getOperand(1 + j));
      if (attrNode == NULL) break;
      MDString *attrName = dyn_cast_or_null<MDString>(attrNode->getOperand(0));
      if (!attrName) continue;

      if (attrName->getString() == "reqd_work_group_size") {
        GBE_ASSERT(attrNode->getNumOperands() == 4);
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
        ConstantInt *x = dyn_cast<ConstantInt>(attrNode->getOperand(1));
        ConstantInt *y = dyn_cast<ConstantInt>(attrNode->getOperand(2));
        ConstantInt *z = dyn_cast<ConstantInt>(attrNode->getOperand(3));
#else
        ConstantInt *x = mdconst::extract<ConstantInt>(attrNode->getOperand(1));
        ConstantInt *y = mdconst::extract<ConstantInt>(attrNode->getOperand(2));
        ConstantInt *z = mdconst::extract<ConstantInt>(attrNode->getOperand(3));
#endif
        GBE_ASSERT(x && y && z);
        reqd_wg_sz[0] = x->getZExtValue();
        reqd_wg_sz[1] = y->getZExtValue();
        reqd_wg_sz[2] = z->getZExtValue();
        functionAttributes += attrName->getString();
        std::stringstream param;
        char buffer[100] = {0};
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
      } else if (attrName->getString() == "kernel_arg_base_type") {
        typeBaseNameNode = attrNode;
      } else if (attrName->getString() == "kernel_arg_type_qual") {
        typeQualNode = attrNode;
      } else if (attrName->getString() == "kernel_arg_name") {
        argNameNode = attrNode;
      } else if (attrName->getString() == "vec_type_hint") {
        GBE_ASSERT(attrNode->getNumOperands() == 3);
        functionAttributes += attrName->getString();
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
        Value* V = attrNode->getOperand(1);
#else
        auto *Op1 = cast<ValueAsMetadata>(attrNode->getOperand(1));
        Value *V = Op1 ? Op1->getValue() : NULL;
#endif
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
        ConstantInt *sign = dyn_cast<ConstantInt>(attrNode->getOperand(2));
#else
        ConstantInt *sign = mdconst::extract<ConstantInt>(attrNode->getOperand(2));
#endif
        size_t signValue = sign->getZExtValue();
        Type* vtype = V->getType();
        Type* stype = vtype;
        uint32_t elemNum = 0;
        if(vtype->isVectorTy()) {
          VectorType *vectorType = cast<VectorType>(vtype);
          stype = vectorType->getElementType();
          elemNum = vectorType->getNumElements();
        }

        std::string typeName = getTypeName(ctx, stype, signValue);

        std::stringstream param;
        char buffer[100] = {0};
        param <<"(";
        param << typeName;
        if(vtype->isVectorTy())
          param << elemNum;
        param <<")";
        param >> buffer;
        functionAttributes += buffer;
        functionAttributes += " ";
      } else if (attrName->getString() == "work_group_size_hint") {
        GBE_ASSERT(attrNode->getNumOperands() == 4);
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
        ConstantInt *x = dyn_cast<ConstantInt>(attrNode->getOperand(1));
        ConstantInt *y = dyn_cast<ConstantInt>(attrNode->getOperand(2));
        ConstantInt *z = dyn_cast<ConstantInt>(attrNode->getOperand(3));
#else
        ConstantInt *x = mdconst::extract<ConstantInt>(attrNode->getOperand(1));
        ConstantInt *y = mdconst::extract<ConstantInt>(attrNode->getOperand(2));
        ConstantInt *z = mdconst::extract<ConstantInt>(attrNode->getOperand(3));
#endif
        GBE_ASSERT(x && y && z);
        hint_wg_sz[0] = x->getZExtValue();
        hint_wg_sz[1] = y->getZExtValue();
        hint_wg_sz[2] = z->getZExtValue();
        functionAttributes += attrName->getString();
        std::stringstream param;
        char buffer[100] = {0};
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
#endif /* LLVM 3.9 Function metadata */

    ctx.getFunction().setCompileWorkGroupSize(reqd_wg_sz[0], reqd_wg_sz[1], reqd_wg_sz[2]);

    ctx.getFunction().setFunctionAttributes(functionAttributes);
    // Loop over the arguments and output registers for them
    if (!F.arg_empty()) {
      uint32_t argID = 0;
      ImageArgsInfo imageArgsInfo = {};
      Function::arg_iterator I = F.arg_begin(), E = F.arg_end();

      // Insert a new register for each function argument
#if LLVM_VERSION_MINOR <= 1
      const AttrListPtr &PAL = F.getAttributes();
#endif /* LLVM_VERSION_MINOR <= 1 */
      for (; I != E; ++I, ++argID) {
        uint32_t opID = argID;
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR < 9
        opID += 1;
#endif
        const std::string &argName = I->getName().str();
        Type *type = I->getType();
        if(addrSpaceNode) {
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
          llvmInfo.addrSpace = (cast<ConstantInt>(addrSpaceNode->getOperand(opID)))->getZExtValue();
#else
          llvmInfo.addrSpace = (mdconst::extract<ConstantInt>(addrSpaceNode->getOperand(opID)))->getZExtValue();
#endif
        }
        if(typeNameNode) {
          llvmInfo.typeName = (cast<MDString>(typeNameNode->getOperand(opID)))->getString();
          //LLVM 3.9 image's type name include access qual, don't match OpenCL spec, erase them.
          std::vector<std::string> filters = {"__read_only ", "__write_only "};
          for (uint32_t i = 0; i < filters.size(); i++) {
            size_t pos = llvmInfo.typeName.find(filters[i]);
            if (pos != std::string::npos) {
              llvmInfo.typeName = llvmInfo.typeName.erase(pos, filters[i].length());
            }
          }
        }
        if(typeBaseNameNode){
          llvmInfo.typeBaseName = (cast<MDString>(typeBaseNameNode->getOperand(opID)))->getString();
        }
        if(accessQualNode) {
          llvmInfo.accessQual = (cast<MDString>(accessQualNode->getOperand(opID)))->getString();
        }
        if(typeQualNode) {
          llvmInfo.typeQual = (cast<MDString>(typeQualNode->getOperand(opID)))->getString();
        }
        if(argNameNode){
          llvmInfo.argName = (cast<MDString>(argNameNode->getOperand(opID)))->getString();
        }

        // function arguments are uniform values.
        this->newRegister(&*I, NULL, true);

        // add support for vector argument.
        if(type->isVectorTy()) {
          VectorType *vectorType = cast<VectorType>(type);
          ir::Register reg = getRegister(&*I, 0);
          Type *elemType = vectorType->getElementType();
          const uint32_t elemSize = getTypeByteSize(unit, elemType);
          const uint32_t elemNum = vectorType->getNumElements();
          //vector's elemType always scalar type
          ctx.input(argName, ir::FunctionArgument::VALUE, reg, llvmInfo, getTypeByteSize(unit, type), getAlignmentByte(unit, type), 0);

          ir::Function& fn = ctx.getFunction();
          for(uint32_t i=1; i < elemNum; i++) {
            ir::PushLocation argLocation(fn, argID, elemSize*i);
            reg = getRegister(&*I, i);
            ctx.appendPushedConstant(reg, argLocation);  //add to push map for reg alloc
          }
          continue;
        }

        GBE_ASSERTM(isScalarType(type) == true,
                    "vector type in the function argument is not supported yet");
        const ir::Register reg = getRegister(&*I);
        if (llvmInfo.isImageType()) {
          ctx.input(argName, ir::FunctionArgument::IMAGE, reg, llvmInfo, 4, 4, 0);
          ctx.getFunction().getImageSet()->append(reg, &ctx, BtiMap.find(&*I)->second);
          collectImageArgs(llvmInfo.accessQual, imageArgsInfo);
          continue;
        }

        if (llvmInfo.isSamplerType()) {
          ctx.input(argName, ir::FunctionArgument::SAMPLER, reg, llvmInfo, getTypeByteSize(unit, type), getAlignmentByte(unit, type), 0);
          (void)ctx.getFunction().getSamplerSet()->append(reg, &ctx);
          continue;
        }
        if(llvmInfo.isPipeType()) {
          llvmInfo.typeSize = getTypeSize(F.getParent(),unit,llvmInfo.typeName);
          ctx.input(argName, ir::FunctionArgument::PIPE, reg, llvmInfo, getTypeByteSize(unit, type), getAlignmentByte(unit, type), BtiMap.find(&*I)->second);
          continue;
        }

        if (type->isPointerTy() == false)
          ctx.input(argName, ir::FunctionArgument::VALUE, reg, llvmInfo, getTypeByteSize(unit, type), getAlignmentByte(unit, type), 0);
        else {
          PointerType *pointerType = dyn_cast<PointerType>(type);
          if(!pointerType)
            continue;
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
                ctx.input(argName, ir::FunctionArgument::GLOBAL_POINTER, reg, llvmInfo, ptrSize, align, BtiMap.find(&*I)->second);
              break;
              case ir::MEM_LOCAL:
                ctx.input(argName, ir::FunctionArgument::LOCAL_POINTER, reg,  llvmInfo, ptrSize, align, BTI_LOCAL);
                ctx.getFunction().setUseSLM(true);
              break;
              case ir::MEM_CONSTANT:
                ctx.input(argName, ir::FunctionArgument::CONSTANT_POINTER, reg,  llvmInfo, ptrSize, align, 0x2);
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
      if (insn.getOpcode() == ir::OP_MOV &&
          insn.getDst(0) == insn.getSrc(0)) {
        insn.remove();
        return;
      }
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

  void GenWriter::optimizePhiCopy(ir::Liveness &liveness, ir::Function &fn,
          map<ir::Register, ir::Register> &replaceMap,
          map<ir::Register, ir::Register> &redundantPhiCopyMap)
  {
    // The overall idea behind is we check whether there is any interference
    // between phi and phiCopy live range. If there is no point that
    // phi & phiCopy are both alive, then we can optimize off the move
    // from phiCopy to phi, and use phiCopy directly instead of phi.
    // right now, the algorithm is still very conservative, we need to do
    // aggressive coaleasing for the moves added during phi elimination.

    using namespace ir;
    ir::FunctionDAG *dag = new ir::FunctionDAG(liveness);
    for (auto &it : phiMap) {
      const Register phi = it.first;
      const Register phiCopy = it.second;

      const ir::DefSet *phiCopyDef = dag->getRegDef(phiCopy);
      const ir::UseSet *phiUse = dag->getRegUse(phi);
      const DefSet *phiDef = dag->getRegDef(phi);
      bool isOpt = true;

      // FIXME, I find under some situation, the phiDef maybe null, seems a bug when building FunctionDAg.
      // need fix it there.
      if (phiDef->empty()) continue;

      const ir::BasicBlock *phiDefBB = (*phiDef->begin())->getInstruction()->getParent();

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

        const ir::Register phiCopySrc = phiCopyDefInsn->getSrc(0);
        const ir::UseSet *phiCopySrcUse = dag->getRegUse(phiCopySrc);
        const ir::DefSet *phiCopySrcDef = dag->getRegDef(phiCopySrc);

        // we should only do coaleasing on instruction-def and ssa-value
        if (phiCopySrcDef->size() == 1 && (*(phiCopySrcDef->begin()))->getType() == ValueDef::DEF_INSN_DST) {
          const ir::Instruction *phiCopySrcDefInsn = (*(phiCopySrcDef->begin()))->getInstruction();
          if(bb == phiDefBB && bb == phiCopySrcDefInsn->getParent()) {
            // phiCopy, phiCopySrc defined in same basicblock as phi
            // try to coalease phiCopy and phiCopySrc first.
            // consider below situation:
            // bb1:
            //    ...
            // bb2:
            //    x = phi [x1, bb1], [x2, bb2]
            //    x2 = x+1;
            // after de-ssa:
            // bb2:
            //    mov x, x-copy
            //    add x2, x, 1
            //    mov x-copy, x2
            //  obviously x2, x-copy and x2 can be mapped to same virtual register

            ir::BasicBlock::const_iterator iter = ir::BasicBlock::const_iterator(phiCopySrcDefInsn);
            ir::BasicBlock::const_iterator iterE = bb->end();

            iter++;
            // check no use of phi in this basicblock between [phiCopySrc def, bb end]
            bool phiPhiCopySrcInterfere = false;
            while (iter != iterE) {
              const ir::Instruction *insn = iter.node();
              // check phiUse
              for (unsigned i = 0; i < insn->getSrcNum(); i++) {
                ir::Register src = insn->getSrc(i);
                if (src == phi) {
                  phiPhiCopySrcInterfere = true; break;
                }
              }
              ++iter;
            }
            if (!phiPhiCopySrcInterfere) {
              replaceSrc(const_cast<Instruction *>(phiCopyDefInsn), phiCopySrc, phiCopy);

              for (auto &s : *phiCopySrcDef) {
                const Instruction *phiSrcDefInsn = s->getInstruction();
                replaceDst(const_cast<Instruction *>(phiSrcDefInsn), phiCopySrc, phiCopy);
              }

              for (auto &s : *phiCopySrcUse) {
                const Instruction *phiSrcUseInsn = s->getInstruction();
                replaceSrc(const_cast<Instruction *>(phiSrcUseInsn), phiCopySrc, phiCopy);
              }
              replaceMap.insert(std::make_pair(phiCopySrc, phiCopy));
            }
          }
        } else {
          // FIXME, if the phiCopySrc is a phi value and has been used for more than one phiCopySrc
          // This 1:1 map will ignore the second one.
          if (((*(phiCopySrcDef->begin()))->getType() == ValueDef::DEF_INSN_DST) &&
              redundantPhiCopyMap.find(phiCopySrc) == redundantPhiCopyMap.end())
            redundantPhiCopyMap.insert(std::make_pair(phiCopySrc, phiCopy));
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

      // coalease phi and phiCopy
      if (isOpt) {
        for (auto &x : *phiDef) {
          replaceDst(const_cast<Instruction *>(x->getInstruction()), phi, phiCopy);
        }
        for (auto &x : *phiUse) {
          const Instruction *phiUseInsn = x->getInstruction();
          replaceSrc(const_cast<Instruction *>(phiUseInsn), phi, phiCopy);
          replaceMap.insert(std::make_pair(phi, phiCopy));
        }
      }
    }
    delete dag;
  }

  void GenWriter::postPhiCopyOptimization(ir::Liveness &liveness,
         ir::Function &fn, map <ir::Register, ir::Register> &replaceMap,
         map <ir::Register, ir::Register> &redundantPhiCopyMap)
  {
    // When doing the first pass phi copy optimization, we skip all the phi src MOV cases
    // whoes phiSrdDefs are also a phi value. We leave it here when all phi copy optimizations
    // have been done. Then we don't need to worry about there are still reducible phi copy remained.
    // We only need to check whether those possible redundant phi copy pairs' interfering to
    // each other globally, by leverage the DAG information.
    using namespace ir;

    // Firstly, validate all possible redundant phi copy map and update liveness information
    // accordingly.
    if (replaceMap.size() != 0) {
      for (auto pair : replaceMap) {
        if (redundantPhiCopyMap.find(pair.first) != redundantPhiCopyMap.end()) {
          auto it = redundantPhiCopyMap.find(pair.first);
          Register phiCopy = it->second;
          Register newPhiCopySrc = pair.second;
          redundantPhiCopyMap.erase(it);
          redundantPhiCopyMap.insert(std::make_pair(newPhiCopySrc, phiCopy));
        }
      }
      liveness.replaceRegs(replaceMap);
      replaceMap.clear();
    }
    if (redundantPhiCopyMap.size() == 0)
      return;
    auto dag = new FunctionDAG(liveness);

    map<Register, Register> newRedundant;
    map<Register, Register> *curRedundant = &redundantPhiCopyMap;
    map<Register, Register> *nextRedundant = &newRedundant, tmp;
    map<Register, Register> replacedRegs, revReplacedRegs;
    // Do multi pass redundant phi copy elimination based on the global interfering information.
    // FIXME, we don't need to re-compute the whole DAG for each pass.
    while (curRedundant->size() > 0) {
      //for (auto &pair = *curRedundant) {
      for (auto pair = curRedundant->begin(); pair != curRedundant->end(); ) {
        auto phiCopySrc = pair->first;
        auto phiCopy = pair->second;
        if (replacedRegs.find(phiCopy) != replacedRegs.end() ||
            revReplacedRegs.find(phiCopy) != revReplacedRegs.end() ||
            revReplacedRegs.find(phiCopySrc) != revReplacedRegs.end()) {
          pair++;
          continue;
        }
        if (!dag->interfere(liveness, phiCopySrc, phiCopy)) {
          const ir::DefSet *phiCopySrcDef = dag->getRegDef(phiCopySrc);
          const ir::UseSet *phiCopySrcUse = dag->getRegUse(phiCopySrc);
          for (auto &s : *phiCopySrcDef) {
            const Instruction *phiSrcDefInsn = s->getInstruction();
            replaceDst(const_cast<Instruction *>(phiSrcDefInsn), phiCopySrc, phiCopy);
          }

          for (auto &s : *phiCopySrcUse) {
            const Instruction *phiSrcUseInsn = s->getInstruction();
            replaceSrc(const_cast<Instruction *>(phiSrcUseInsn), phiCopySrc, phiCopy);
          }

          replacedRegs.insert(std::make_pair(phiCopySrc, phiCopy));
          revReplacedRegs.insert(std::make_pair(phiCopy, phiCopySrc));
          curRedundant->erase(pair++);
        } else
          pair++;
      }

      if (replacedRegs.size() != 0) {
        liveness.replaceRegs(replacedRegs);
        for (auto &pair : *curRedundant) {
          auto from = pair.first;
          auto to = pair.second;
          bool revisit = false;
          if (replacedRegs.find(pair.second) != replacedRegs.end()) {
            to = replacedRegs.find(to)->second;
            revisit = true;
          }
          if (revReplacedRegs.find(from) != revReplacedRegs.end() ||
              revReplacedRegs.find(to) != revReplacedRegs.end())
            revisit = true;
          if (revisit)
            nextRedundant->insert(std::make_pair(from, to));
        }
        std::swap(curRedundant, nextRedundant);
      } else
        break;

      nextRedundant->clear();
      replacedRegs.clear();
      revReplacedRegs.clear();
      delete dag;
      dag = new ir::FunctionDAG(liveness);
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
        // FIXME temporary reserve 4 bytes to avoid 0 address
        if (oldSlm == 0) oldSlm = 4;
        uint32_t align = 8 * getAlignmentByte(unit, ty);
        uint32_t padding = getPadding(oldSlm*8, align);

        f.setSLMSize(oldSlm + padding/8 + getTypeByteSize(unit, ty));

        this->newRegister(const_cast<GlobalVariable*>(&v));
        ir::Register reg = regTranslator.getScalar(const_cast<GlobalVariable*>(&v), 0);
        ctx.LOADI(getType(ctx, v.getType()), reg, ctx.newIntegerImmediate(oldSlm + padding/8, getType(ctx, v.getType())));
        } else if(addrSpace == ir::MEM_CONSTANT
               || addrSpace == ir::MEM_GLOBAL
               || v.isConstant()) {
        if(v.getName().equals(StringRef("__gen_ocl_profiling_buf"))) {
          ctx.getUnit().getProfilingInfo()->setBTI(BtiMap.find(const_cast<GlobalVariable*>(&v))->second);
          regTranslator.newScalarProxy(ir::ocl::profilingbptr, const_cast<GlobalVariable*>(&v));
        } else {
          this->newRegister(const_cast<GlobalVariable*>(&v));
          ir::Register reg = regTranslator.getScalar(const_cast<GlobalVariable*>(&v), 0);
          ir::Constant &con = unit.getConstantSet().getConstant(v.getName());
          ctx.LOADI(getType(ctx, v.getType()), reg, ctx.newIntegerImmediate(con.getOffset(), getType(ctx, v.getType())));
          if (!legacyMode) {
            ctx.ADD(getType(ctx, v.getType()), reg, ir::ocl::constant_addrspace, reg);
          }
        }
      } else if(addrSpace == ir::MEM_PRIVATE) {
          this->newRegister(const_cast<GlobalVariable*>(&v));
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

    for (auto loop : lp) {
      loopBBs.clear();
      loopExits.clear();

      const std::vector<BasicBlock*> &inBBs = loop.first->getBlocks();
      for (auto b : inBBs) {
        GBE_ASSERT(labelMap.find(b) != labelMap.end());
        loopBBs.push_back(labelMap[b]);
      }
      BasicBlock *preheader = loop.first->getLoopPredecessor();
      ir::LabelIndex preheaderBB(0);
      if (preheader) {
        preheaderBB = labelMap[preheader];
      }

      SmallVector<Loop::Edge, 8> exitBBs;
      loop.first->getExitEdges(exitBBs);
      for(auto b : exitBBs){
        GBE_ASSERT(labelMap.find(b.first) != labelMap.end());
        GBE_ASSERT(labelMap.find(b.second) != labelMap.end());
        loopExits.push_back(std::make_pair(labelMap[b.first], labelMap[b.second]));
      }
      fn.addLoop(preheaderBB, loop.second, loopBBs, loopExits);
    }
  }


  static unsigned getChildNo(BasicBlock *bb) {
    TerminatorInst *term = bb->getTerminator();
    return term->getNumSuccessors();
  }

  // return NULL if index out-range of children number
  static BasicBlock *getChildPossible(BasicBlock *bb, unsigned index) {

    TerminatorInst *term = bb->getTerminator();
    unsigned childNo = term->getNumSuccessors();
    BasicBlock *child = NULL;
    if(index < childNo) {
      child = term->getSuccessor(index);
    }
    return child;
  }

/*!

  Sorting Basic blocks is mainly used to solve register liveness issue, take a
  look at below CFG:

       -<--1--
      |       |
      |        ->2
   -- 3 <---     |
  |   ^     |     -->4--
  |   |     |        |  |
  |   |     -----5<--   |
  |   |                 |
  |    ----------6<-----
  |
   -->7

  1.) A register %10 defined in bb4, and used in bb5 & bb6. In normal liveness
  analysis, %10 is not alive in bb3. But under simd execution model, after
  executing bb4, some channel jump through bb5 to bb3, other channel may jump
  to bb6, we must execute bb3 first, then bb6, to avoid missing instructions.
  The physical register of %10 was assigned some value in bb4, but when
  executing bb3, its content may be over-written as it is dead in bb3. When
  jumping back to execute bb6, it will get polluted data. What a disaster!
  What we do here is do a topological sorting of basic blocks, For this case
  we can see the bb3 will be placed after bb5 & bb6. The liveness calculation
  is just as normal and will be correct.

  2.) Another advantage of sorting basic blocks is reducing register pressure.
  In the above CFG, a register defined in bb3 and used in bb7 will be
  alive through 3,4,5,6,7. But in fact it should be only alive in bb3 and bb7.
  After topological sorting, this kind of register would be only alive in bb3
  and bb7. Register pressure in 4,5,6 is reduced.

  3.) Classical post-order traversal will automatically choose a order for the
  successors of a basic block, But this order may be hard to handle, take a look
  at below CFG:

       1 <-----
      /        |
      2 --> 4 -
      |
      3
      |
      5
  In the post oder traversal, it may be: 5->4->3->2->1, as 4, 3 does not have
  strict order. This is a serious issue, a value defined in bb3, used in bb5
  may be overwritten in bb1. Remember the simd execution model? some lanes
  may execute bb4 after other lanes finish bb3, and then jump to bb1, but live
  range of the register does not cover bb1. what we done here is for a loop
  exit (here bb3), we alwasy make sure it is visited first in the post-order
  traversal, for the graph, that means 5->3->4->2->1. Then a definition in bb3,
  and used in 5 will not interfere with any other values defined in the loop.
  FIXME: For irreducible graph, we need to identify it and convert to reducible graph.
*/
  void GenWriter::sortBasicBlock(Function &F) {
    BasicBlock &entry = F.getEntryBlock();
    std::vector<BasicBlock *> visitStack;
    std::vector<BasicBlock *> sorted;
    std::set<BasicBlock *> visited;

    visitStack.push_back(&entry);
    visited.insert(&entry);

    while (!visitStack.empty()) {
      BasicBlock *top = visitStack.back();
      unsigned childNo = getChildNo(top);
      GBE_ASSERT(childNo <= 2);

      BasicBlock *child0 = getChildPossible(top, 0);
      BasicBlock *child1 = getChildPossible(top, 1);
      if(childNo == 2) {
        Loop *loop = LI->getLoopFor(top);
        // visit loop exit node first, so loop exit block will be placed
        // after blocks in loop in 'reverse post-order' list.
        if (loop && loop->contains(child0) && !loop->contains(child1)) {
          BasicBlock *tmp = child0; child0 = child1; child1 = tmp;
        }
      }

      if (child0 != NULL && visited.find(child0) == visited.end()) {
        visitStack.push_back(child0);
        visited.insert(child0);
      } else if (child1 != NULL && visited.find(child1) == visited.end()) {
        visitStack.push_back(child1);
        visited.insert(child1);
      } else {
        sorted.push_back(visitStack.back());
        visitStack.pop_back();
      }
    }

    Function::BasicBlockListType &bbList = F.getBasicBlockList();
    for (std::vector<BasicBlock *>::iterator iter = sorted.begin(); iter != sorted.end(); ++iter) {
      (*iter)->removeFromParent();
    }

    for (std::vector<BasicBlock *>::reverse_iterator iter = sorted.rbegin(); iter != sorted.rend(); ++iter) {
      bbList.push_back(*iter);
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
      case CallingConv::Fast:
      case CallingConv::SPIR_KERNEL:
#endif
        break;
      default:
        GBE_ASSERTM(false, "Unsupported calling convention");
    }

    ctx.startFunction(F.getName());

    ir::Function &fn = ctx.getFunction();
    this->regTranslator.clear();
    this->labelMap.clear();
    this->emitFunctionPrototype(F);

    this->allocateGlobalVariableRegister(F);

    sortBasicBlock(F);
    // Visit all the instructions and emit the IR registers or the value to
    // value mapping when a new register is not needed
    pass = PASS_EMIT_REGISTERS;
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
      visit(*I);
    
    // Abort if this found an error (otherwise emitBasicBlock will assert)
    if(has_errors){return;}

    // First create all the labels (one per block) ...
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      this->newLabelIndex(&*BB);

    // Then, for all branch instructions that have conditions, see if we can
    // simplify the code by inverting condition code
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      this->simplifyTerminator(&*BB);

    // gather loop info, which is useful for liveness analysis
    gatherLoopInfo(fn);

    // ... then, emit the instructions for all basic blocks
    pass = PASS_EMIT_INSTRUCTIONS;
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
      emitBasicBlock(&*BB);
    ctx.endFunction();

    // Liveness can be shared when we optimized the immediates and the MOVs
    ir::Liveness liveness(fn);

    if (OCL_OPTIMIZE_LOADI) this->removeLOADIs(liveness, fn);
    if (OCL_OPTIMIZE_PHI_MOVES) {
      map <ir::Register, ir::Register> replaceMap, redundantPhiCopyMap;
      this->optimizePhiCopy(liveness, fn, replaceMap, redundantPhiCopyMap);
      this->postPhiCopyOptimization(liveness, fn, replaceMap, redundantPhiCopyMap);
      this->removeMOVs(liveness, fn);
    }
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
        Type *dstType = dstValue->getType();
        Type *srcType = srcValue->getType();

        if (getTypeByteSize(unit, dstType) == getTypeByteSize(unit, srcType))
        {
#if GBE_DEBUG
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
      case Instruction::AddrSpaceCast:
        regTranslator.newValueProxy(srcValue, dstValue);
        break;
      default: NOT_SUPPORTED;
    }
  }

  void GenWriter::emitCastInst(CastInst &I) {
    switch (I.getOpcode())
    {
      case Instruction::AddrSpaceCast:
        break;
      case Instruction::PtrToInt:
      case Instruction::IntToPtr:
      {
        Value *dstValue = &I;
        Value *srcValue = I.getOperand(0);
        Type *dstType = dstValue->getType();
        Type *srcType = srcValue->getType();

        if (getTypeByteSize(unit, dstType) != getTypeByteSize(unit, srcType)) {
          const ir::Register dst = this->getRegister(&I);
          const ir::Register src = this->getRegister(srcValue);
          ctx.CVT(getType(ctx, dstType), getType(ctx, srcType), dst, src);
        }
      }
      break;
      case Instruction::BitCast:
      {
        Value *srcValue = I.getOperand(0);
        Value *dstValue = &I;
        uint32_t srcElemNum = 0, dstElemNum = 0 ;
        ir::Type srcType = getVectorInfo(ctx, srcValue, srcElemNum);
        ir::Type dstType = getVectorInfo(ctx, dstValue, dstElemNum);
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
          ir::ImmediateIndex zero;
          if(dstType == ir::TYPE_FLOAT)
            zero = ctx.newFloatImmediate(0);
          else if(dstType == ir::TYPE_DOUBLE)
            zero = ctx.newDoubleImmediate(0);
	  else
            zero = ctx.newIntegerImmediate(0, dstType);
          ir::ImmediateIndex one;
          if (I.getOpcode() == Instruction::SExt
              && (dstType == ir::TYPE_S8 || dstType == ir::TYPE_S16 || dstType == ir::TYPE_S32 || dstType == ir::TYPE_S64))
            one = ctx.newIntegerImmediate(-1, dstType);
          else if(dstType == ir::TYPE_FLOAT)
            one = ctx.newFloatImmediate(1);
          else if(dstType == ir::TYPE_DOUBLE)
            one = ctx.newDoubleImmediate(1);
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
        /* For half <---> float conversion, we use F16TO32 or F32TO16, make the code path same. */
        else if (srcType == ir::TYPE_HALF && dstType == ir::TYPE_FLOAT) {
          ctx.F16TO32(ir::TYPE_FLOAT, ir::TYPE_U16, getRegister(&I), getRegister(I.getOperand(0)));
        } else if (srcType == ir::TYPE_FLOAT && dstType == ir::TYPE_HALF) {
          ctx.F32TO16(ir::TYPE_U16, ir::TYPE_FLOAT, getRegister(&I), getRegister(I.getOperand(0)));
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

  void GenWriter::regAllocateExtractValue(ExtractValueInst &I) {
    Value *agg = I.getAggregateOperand();
    for (const unsigned *i = I.idx_begin(), *e = I.idx_end(); i != e; i++)
      regTranslator.newValueProxy(agg, &I, *i, 0);
  }

  void GenWriter::emitExtractValue(ExtractValueInst &I) {
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
    if(I.getNumArgOperands()) GBE_ASSERT(I.hasStructRetAttr() == false);

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
          case Intrinsic::trap:
          case Intrinsic::dbg_value:
          case Intrinsic::dbg_declare:
          break;
          case Intrinsic::sadd_with_overflow:
          case Intrinsic::uadd_with_overflow:
          case Intrinsic::ssub_with_overflow:
          case Intrinsic::usub_with_overflow:
          case Intrinsic::smul_with_overflow:
          case Intrinsic::umul_with_overflow:
            this->newRegister(&I);
          break;
          case Intrinsic::ctlz:
          case Intrinsic::cttz:
          case Intrinsic::bswap:
            this->newRegister(&I);
          break;
          case Intrinsic::fabs:
          case Intrinsic::sqrt:
          case Intrinsic::ceil:
          case Intrinsic::fma:
          case Intrinsic::trunc:
          case Intrinsic::rint:
          case Intrinsic::floor:
          case Intrinsic::sin:
          case Intrinsic::cos:
          case Intrinsic::log2:
          case Intrinsic::exp2:
          case Intrinsic::pow:
            this->newRegister(&I);
          break;
          default:
          GBE_ASSERTM(false, "Unsupported intrinsics");
        }
        return;
      }
    }
    // Get the name of the called function and handle it
    const std::string fnName = Callee->stripPointerCasts()->getName();
    auto genIntrinsicID = intrinsicMap.find(fnName);
    switch (genIntrinsicID) {
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
      case GEN_OCL_GET_ENQUEUED_LOCAL_SIZE0:
        regTranslator.newScalarProxy(ir::ocl::enqlsize0, dst); break;
      case GEN_OCL_GET_ENQUEUED_LOCAL_SIZE1:
        regTranslator.newScalarProxy(ir::ocl::enqlsize1, dst); break;
      case GEN_OCL_GET_ENQUEUED_LOCAL_SIZE2:
        regTranslator.newScalarProxy(ir::ocl::enqlsize2, dst); break;
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
      case GEN_OCL_GET_THREAD_NUM:
        regTranslator.newScalarProxy(ir::ocl::threadn, dst); break;
      case GEN_OCL_GET_THREAD_ID:
        regTranslator.newScalarProxy(ir::ocl::threadid, dst); break;
      case GEN_OCL_GET_WORK_DIM:
        regTranslator.newScalarProxy(ir::ocl::workdim, dst); break;
      case GEN_OCL_FBH:
      case GEN_OCL_FBL:
      case GEN_OCL_CBIT:
      case GEN_OCL_RSQ:
      case GEN_OCL_RCP:
      case GEN_OCL_ABS:
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
      case GEN_OCL_BARRIER:
        ctx.getFunction().setUseSLM(true);
        break;
      case GEN_OCL_WRITE_IMAGE_I:
      case GEN_OCL_WRITE_IMAGE_UI:
      case GEN_OCL_WRITE_IMAGE_F:
        break;
      case GEN_OCL_READ_IMAGE_I:
      case GEN_OCL_READ_IMAGE_UI:
      case GEN_OCL_READ_IMAGE_F:
      {
        // dst is a 4 elements vector. We allocate all 4 registers here.
        uint32_t elemNum;
        (void)getVectorInfo(ctx, &I, elemNum);
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
      case GEN_OCL_SAT_CONV_F16_TO_I8:
      case GEN_OCL_SAT_CONV_F16_TO_U8:
      case GEN_OCL_SAT_CONV_F16_TO_I16:
      case GEN_OCL_SAT_CONV_F16_TO_U16:
      case GEN_OCL_SAT_CONV_F16_TO_I32:
      case GEN_OCL_SAT_CONV_F16_TO_U32:
      case GEN_OCL_CONV_F16_TO_F32:
      case GEN_OCL_CONV_F32_TO_F16:
      case GEN_OCL_SIMD_ANY:
      case GEN_OCL_SIMD_ALL:
      case GEN_OCL_SIMD_SIZE:
      case GEN_OCL_READ_TM:
      case GEN_OCL_REGION:
      case GEN_OCL_IN_PRIVATE:
      case GEN_OCL_SIMD_ID:
      case GEN_OCL_SIMD_SHUFFLE:
      case GEN_OCL_VME:
      case GEN_OCL_WORK_GROUP_ALL:
      case GEN_OCL_WORK_GROUP_ANY:
      case GEN_OCL_WORK_GROUP_BROADCAST:
      case GEN_OCL_WORK_GROUP_REDUCE_ADD:
      case GEN_OCL_WORK_GROUP_REDUCE_MAX:
      case GEN_OCL_WORK_GROUP_REDUCE_MIN:
      case GEN_OCL_WORK_GROUP_SCAN_EXCLUSIVE_ADD:
      case GEN_OCL_WORK_GROUP_SCAN_EXCLUSIVE_MAX:
      case GEN_OCL_WORK_GROUP_SCAN_EXCLUSIVE_MIN:
      case GEN_OCL_WORK_GROUP_SCAN_INCLUSIVE_ADD:
      case GEN_OCL_WORK_GROUP_SCAN_INCLUSIVE_MAX:
      case GEN_OCL_WORK_GROUP_SCAN_INCLUSIVE_MIN:
      case GEN_OCL_SUB_GROUP_BROADCAST:
      case GEN_OCL_SUB_GROUP_REDUCE_ADD:
      case GEN_OCL_SUB_GROUP_REDUCE_MAX:
      case GEN_OCL_SUB_GROUP_REDUCE_MIN:
      case GEN_OCL_SUB_GROUP_SCAN_EXCLUSIVE_ADD:
      case GEN_OCL_SUB_GROUP_SCAN_EXCLUSIVE_MAX:
      case GEN_OCL_SUB_GROUP_SCAN_EXCLUSIVE_MIN:
      case GEN_OCL_SUB_GROUP_SCAN_INCLUSIVE_ADD:
      case GEN_OCL_SUB_GROUP_SCAN_INCLUSIVE_MAX:
      case GEN_OCL_SUB_GROUP_SCAN_INCLUSIVE_MIN:
      case GEN_OCL_LRP:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM2:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM4:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM8:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE2:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE4:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE8:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM2:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM4:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM8:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE2:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE4:
      case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE8:
      case GEN_OCL_ENQUEUE_SET_NDRANGE_INFO:
      case GEN_OCL_ENQUEUE_GET_NDRANGE_INFO:
        this->newRegister(&I);
        break;
      case GEN_OCL_GET_PIPE:
      {
        Value *srcValue = I.getOperand(0);
        if( BtiMap.find(dst) == BtiMap.end())
        {
          unsigned tranBti = BtiMap.find(srcValue)->second;
          BtiMap.insert(std::make_pair(dst, tranBti));
        }
        regTranslator.newValueProxy(srcValue, dst);
        break;
      }
      case GEN_OCL_MAKE_RID:
      case GEN_OCL_GET_RID:
      {
        Value *srcValue = I.getOperand(0);
        regTranslator.newValueProxy(srcValue, dst);
        break;
      }
      case GEN_OCL_ENQUEUE_GET_ENQUEUE_INFO_ADDR:
        regTranslator.newScalarProxy(ir::ocl::enqueuebufptr, dst);
        break;
      case GEN_OCL_PRINTF:
        this->newRegister(&I);  // fall through
      case GEN_OCL_PUTS:
      {
         // We need a new BTI as printf output.
         if (printfBti < 0) {
           printfBti = this->getNewBti(&I, true);
           ctx.getFunction().getPrintfSet()->setBufBTI(printfBti);
         }
         break;
      }
      case GEN_OCL_CALC_TIMESTAMP:
      case GEN_OCL_STORE_PROFILING:
      case GEN_OCL_DEBUGWAIT:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM2:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM4:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM8:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE2:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE4:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE8:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM2:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM4:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM8:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE2:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE4:
      case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE8:
        break;
      case GEN_OCL_NOT_FOUND:
      default:
        has_errors = true;
        Func->getContext().emitError(&I,"function '" + fnName + "' not found or cannot be inlined");
    };
  }

  void GenWriter::emitUnaryCallInst(CallInst &I, CallSite &CS, ir::Opcode opcode, ir::Type type) {
    CallSite::arg_iterator AI = CS.arg_begin();
#if GBE_DEBUG
    CallSite::arg_iterator AE = CS.arg_end();
#endif /* GBE_DEBUG */
    GBE_ASSERT(AI != AE);
    const ir::Register src = this->getRegister(*AI);
    const ir::Register dst = this->getRegister(&I);
    ctx.ALU1(opcode, type, dst, src);
  }

  void GenWriter::regAllocateAtomicCmpXchgInst(AtomicCmpXchgInst &I) {
    this->newRegister(&I);
  }

  void GenWriter::emitAtomicInstHelper(const ir::AtomicOps opcode,const ir::Type type, const ir::Register dst, llvm::Value* llvmPtr, const ir::Tuple payloadTuple) {
    ir::Register pointer = this->getRegister(llvmPtr);
    ir::AddressSpace addrSpace = addressSpaceLLVMToGen(llvmPtr->getType()->getPointerAddressSpace());
    // Get the function arguments
    ir::Register ptr;
    ir::Register btiReg;
    unsigned SurfaceIndex = 0xff;
    ir::AddressMode AM;
    if (legacyMode) {
      Value *bti = getBtiRegister(llvmPtr);
      Value *ptrBase = getPointerBase(llvmPtr);
      ir::Register baseReg = this->getRegister(ptrBase);
      if (isa<ConstantInt>(bti)) {
        AM = ir::AM_StaticBti;
        SurfaceIndex = cast<ConstantInt>(bti)->getZExtValue();
        addrSpace = btiToGen(SurfaceIndex);
      } else {
        AM = ir::AM_DynamicBti;
        addrSpace = ir::MEM_MIXED;
        btiReg = this->getRegister(bti);
      }
      const ir::RegisterFamily pointerFamily = ctx.getPointerFamily();
      ptr = ctx.reg(pointerFamily);
      ctx.SUB(ir::TYPE_U32, ptr, pointer, baseReg);
    } else {
      AM = ir::AM_Stateless;
      ptr = pointer;
    }

    ctx.ATOMIC(opcode, type, dst, addrSpace, ptr, payloadTuple, AM, SurfaceIndex);
  }

  void GenWriter::emitAtomicCmpXchgInst(AtomicCmpXchgInst &I) {
    // Get the function arguments
    Value *llvmPtr = I.getPointerOperand();
    ir::AtomicOps opcode = ir::ATOMIC_OP_CMPXCHG;
    uint32_t payloadNum = 0;
    vector<ir::Register> payload;
    const ir::Register oldValue = this->getRegister(&I, 0);
    const ir::Register compareRet = this->getRegister(&I, 1);
    const ir::Register expected = this->getRegister(I.getCompareOperand());

    payload.push_back(this->getRegister(I.getCompareOperand()));
    payloadNum++;
    payload.push_back(this->getRegister(I.getNewValOperand()));
    payloadNum++;
    ir::Type type = getType(ctx, llvmPtr->getType()->getPointerElementType());
    const ir::Tuple payloadTuple = payloadNum == 0 ?
                                   ir::Tuple(0) :
                                   ctx.arrayTuple(&payload[0], payloadNum);
    this->emitAtomicInstHelper(opcode, type, oldValue, llvmPtr, payloadTuple);
    ctx.EQ(type, compareRet, oldValue, expected);
  }

  void GenWriter::regAllocateAtomicRMWInst(AtomicRMWInst &I) {
    this->newRegister(&I);
  }

  static INLINE ir::AtomicOps atomicOpsLLVMToGen(llvm::AtomicRMWInst::BinOp llvmOp) {
    switch(llvmOp) {
      case llvm::AtomicRMWInst::Xchg: return ir::ATOMIC_OP_XCHG;
      case llvm::AtomicRMWInst::Add:  return ir::ATOMIC_OP_ADD;
      case llvm::AtomicRMWInst::Sub:  return ir::ATOMIC_OP_SUB;
      case llvm::AtomicRMWInst::And:  return ir::ATOMIC_OP_AND;
      case llvm::AtomicRMWInst::Or:   return ir::ATOMIC_OP_OR;
      case llvm::AtomicRMWInst::Xor:  return ir::ATOMIC_OP_XOR;
      case llvm::AtomicRMWInst::Max:  return ir::ATOMIC_OP_IMAX;
      case llvm::AtomicRMWInst::Min:  return ir::ATOMIC_OP_IMIN;
      case llvm::AtomicRMWInst::UMax: return ir::ATOMIC_OP_UMAX;
      case llvm::AtomicRMWInst::UMin: return ir::ATOMIC_OP_UMIN;
      case llvm::AtomicRMWInst::Nand:
      case llvm::AtomicRMWInst::BAD_BINOP: break;
    }
    GBE_ASSERT(false);
    return ir::ATOMIC_OP_INVALID;
  }

  void GenWriter::emitAtomicRMWInst(AtomicRMWInst &I) {
    // Get the function arguments
    llvm::AtomicRMWInst::BinOp llvmOpcode = I.getOperation();
    Value *llvmPtr = I.getOperand(0);
    ir::AtomicOps opcode = atomicOpsLLVMToGen(llvmOpcode);

    const ir::Register dst = this->getRegister(&I);

    uint32_t payloadNum = 0;
    vector<ir::Register> payload;

    payload.push_back(this->getRegister(I.getOperand(1)));
    payloadNum++;
    ir::Type type = getType(ctx, llvmPtr->getType()->getPointerElementType());
    const ir::Tuple payloadTuple = payloadNum == 0 ?
                                   ir::Tuple(0) :
                                   ctx.arrayTuple(&payload[0], payloadNum);
    this->emitAtomicInstHelper(opcode, type, dst, llvmPtr, payloadTuple);
  }

  void GenWriter::emitAtomicInst(CallInst &I, CallSite &CS, ir::AtomicOps opcode) {
    CallSite::arg_iterator AI = CS.arg_begin();
    CallSite::arg_iterator AE = CS.arg_end();
    GBE_ASSERT(AI != AE);
    Value *llvmPtr = *AI;
    ir::AddressSpace addrSpace = addressSpaceLLVMToGen(llvmPtr->getType()->getPointerAddressSpace());
    ir::Register pointer = this->getRegister(llvmPtr);

    ir::Register ptr;
    ir::Register btiReg;
    unsigned SurfaceIndex = 0xff;;

    ir::AddressMode AM;
    if (legacyMode) {
      Value *bti = getBtiRegister(llvmPtr);
      Value *ptrBase = getPointerBase(llvmPtr);
      ir::Register baseReg = this->getRegister(ptrBase);
      if (isa<ConstantInt>(bti)) {
        AM = ir::AM_StaticBti;
        SurfaceIndex = cast<ConstantInt>(bti)->getZExtValue();
        addrSpace = btiToGen(SurfaceIndex);
      } else {
        AM = ir::AM_DynamicBti;
        addrSpace = ir::MEM_MIXED;
        btiReg = this->getRegister(bti);
      }
      const ir::RegisterFamily pointerFamily = ctx.getPointerFamily();
      ptr = ctx.reg(pointerFamily);
      ctx.SUB(ir::TYPE_U32, ptr, pointer, baseReg);
    } else {
      AM = ir::AM_Stateless;
      ptr = pointer;
    }

    const ir::Register dst = this->getRegister(&I);

    uint32_t payloadNum = 0;
    vector<ir::Register> payload;
    AI++;

    while(AI != AE) {
      payload.push_back(this->getRegister(*(AI++)));
      payloadNum++;
    }
    ir::Type type = getType(ctx, llvmPtr->getType()->getPointerElementType());
    const ir::Tuple payloadTuple = payloadNum == 0 ?
                                   ir::Tuple(0) :
                                   ctx.arrayTuple(&payload[0], payloadNum);
    if (AM == ir::AM_DynamicBti) {
      ctx.ATOMIC(opcode, type, dst, addrSpace, ptr, payloadTuple, AM, btiReg);
    } else {
      ctx.ATOMIC(opcode, type, dst, addrSpace, ptr, payloadTuple, AM, SurfaceIndex);
    }
  }

  void GenWriter::emitWorkGroupInst(CallInst &I, CallSite &CS, ir::WorkGroupOps opcode) {
    ir::Function &f = ctx.getFunction();

    if (f.getwgBroadcastSLM() < 0 && opcode == ir::WORKGROUP_OP_BROADCAST) {
      uint32_t mapSize = 8;
      f.setUseSLM(true);
      uint32_t oldSlm = f.getSLMSize();
      f.setSLMSize(oldSlm + mapSize);
      f.setwgBroadcastSLM(oldSlm);
      GBE_ASSERT(f.getwgBroadcastSLM() >= 0);
    }

    else if (f.gettidMapSLM() < 0 && opcode >= ir::WORKGROUP_OP_ANY && opcode <= ir::WORKGROUP_OP_EXCLUSIVE_MAX) {
      /* 1. For thread SLM based communication (default):
       * Threads will use SLM to write partial results computed individually
         and then read the whole set. Because the read is done in chunks of 4
         extra padding is required.

         When we come to here, the global thread local vars should have all been
         allocated, so it's safe for us to steal a piece of SLM for this usage. */

      // at most 64 thread for one subslice, along with extra padding
      uint32_t mapSize = sizeof(uint32_t) * (64 + 4);
      f.setUseSLM(true);
      uint32_t oldSlm = f.getSLMSize();
      f.setSLMSize(oldSlm + mapSize);
      f.settidMapSLM(oldSlm);
      GBE_ASSERT(f.gettidMapSLM() >= 0);
    }

    CallSite::arg_iterator AI = CS.arg_begin();
    CallSite::arg_iterator AE = CS.arg_end();
    GBE_ASSERT(AI != AE);

    if (opcode == ir::WORKGROUP_OP_ALL || opcode == ir::WORKGROUP_OP_ANY) {
      GBE_ASSERT(getType(ctx, (*AI)->getType()) == ir::TYPE_S32);
      ir::Register src[3];
      src[0] = ir::ocl::threadn;
      src[1] = ir::ocl::threadid;
      src[2] = this->getRegister(*(AI++));
      const ir::Tuple srcTuple = ctx.arrayTuple(&src[0], 3);
      ctx.WORKGROUP(opcode, (uint32_t)f.gettidMapSLM(), getRegister(&I), srcTuple, 3, ir::TYPE_S32);
    } else if (opcode == ir::WORKGROUP_OP_BROADCAST) {
      int argNum = CS.arg_size();
      std::vector<ir::Register> src(argNum);
      for (int i = 0; i < argNum; i++) {
        src[i] = this->getRegister(*(AI++));
      }
      const ir::Tuple srcTuple = ctx.arrayTuple(&src[0], argNum);
      ctx.WORKGROUP(ir::WORKGROUP_OP_BROADCAST, (uint32_t)f.getwgBroadcastSLM(), getRegister(&I), srcTuple, argNum,
          getType(ctx, (*CS.arg_begin())->getType()));
    } else {
      ConstantInt *sign = dyn_cast<ConstantInt>(AI);
      GBE_ASSERT(sign);
      bool isSign = sign->getZExtValue();
      AI++;
      ir::Type ty;
      if (isSign) {
        ty = getType(ctx, (*AI)->getType());

      } else {
        ty = getUnsignedType(ctx, (*AI)->getType());
      }

      ir::Register src[3];
      src[0] = ir::ocl::threadn;
      src[1] = ir::ocl::threadid;
      src[2] = this->getRegister(*(AI++));
      const ir::Tuple srcTuple = ctx.arrayTuple(&src[0], 3);
      ctx.WORKGROUP(opcode, (uint32_t)f.gettidMapSLM(), getRegister(&I), srcTuple, 3, ty);
    }

    GBE_ASSERT(AI == AE);
  }

  void GenWriter::emitSubGroupInst(CallInst &I, CallSite &CS, ir::WorkGroupOps opcode) {
    CallSite::arg_iterator AI = CS.arg_begin();
    CallSite::arg_iterator AE = CS.arg_end();
    GBE_ASSERT(AI != AE);

    if (opcode == ir::WORKGROUP_OP_ALL || opcode == ir::WORKGROUP_OP_ANY) {
      GBE_ASSERT(getType(ctx, (*AI)->getType()) == ir::TYPE_S32);
      ir::Register src[3];
      src[0] = this->getRegister(*(AI++));
      const ir::Tuple srcTuple = ctx.arrayTuple(&src[0], 1);
      ctx.SUBGROUP(opcode, getRegister(&I), srcTuple, 1, ir::TYPE_S32);
    } else if (opcode == ir::WORKGROUP_OP_BROADCAST) {
      int argNum = CS.arg_size();
      GBE_ASSERT(argNum == 2);
      std::vector<ir::Register> src(argNum);
      for (int i = 0; i < argNum; i++) {
        src[i] = this->getRegister(*(AI++));
      }
      const ir::Tuple srcTuple = ctx.arrayTuple(&src[0], argNum);
      ctx.SUBGROUP(ir::WORKGROUP_OP_BROADCAST, getRegister(&I), srcTuple, argNum,
          getType(ctx, (*CS.arg_begin())->getType()));
    } else {
      ConstantInt *sign = dyn_cast<ConstantInt>(AI);
      GBE_ASSERT(sign);
      bool isSign = sign->getZExtValue();
      AI++;
      ir::Type ty;
      if (isSign) {
        ty = getType(ctx, (*AI)->getType());

      } else {
        ty = getUnsignedType(ctx, (*AI)->getType());
      }

      ir::Register src[3];
      src[0] = this->getRegister(*(AI++));
      const ir::Tuple srcTuple = ctx.arrayTuple(&src[0], 1);
      ctx.SUBGROUP(opcode, getRegister(&I), srcTuple, 1, ty);
    }

    GBE_ASSERT(AI == AE);
  }

  void GenWriter::emitBlockReadWriteMemInst(CallInst &I, CallSite &CS, bool isWrite, uint8_t vec_size, ir::Type type) {
    CallSite::arg_iterator AI = CS.arg_begin();
    CallSite::arg_iterator AE = CS.arg_end();
    GBE_ASSERT(AI != AE);

    Value *llvmPtr = *(AI++);
    ir::AddressSpace addrSpace = addressSpaceLLVMToGen(llvmPtr->getType()->getPointerAddressSpace());
    GBE_ASSERT(addrSpace == ir::MEM_GLOBAL);
    ir::Register pointer = this->getRegister(llvmPtr);

    ir::Register ptr;
    ir::Register btiReg;
    unsigned SurfaceIndex = 0xff;

    ir::AddressMode AM;
    if (legacyMode) {
      Value *bti = getBtiRegister(llvmPtr);
      Value *ptrBase = getPointerBase(llvmPtr);
      ir::Register baseReg = this->getRegister(ptrBase);
      if (isa<ConstantInt>(bti)) {
        AM = ir::AM_StaticBti;
        SurfaceIndex = cast<ConstantInt>(bti)->getZExtValue();
        addrSpace = btiToGen(SurfaceIndex);
      } else {
        AM = ir::AM_DynamicBti;
        addrSpace = ir::MEM_MIXED;
        btiReg = this->getRegister(bti);
      }
      const ir::RegisterFamily pointerFamily = ctx.getPointerFamily();
      ptr = ctx.reg(pointerFamily);
      ctx.SUB(ir::TYPE_U32, ptr, pointer, baseReg);
    } else {
      AM = ir::AM_Stateless;
      ptr = pointer;
    }

    GBE_ASSERT(AM != ir::AM_DynamicBti);

    if(isWrite){
      Value *llvmValues = *(AI++);
      vector<ir::Register> srcTupleData;
      for(int i = 0;i < vec_size; i++)
        srcTupleData.push_back(getRegister(llvmValues, i));
      const ir::Tuple tuple = ctx.arrayTuple(&srcTupleData[0], vec_size);
      ctx.STORE(type, tuple, ptr, addrSpace, vec_size, true, AM, SurfaceIndex, true);
    } else {
      vector<ir::Register> dstTupleData;
      for(int i = 0;i < vec_size; i++)
        dstTupleData.push_back(getRegister(&I, i));
      const ir::Tuple tuple = ctx.arrayTuple(&dstTupleData[0], vec_size);
      ctx.LOAD(type, tuple, ptr, addrSpace, vec_size, true, AM, SurfaceIndex, true);
    }

    GBE_ASSERT(AI == AE);
  }

  void GenWriter::emitBlockReadWriteImageInst(CallInst &I, CallSite &CS, bool isWrite, uint8_t vec_size, ir::Type type) {
    CallSite::arg_iterator AI = CS.arg_begin();
    CallSite::arg_iterator AE = CS.arg_end();
    GBE_ASSERT(AI != AE);

    const uint8_t imageID = getImageID(I);
    AI++;

    if(isWrite){
      vector<ir::Register> srcTupleData;
      srcTupleData.push_back(getRegister(*(AI++)));
      srcTupleData.push_back(getRegister(*(AI++)));
      for(int i = 0;i < vec_size; i++)
        srcTupleData.push_back(getRegister(*(AI), i));
      AI++;
      const ir::Tuple srctuple = ctx.arrayTuple(&srcTupleData[0], 2 + vec_size);
      ctx.MBWRITE(imageID, srctuple, 2 + vec_size, vec_size, type);
    } else {
      ir::Register src[2];
      src[0] = getRegister(*(AI++));
      src[1] = getRegister(*(AI++));
      vector<ir::Register> dstTupleData;
      for(int i = 0;i < vec_size; i++)
        dstTupleData.push_back(getRegister(&I, i));
      const ir::Tuple srctuple = ctx.arrayTuple(src, 2);
      const ir::Tuple dsttuple = ctx.arrayTuple(&dstTupleData[0], vec_size);
      ctx.MBREAD(imageID, dsttuple, vec_size, srctuple, 2, type);
    }

    GBE_ASSERT(AI == AE);
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
      GBE_ASSERTM(x.getType() == ir::TYPE_U32 || x.getType() == ir::TYPE_S32, "Invalid sampler type");

      index = ctx.getFunction().getSamplerSet()->append(x.getIntegerValue(), &ctx);
    } else {
      const ir::Register samplerReg = this->getRegister(*AI);
      index = ctx.getFunction().getSamplerSet()->append(samplerReg, &ctx);
    }
    return index;
  }

  uint8_t GenWriter::getImageID(CallInst &I) {
    const ir::Register imageReg = this->getRegister(I.getOperand(0));
    return ctx.getFunction().getImageSet()->getIdx(imageReg);
  }

  void GenWriter::emitCallInst(CallInst &I) {
    if (Function *F = I.getCalledFunction()) {
      if (F->getIntrinsicID() != 0) {
        const ir::Function &fn = ctx.getFunction();

        // Get the function arguments
        CallSite CS(&I);
        CallSite::arg_iterator AI = CS.arg_begin();
#if GBE_DEBUG
        CallSite::arg_iterator AE = CS.arg_end();
#endif /* GBE_DEBUG */
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
          case Intrinsic::lifetime_start:
          case Intrinsic::lifetime_end:
          break;
#endif /* LLVM_VERSION_MINOR >= 2 */
          case Intrinsic::debugtrap:
          case Intrinsic::trap:
          case Intrinsic::dbg_value:
          case Intrinsic::dbg_declare:
          break;
          case Intrinsic::uadd_with_overflow:
          {
            Type *llvmDstType = I.getType();
            GBE_ASSERT(llvmDstType->isStructTy());
            ir::Type dst0Type = getType(ctx, llvmDstType->getStructElementType(0));
            const ir::Register dst0  = this->getRegister(&I, 0);
            const ir::Register src0 = this->getRegister(I.getOperand(0));
            const ir::Register src1 = this->getRegister(I.getOperand(1));
            ctx.ADD(dst0Type, dst0, src0, src1);

            ir::Register overflow = this->getRegister(&I, 1);
            const ir::Type unsignedType = makeTypeUnsigned(dst0Type);
            ctx.LT(unsignedType, overflow, dst0, src1);
          }
          break;
          case Intrinsic::usub_with_overflow:
          {
            Type *llvmDstType = I.getType();
            GBE_ASSERT(llvmDstType->isStructTy());
            ir::Type dst0Type = getType(ctx, llvmDstType->getStructElementType(0));
            const ir::Register dst0  = this->getRegister(&I, 0);
            const ir::Register src0 = this->getRegister(I.getOperand(0));
            const ir::Register src1 = this->getRegister(I.getOperand(1));
            ctx.SUB(dst0Type, dst0, src0, src1);

            ir::Register overflow = this->getRegister(&I, 1);
            const ir::Type unsignedType = makeTypeUnsigned(dst0Type);
            ctx.GT(unsignedType, overflow, dst0, src0);
          }
          break;
          case Intrinsic::sadd_with_overflow:
          case Intrinsic::ssub_with_overflow:
          case Intrinsic::smul_with_overflow:
          case Intrinsic::umul_with_overflow:
          NOT_IMPLEMENTED;
          break;
          case Intrinsic::ctlz:
          {
            Type *llvmDstType = I.getType();
            ir::Type dstType = getType(ctx, llvmDstType);
            Type *llvmSrcType = I.getOperand(0)->getType();
            ir::Type srcType = getUnsignedType(ctx, llvmSrcType);

            //the llvm.ctlz.i64 is lowered to two llvm.ctlz.i32 call in ocl_clz.ll
            GBE_ASSERT(srcType != ir::TYPE_U64);

            const ir::Register dst = this->getRegister(&I);
            const ir::Register src = this->getRegister(I.getOperand(0));
            int imm_value = 0;
            if(srcType == ir::TYPE_U16) {
              imm_value = 16;
            }else if(srcType == ir::TYPE_U8) {
              imm_value = 24;
            }

            if(srcType == ir::TYPE_U16 || srcType == ir::TYPE_U8) {
              ir::ImmediateIndex imm;
              ir::Type tmpType = ir::TYPE_S32;
              imm = ctx.newIntegerImmediate(imm_value, tmpType);
              const ir::RegisterFamily family = getFamily(tmpType);
              const ir::Register immReg = ctx.reg(family);
              ctx.LOADI(ir::TYPE_S32, immReg, imm);

              ir::Register tmp0 = ctx.reg(getFamily(tmpType));
              ir::Register tmp1 = ctx.reg(getFamily(tmpType));
              ir::Register tmp2 = ctx.reg(getFamily(tmpType));
              ctx.CVT(tmpType, srcType, tmp0, src);
              ctx.ALU1(ir::OP_LZD, ir::TYPE_U32, tmp1, tmp0);
              ctx.SUB(tmpType, tmp2, tmp1, immReg);
              ctx.CVT(dstType, tmpType, dst, tmp2);
            }
            else
            {
              GBE_ASSERT(srcType == ir::TYPE_U32);
              ctx.ALU1(ir::OP_LZD, srcType, dst, src);
            }
          }
          break;
          case Intrinsic::cttz:
          {
            Type *llvmDstType = I.getType();
            ir::Type dstType = getType(ctx, llvmDstType);
            Type *llvmSrcType = I.getOperand(0)->getType();
            ir::Type srcType = getUnsignedType(ctx, llvmSrcType);

            //the llvm.ctlz.i64 is lowered to two llvm.cttz.i32 call in ocl_ctz.ll
            GBE_ASSERT(srcType != ir::TYPE_U64);

            const ir::Register dst = this->getRegister(&I);
            const ir::Register src = this->getRegister(I.getOperand(0));

            uint32_t imm_value = 0;
            if(srcType == ir::TYPE_U16) {
              imm_value = 0xFFFF0000;
            }else if(srcType == ir::TYPE_U8) {
              imm_value = 0xFFFFFF00;
            }
            if(srcType == ir::TYPE_U16 || srcType == ir::TYPE_U8) {
              ir::ImmediateIndex imm;
              ir::Type tmpType = ir::TYPE_S32;
              ir::Type revType = ir::TYPE_U32;
              imm = ctx.newIntegerImmediate(imm_value, revType);
              const ir::RegisterFamily family = getFamily(revType);
              const ir::Register immReg = ctx.reg(family);
              ctx.LOADI(ir::TYPE_U32, immReg, imm);

              ir::Register tmp0 = ctx.reg(getFamily(tmpType));
              ir::Register tmp1 = ctx.reg(getFamily(revType));
              ir::Register tmp2 = ctx.reg(getFamily(revType));
              ir::Register revTmp = ctx.reg(getFamily(revType));

              ctx.CVT(tmpType, srcType, tmp0, src);
              //gen does not have 'tzd', so reverse first
              ctx.ADD(revType, tmp1, tmp0, immReg);
              ctx.ALU1(ir::OP_BFREV, revType, revTmp, tmp1);
              ctx.ALU1(ir::OP_LZD, ir::TYPE_U32, tmp2, revTmp);
              ctx.CVT(dstType, tmpType, dst, tmp2);
            }
            else
            {
              GBE_ASSERT(srcType == ir::TYPE_U32);
              ir::Type revType = ir::TYPE_U32;
              ir::Register revTmp = ctx.reg(getFamily(revType));
              ctx.ALU1(ir::OP_BFREV, revType, revTmp, src);
              ctx.ALU1(ir::OP_LZD, ir::TYPE_U32, dst, revTmp);
            }
          }
          break;
          case Intrinsic::fma:
          case Intrinsic::fmuladd:
          {
            ir::Type srcType = getType(ctx, I.getType());
            const ir::Register dst = this->getRegister(&I);
            const ir::Register src0 = this->getRegister(I.getOperand(0));
            const ir::Register src1 = this->getRegister(I.getOperand(1));
            const ir::Register src2 = this->getRegister(I.getOperand(2));
            ctx.MAD(srcType, dst, src0, src1, src2);
          }
          break;
          case Intrinsic::sqrt: this->emitUnaryCallInst(I,CS,ir::OP_SQR); break;
          case Intrinsic::ceil: this->emitUnaryCallInst(I,CS,ir::OP_RNDU); break;
          case Intrinsic::trunc: this->emitUnaryCallInst(I,CS,ir::OP_RNDZ); break;
          case Intrinsic::rint: this->emitUnaryCallInst(I,CS,ir::OP_RNDE); break;
          case Intrinsic::floor: this->emitUnaryCallInst(I,CS,ir::OP_RNDD); break;
          case Intrinsic::sin: this->emitUnaryCallInst(I,CS,ir::OP_SIN); break;
          case Intrinsic::cos: this->emitUnaryCallInst(I,CS,ir::OP_COS); break;
          case Intrinsic::log2: this->emitUnaryCallInst(I,CS,ir::OP_LOG); break;
          case Intrinsic::exp2: this->emitUnaryCallInst(I,CS,ir::OP_EXP); break;
          case Intrinsic::bswap:
            this->emitUnaryCallInst(I,CS,ir::OP_BSWAP, getUnsignedType(ctx, I.getType())); break;
          case Intrinsic::pow:
          {
            const ir::Register src0 = this->getRegister(*AI); ++AI;
            const ir::Register src1 = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.POW(ir::TYPE_FLOAT, dst, src0, src1);
            break;
          }
          case Intrinsic::fabs:
          {
            const ir::Register src = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU1(ir::OP_ABS, getType(ctx, (*AI)->getType()), dst, src);
            break;
          }
          default: NOT_IMPLEMENTED;
        }
      } else {
        // Get the name of the called function and handle it
        Value *Callee = I.getCalledValue();
        const std::string fnName = Callee->stripPointerCasts()->getName();
        auto genIntrinsicID = intrinsicMap.find(fnName);

        // Get the function arguments
        CallSite CS(&I);
        CallSite::arg_iterator AI = CS.arg_begin();
#if GBE_DEBUG
        CallSite::arg_iterator AE = CS.arg_end();
#endif /* GBE_DEBUG */

        switch (genIntrinsicID) {
          case GEN_OCL_FBH: this->emitUnaryCallInst(I,CS,ir::OP_FBH, ir::TYPE_U32); break;
          case GEN_OCL_FBL: this->emitUnaryCallInst(I,CS,ir::OP_FBL, ir::TYPE_U32); break;
          case GEN_OCL_CBIT: this->emitUnaryCallInst(I,CS,ir::OP_CBIT, getUnsignedType(ctx, (*AI)->getType())); break;
          case GEN_OCL_ABS:
          {
            const ir::Register src = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU1(ir::OP_ABS, getType(ctx, (*AI)->getType()), dst, src);
            break;
          }
          case GEN_OCL_SIMD_ALL:
          {
            const ir::Register src = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU1(ir::OP_SIMD_ALL, ir::TYPE_S32, dst, src);
            break;
          }
          case GEN_OCL_SIMD_ANY:
          {
            const ir::Register src = this->getRegister(*AI);
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU1(ir::OP_SIMD_ANY, ir::TYPE_S32, dst, src);
            break;
          }
          case GEN_OCL_READ_TM:
          {
            const ir::Register dst = this->getRegister(&I);
            ctx.READ_ARF(ir::TYPE_U32, dst, ir::ARF_TM);
            break;
          }
          case GEN_OCL_VME:
          {

            const uint8_t imageID = getImageID(I);

            AI++;
            AI++;

            uint32_t src_length = 40;

            vector<ir::Register> dstTupleData, srcTupleData;
            for (uint32_t i = 0; i < src_length; i++, AI++){
              srcTupleData.push_back(this->getRegister(*AI));
            }

            const ir::Tuple srcTuple = ctx.arrayTuple(&srcTupleData[0], src_length);

            Constant *msg_type_cpv = dyn_cast<Constant>(*AI);
            assert(msg_type_cpv);
            const ir::Immediate &msg_type_x = processConstantImm(msg_type_cpv);
            int msg_type = msg_type_x.getIntegerValue();
            uint32_t dst_length;
            //msy_type =1 indicate inter search only of gen vme shared function
            GBE_ASSERT(msg_type == 1);
            if(msg_type == 1)
              dst_length = 6;
            for (uint32_t elemID = 0; elemID < dst_length; ++elemID) {
              const ir::Register reg = this->getRegister(&I, elemID);
              dstTupleData.push_back(reg);
            }
            const ir::Tuple dstTuple = ctx.arrayTuple(&dstTupleData[0], dst_length);
            ++AI;
            Constant *vme_search_path_lut_cpv = dyn_cast<Constant>(*AI);
            assert(vme_search_path_lut_cpv);
            const ir::Immediate &vme_search_path_lut_x = processConstantImm(vme_search_path_lut_cpv);
            ++AI;
            Constant *lut_sub_cpv = dyn_cast<Constant>(*AI);
            assert(lut_sub_cpv);
            const ir::Immediate &lut_sub_x = processConstantImm(lut_sub_cpv);

            ctx.VME(imageID, dstTuple, srcTuple, dst_length, src_length,
                    msg_type, vme_search_path_lut_x.getIntegerValue(),
                    lut_sub_x.getIntegerValue());
            break;
          }
          case GEN_OCL_IN_PRIVATE:
          {
            const ir::Register dst = this->getRegister(&I);
            uint32_t stackSize = ctx.getFunction().getStackSize();
            if (stackSize == 0) {
              ir::ImmediateIndex imm = ctx.newImmediate((bool)0);
              ctx.LOADI(ir::TYPE_BOOL, dst, imm);
            } else {
              ir::Register cmp0 = ctx.reg(ir::FAMILY_BOOL);
              ir::Register cmp1 = ctx.reg(ir::FAMILY_BOOL);
              const ir::Register src0 = this->getRegister(*AI);
              ir::Register tmp = ctx.reg(ir::FAMILY_QWORD);

              ctx.GE(ir::TYPE_U64, cmp0, src0, ir::ocl::stackbuffer);
              ctx.ADD(ir::TYPE_U64, tmp, ir::ocl::stackbuffer, ir::ocl::stacksize);
              ctx.LT(ir::TYPE_U64, cmp1, src0, tmp);
              ctx.AND(ir::TYPE_BOOL, dst, cmp0, cmp1);
            }
            break;
          }
          case GEN_OCL_REGION:
          {
            const ir::Register dst = this->getRegister(&I);
            // offset must be immediate
            GBE_ASSERT(AI != AE); Constant *CPV = dyn_cast<Constant>(*AI);
            assert(CPV);
            const ir::Immediate &x = processConstantImm(CPV);

            AI++;
            const ir::Register src = this->getRegister(*AI);

            ctx.REGION(dst, src, x.getIntegerValue());
            break;
          }
          case GEN_OCL_RSQ: this->emitUnaryCallInst(I,CS,ir::OP_RSQ); break;
          case GEN_OCL_RCP: this->emitUnaryCallInst(I,CS,ir::OP_RCP); break;
          case GEN_OCL_FORCE_SIMD8: ctx.setSimdWidth(8); break;
          case GEN_OCL_FORCE_SIMD16: ctx.setSimdWidth(16); break;
          case GEN_OCL_LBARRIER: ctx.SYNC(ir::syncLocalBarrier); break;
          case GEN_OCL_GBARRIER: ctx.SYNC(ir::syncGlobalBarrier); break;
          case GEN_OCL_BARRIER:
          {
            Constant *CPV = dyn_cast<Constant>(*AI);
            unsigned syncFlag = 0;
            if (CPV) {
              const ir::Immediate &x = processConstantImm(CPV);
              unsigned barrierArg = x.getIntegerValue();
              if (barrierArg & 0x1) {
                syncFlag |= ir::syncLocalBarrier;
              }
              if (barrierArg & 0x2) {
                syncFlag |= ir::syncGlobalBarrier;
              }
              if (barrierArg & 0x4) {
                syncFlag |= ir::syncImageBarrier;
              }
            } else {
              // FIXME we default it to do global fence and barrier.
              // we need to do runtime check here.
              syncFlag = ir::syncLocalBarrier | ir::syncGlobalBarrier;
            }

            ctx.SYNC(syncFlag);
            break;
          }
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
            const uint8_t imageID = getImageID(I);
            GBE_ASSERT(AI != AE); ++AI;
            const ir::Register reg = this->getRegister(&I, 0);
            int infoType = genIntrinsicID - GEN_OCL_GET_IMAGE_WIDTH;
            ir::ImageInfoKey key(imageID, infoType);
            const ir::Register infoReg = ctx.getFunction().getImageSet()->appendInfo(key, &ctx);
            ctx.GET_IMAGE_INFO(infoType, reg, imageID, infoReg);
            break;
          }

          case GEN_OCL_READ_IMAGE_I:
          case GEN_OCL_READ_IMAGE_UI:
          case GEN_OCL_READ_IMAGE_F:
          {
            const uint8_t imageID = getImageID(I);
            GBE_ASSERT(AI != AE); ++AI;
            GBE_ASSERT(AI != AE);
            const uint8_t sampler = this->appendSampler(AI);
            ++AI; GBE_ASSERT(AI != AE);
            uint32_t coordNum;
            const ir::Type coordType = getVectorInfo(ctx, *AI, coordNum);
            if (coordNum == 4)
              coordNum = 3;
            const uint32_t imageDim = coordNum;
            GBE_ASSERT(imageDim >= 1 && imageDim <= 3);

            uint8_t samplerOffset = 0;
            Value *coordVal = *AI;
            ++AI; GBE_ASSERT(AI != AE);
            Value *samplerOffsetVal = *AI;
#ifdef GEN7_SAMPLER_CLAMP_BORDER_WORKAROUND
            Constant *CPV = dyn_cast<Constant>(samplerOffsetVal);
            assert(CPV);
            const ir::Immediate &x = processConstantImm(CPV);
            GBE_ASSERTM(x.getType() == ir::TYPE_U32 || x.getType() == ir::TYPE_S32, "Invalid sampler type");
            samplerOffset = x.getIntegerValue();
#endif
            bool isFloatCoord = coordType == ir::TYPE_FLOAT;
            bool requiredFloatCoord = samplerOffset == 0;

            (void) isFloatCoord;
            GBE_ASSERT(isFloatCoord == requiredFloatCoord);

            vector<ir::Register> dstTupleData, srcTupleData;
            for (uint32_t elemID = 0; elemID < imageDim; elemID++)
              srcTupleData.push_back(this->getRegister(coordVal, elemID));

            uint32_t elemNum;
            ir::Type dstType = getVectorInfo(ctx, &I, elemNum);
            GBE_ASSERT(elemNum == 4);

            for (uint32_t elemID = 0; elemID < elemNum; ++elemID) {
              const ir::Register reg = this->getRegister(&I, elemID);
              dstTupleData.push_back(reg);
            }
            const ir::Tuple dstTuple = ctx.arrayTuple(&dstTupleData[0], elemNum);
            const ir::Tuple srcTuple = ctx.arrayTuple(&srcTupleData[0], imageDim);

            ctx.SAMPLE(imageID, dstTuple, srcTuple, imageDim, dstType == ir::TYPE_FLOAT,
                       requiredFloatCoord, sampler, samplerOffset);
            break;
          }

          case GEN_OCL_WRITE_IMAGE_I:
          case GEN_OCL_WRITE_IMAGE_UI:
          case GEN_OCL_WRITE_IMAGE_F:
          {
            const uint8_t imageID = getImageID(I);
            GBE_ASSERT(AI != AE); ++AI; GBE_ASSERT(AI != AE);
            uint32_t coordNum;
            (void)getVectorInfo(ctx, *AI, coordNum);
            if (coordNum == 4)
              coordNum = 3;
            const uint32_t imageDim = coordNum;
            vector<ir::Register> srcTupleData;
            GBE_ASSERT(imageDim >= 1 && imageDim <= 3);

            for (uint32_t elemID = 0; elemID < imageDim; elemID++)
              srcTupleData.push_back(this->getRegister(*AI, elemID));

            ++AI; GBE_ASSERT(AI != AE);
            uint32_t elemNum;
            ir::Type srcType = getVectorInfo(ctx, *AI, elemNum);
            GBE_ASSERT(elemNum == 4);

            for (uint32_t elemID = 0; elemID < elemNum; ++elemID) {
              const ir::Register reg = this->getRegister(*AI, elemID);
              srcTupleData.push_back(reg);
            }
            const ir::Tuple srcTuple = ctx.arrayTuple(&srcTupleData[0], imageDim + 4);
            ctx.TYPED_WRITE(imageID, srcTuple, imageDim + 4, srcType, ir::TYPE_U32);
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
          case GEN_OCL_FMAX:
          case GEN_OCL_FMIN:{
            GBE_ASSERT(AI != AE); const ir::Register src0 = this->getRegister(*AI); ++AI;
            GBE_ASSERT(AI != AE); const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            const ir::Register cmp = ctx.reg(ir::FAMILY_BOOL);
            //Becasue cmp's sources are same as sel's source, so cmp instruction and sel
            //instruction will be merged to one sel_cmp instruction in the gen selection
            //Add two intruction here for simple.
            if(genIntrinsicID == GEN_OCL_FMAX)
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
          case GEN_OCL_SAT_CONV_F16_TO_I8:
            DEF(ir::TYPE_S8, ir::TYPE_HALF);
          case GEN_OCL_SAT_CONV_F16_TO_U8:
            DEF(ir::TYPE_U8, ir::TYPE_HALF);
          case GEN_OCL_SAT_CONV_F16_TO_I16:
            DEF(ir::TYPE_S16, ir::TYPE_HALF);
          case GEN_OCL_SAT_CONV_F16_TO_U16:
            DEF(ir::TYPE_U16, ir::TYPE_HALF);
          case GEN_OCL_SAT_CONV_F16_TO_I32:
            DEF(ir::TYPE_S32, ir::TYPE_HALF);
          case GEN_OCL_SAT_CONV_F16_TO_U32:
            DEF(ir::TYPE_U32, ir::TYPE_HALF);
          case GEN_OCL_CONV_F16_TO_F32:
            ctx.F16TO32(ir::TYPE_FLOAT, ir::TYPE_U16, getRegister(&I), getRegister(I.getOperand(0)));
            break;
          case GEN_OCL_CONV_F32_TO_F16:
            ctx.F32TO16(ir::TYPE_U16, ir::TYPE_FLOAT, getRegister(&I), getRegister(I.getOperand(0)));
            break;
#undef DEF

          case GEN_OCL_PRINTF:
          {
            ir::PrintfSet::PrintfFmt* fmt = getPrintfInfo(&I);
            if (fmt == NULL)
              break;

            ctx.getFunction().getPrintfSet()->append(printfNum, fmt);

            vector<ir::Register> tupleData;
            vector<ir::Type> tupleTypeData;
            int argNum = static_cast<int>(I.getNumOperands());
            argNum -= 2; // no fmt and last NULL.
            int realArgNum = argNum;

            for (int n = 0; n < argNum; n++) {
              /* First, ignore %s, the strings are recorded and not passed to GPU. */
              llvm::Constant* args = dyn_cast<llvm::ConstantExpr>(I.getOperand(n + 1));
              llvm::Constant* args_ptr = NULL;
              if (args)
                args_ptr = dyn_cast<llvm::Constant>(args->getOperand(0));

              if (args_ptr) {
                ConstantDataSequential* fmt_arg = dyn_cast<ConstantDataSequential>(args_ptr->getOperand(0));
                if (fmt_arg && fmt_arg->isCString()) {
                  realArgNum--;
                  continue;
                }
              }

              Type * type = I.getOperand(n + 1)->getType();
              if (type->isVectorTy()) {
                uint32_t srcElemNum = 0;
                Value *srcValue = I.getOperand(n + 1);
                ir::Type srcType = getVectorInfo(ctx, srcValue, srcElemNum);
                GBE_ASSERT(!(srcType == ir::TYPE_DOUBLE));

                uint32_t elemID = 0;
                for (elemID = 0; elemID < srcElemNum; ++elemID) {
                  ir::Register reg = getRegister(srcValue, elemID);
                  tupleData.push_back(reg);
                  tupleTypeData.push_back(srcType);
                }
                realArgNum += srcElemNum - 1;
              } else {
                ir::Register reg = getRegister(I.getOperand(n + 1));
                tupleData.push_back(reg);
                tupleTypeData.push_back(getType(ctx, I.getOperand(n + 1)->getType()));
              }
            }

            ir::Tuple tuple;
            ir::Tuple typeTuple;
            if (realArgNum > 0) {
              tuple = ctx.arrayTuple(&tupleData[0], realArgNum);
              typeTuple = ctx.arrayTypeTuple(&tupleTypeData[0], realArgNum);
            }
            ctx.PRINTF(getRegister(&I), tuple, typeTuple, realArgNum, printfBti, printfNum);
            printfNum++;
            break;
          }
          case GEN_OCL_CALC_TIMESTAMP:
          {
            GBE_ASSERT(AI != AE);
            ConstantInt *CI = dyn_cast<ConstantInt>(*AI);
            GBE_ASSERT(CI);
            uint32_t pointNum = CI->getZExtValue();
            AI++;
            GBE_ASSERT(AI != AE);
            CI = dyn_cast<ConstantInt>(*AI);
            GBE_ASSERT(CI);
            uint32_t tsType = CI->getZExtValue();
            ctx.CALC_TIMESTAMP(pointNum, tsType);
            break;
          }
          case GEN_OCL_STORE_PROFILING:
          {
            /* The profiling log always begin at 0 offset, so we
               never need the buffer ptr value and ptrBase, and
               no need for SUB to calculate the real address, neither.
               We just pass down the BTI value to the instruction. */
            GBE_ASSERT(AI != AE);
            Value* llvmPtr = *AI;
            Value *bti = getBtiRegister(llvmPtr);
            GBE_ASSERT(isa<ConstantInt>(bti)); //Should never be mixed pointer.
            uint32_t index = cast<ConstantInt>(bti)->getZExtValue();
            (void) index;
            GBE_ASSERT(btiToGen(index) == ir::MEM_GLOBAL);
            ++AI;
            GBE_ASSERT(AI != AE);
            ConstantInt *CI = dyn_cast<ConstantInt>(*AI);
            GBE_ASSERT(CI);
            uint32_t ptype = CI->getZExtValue();
            ctx.getUnit().getProfilingInfo()->setProfilingType(ptype);
            break;
          }
          case GEN_OCL_SIMD_SIZE:
          {
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU0(ir::OP_SIMD_SIZE, getType(ctx, I.getType()), dst);
            break;
          }
          case GEN_OCL_SIMD_ID:
          {
            const ir::Register dst = this->getRegister(&I);
            ctx.ALU0(ir::OP_SIMD_ID, getType(ctx, I.getType()), dst);
            break;
          }
          case GEN_OCL_SIMD_SHUFFLE:
          {
            const ir::Register src0 = this->getRegister(*AI); ++AI;
            const ir::Register src1 = this->getRegister(*AI); ++AI;
            const ir::Register dst = this->getRegister(&I);
            ctx.SIMD_SHUFFLE(getType(ctx, I.getType()), dst, src0, src1);
            break;
          }
          case GEN_OCL_DEBUGWAIT:
          {
            ctx.WAIT();
            break;
          }
          case GEN_OCL_WORK_GROUP_ALL: this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_ALL); break;
          case GEN_OCL_WORK_GROUP_ANY: this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_ANY); break;
          case GEN_OCL_WORK_GROUP_BROADCAST:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_BROADCAST); break;
          case GEN_OCL_WORK_GROUP_REDUCE_ADD:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_REDUCE_ADD); break;
          case GEN_OCL_WORK_GROUP_REDUCE_MAX:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_REDUCE_MAX); break;
          case GEN_OCL_WORK_GROUP_REDUCE_MIN:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_REDUCE_MIN); break;
          case GEN_OCL_WORK_GROUP_SCAN_EXCLUSIVE_ADD:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_EXCLUSIVE_ADD); break;
          case GEN_OCL_WORK_GROUP_SCAN_EXCLUSIVE_MAX:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_EXCLUSIVE_MAX); break;
          case GEN_OCL_WORK_GROUP_SCAN_EXCLUSIVE_MIN:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_EXCLUSIVE_MIN); break;
          case GEN_OCL_WORK_GROUP_SCAN_INCLUSIVE_ADD:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_INCLUSIVE_ADD); break;
          case GEN_OCL_WORK_GROUP_SCAN_INCLUSIVE_MAX:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_INCLUSIVE_MAX); break;
          case GEN_OCL_WORK_GROUP_SCAN_INCLUSIVE_MIN:
            this->emitWorkGroupInst(I, CS, ir::WORKGROUP_OP_INCLUSIVE_MIN); break;
          case GEN_OCL_SUB_GROUP_BROADCAST:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_BROADCAST); break;
          case GEN_OCL_SUB_GROUP_REDUCE_ADD:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_REDUCE_ADD); break;
          case GEN_OCL_SUB_GROUP_REDUCE_MAX:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_REDUCE_MAX); break;
          case GEN_OCL_SUB_GROUP_REDUCE_MIN:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_REDUCE_MIN); break;
          case GEN_OCL_SUB_GROUP_SCAN_EXCLUSIVE_ADD:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_EXCLUSIVE_ADD); break;
          case GEN_OCL_SUB_GROUP_SCAN_EXCLUSIVE_MAX:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_EXCLUSIVE_MAX); break;
          case GEN_OCL_SUB_GROUP_SCAN_EXCLUSIVE_MIN:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_EXCLUSIVE_MIN); break;
          case GEN_OCL_SUB_GROUP_SCAN_INCLUSIVE_ADD:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_INCLUSIVE_ADD); break;
          case GEN_OCL_SUB_GROUP_SCAN_INCLUSIVE_MAX:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_INCLUSIVE_MAX); break;
          case GEN_OCL_SUB_GROUP_SCAN_INCLUSIVE_MIN:
            this->emitSubGroupInst(I, CS, ir::WORKGROUP_OP_INCLUSIVE_MIN); break;
          case GEN_OCL_LRP:
          {
            const ir::Register dst  = this->getRegister(&I);
            GBE_ASSERT(AI != AE);
            const ir::Register src0 = this->getRegister(*(AI++));
            GBE_ASSERT(AI != AE);
            const ir::Register src1 = this->getRegister(*(AI++));
            GBE_ASSERT(AI != AE);
            const ir::Register src2 = this->getRegister(*(AI++));
            ctx.LRP(ir::TYPE_FLOAT, dst, src0, src1, src2);
            break;
          }
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM:
            this->emitBlockReadWriteMemInst(I, CS, false, 1); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM2:
            this->emitBlockReadWriteMemInst(I, CS, false, 2); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM4:
            this->emitBlockReadWriteMemInst(I, CS, false, 4); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM8:
            this->emitBlockReadWriteMemInst(I, CS, false, 8); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM:
            this->emitBlockReadWriteMemInst(I, CS, true, 1); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM2:
            this->emitBlockReadWriteMemInst(I, CS, true, 2); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM4:
            this->emitBlockReadWriteMemInst(I, CS, true, 4); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM8:
            this->emitBlockReadWriteMemInst(I, CS, true, 8); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE:
            this->emitBlockReadWriteImageInst(I, CS, false, 1); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE2:
            this->emitBlockReadWriteImageInst(I, CS, false, 2); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE4:
            this->emitBlockReadWriteImageInst(I, CS, false, 4); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE8:
            this->emitBlockReadWriteImageInst(I, CS, false, 8); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE:
            this->emitBlockReadWriteImageInst(I, CS, true, 1); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE2:
            this->emitBlockReadWriteImageInst(I, CS, true, 2); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE4:
            this->emitBlockReadWriteImageInst(I, CS, true, 4); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE8:
            this->emitBlockReadWriteImageInst(I, CS, true, 8); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM:
            this->emitBlockReadWriteMemInst(I, CS, false, 1, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM2:
            this->emitBlockReadWriteMemInst(I, CS, false, 2, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM4:
            this->emitBlockReadWriteMemInst(I, CS, false, 4, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM8:
            this->emitBlockReadWriteMemInst(I, CS, false, 8, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM:
            this->emitBlockReadWriteMemInst(I, CS, true, 1, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM2:
            this->emitBlockReadWriteMemInst(I, CS, true, 2, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM4:
            this->emitBlockReadWriteMemInst(I, CS, true, 4, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM8:
            this->emitBlockReadWriteMemInst(I, CS, true, 8, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE:
            this->emitBlockReadWriteImageInst(I, CS, false, 1, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE2:
            this->emitBlockReadWriteImageInst(I, CS, false, 2, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE4:
            this->emitBlockReadWriteImageInst(I, CS, false, 4, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE8:
            this->emitBlockReadWriteImageInst(I, CS, false, 8, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE:
            this->emitBlockReadWriteImageInst(I, CS, true, 1, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE2:
            this->emitBlockReadWriteImageInst(I, CS, true, 2, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE4:
            this->emitBlockReadWriteImageInst(I, CS, true, 4, ir::TYPE_U16); break;
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE8:
            this->emitBlockReadWriteImageInst(I, CS, true, 8, ir::TYPE_U16); break;
          case GEN_OCL_GET_PIPE:
          case GEN_OCL_MAKE_RID:
          case GEN_OCL_GET_RID:
          {
            break;
          }
          case GEN_OCL_ENQUEUE_SET_NDRANGE_INFO:
          {
            GBE_ASSERT(AI != AE);
            Value *srcValue = *AI;
            ++AI;
            Value *dstValue = &I;
            regTranslator.newValueProxy(srcValue, dstValue);
            break;
          }
          case GEN_OCL_ENQUEUE_GET_NDRANGE_INFO:
          {
            GBE_ASSERT(AI != AE);
            Value *srcValue = *AI;
            ++AI;
            Value *dstValue = &I;
            regTranslator.newValueProxy(srcValue, dstValue);
            break;
          }
          case GEN_OCL_ENQUEUE_GET_ENQUEUE_INFO_ADDR:
          {
            ctx.getFunction().setUseDeviceEnqueue(true);
            break;
          }
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
        ir::ImmediateIndex stepImm;
        ir::Type pointerTy = getType(pointerFamily);
        if (ctx.getPointerSize() == ir::POINTER_32_BITS)
          stepImm = ctx.newImmediate(uint32_t(step));
        else
          stepImm = ctx.newImmediate(uint64_t(step));
        ir::Register stepReg = ctx.reg(ctx.getPointerFamily());
        ctx.LOADI(pointerTy, stepReg, stepImm);
        ctx.ADD(pointerTy, stack, stack, stepReg);
        ctx.getFunction().pushStackSize(step);
      }
    }
    // Set the destination register properly
    if (legacyMode)
      ctx.MOV(imm.getType(), dst, stack);
    else
      ctx.ADD(imm.getType(), dst, stack, ir::ocl::stackbuffer);

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
  void GenWriter::emitLoadInst(LoadInst &I) {
    MemoryInstHelper *h = new MemoryInstHelper(ctx, unit, this, legacyMode);
    h->emitLoadOrStore<true>(I);
    delete h;
  }

  void GenWriter::emitStoreInst(StoreInst &I) {
    MemoryInstHelper *h = new MemoryInstHelper(ctx, unit, this, legacyMode);
    h->emitLoadOrStore<false>(I);
    delete h;
  }

  llvm::FunctionPass *createGenPass(ir::Unit &unit) {
    return new GenWriter(unit);
  }

  ir::Tuple MemoryInstHelper::getValueTuple(llvm::Value *llvmValues, llvm::Type *elemType, unsigned start, unsigned elemNum) {
      vector<ir::Register> tupleData; // put registers here
      for (uint32_t elemID = 0; elemID < elemNum; ++elemID) {
        ir::Register reg;
        if(writer->regTranslator.isUndefConst(llvmValues, elemID)) {
          Value *v = Constant::getNullValue(elemType);
          reg = writer->getRegister(v);
        } else
          reg = writer->getRegister(llvmValues, start + elemID);

        tupleData.push_back(reg);
      }
      const ir::Tuple tuple = ctx.arrayTuple(&tupleData[0], elemNum);
      return tuple;
  }

  void MemoryInstHelper::emitBatchLoadOrStore(const ir::Type type, const uint32_t elemNum,
                                      Value *llvmValues,
                                      Type * elemType) {
    uint32_t totalSize = elemNum * getFamilySize(getFamily(type));
    uint32_t msgNum = totalSize > 16 ? totalSize / 16 : 1;
    const uint32_t perMsgNum = elemNum / msgNum;

    for (uint32_t msg = 0; msg < msgNum; ++msg) {
      // Build the tuple data in the vector
     ir::Tuple tuple = getValueTuple(llvmValues, elemType, perMsgNum*msg, perMsgNum);
        // each message can read/write 16 byte
        const int32_t stride = 16;
      ir::Register addr = getOffsetAddress(mPtr, msg*stride);
      shootMessage(type, addr, tuple, perMsgNum);
    }
  }

  ir::Register MemoryInstHelper::getOffsetAddress(ir::Register basePtr, unsigned offset) {
    const ir::RegisterFamily pointerFamily = ctx.getPointerFamily();
    ir::Register addr;
    if (offset == 0)
      addr = basePtr;
    else {
      const ir::Register offsetReg = ctx.reg(pointerFamily);
      ir::ImmediateIndex immIndex;
      ir::Type immType;

      if (pointerFamily == ir::FAMILY_DWORD) {
        immIndex = ctx.newImmediate(int32_t(offset));
        immType = ir::TYPE_S32;
      } else {
        immIndex = ctx.newImmediate(int64_t(offset));
        immType = ir::TYPE_S64;
      }

      addr = ctx.reg(pointerFamily);
      ctx.LOADI(immType, offsetReg, immIndex);
      ctx.ADD(immType, addr, basePtr, offsetReg);
    }
    return addr;
  }

  // handle load of dword/qword with unaligned address
  void MemoryInstHelper::emitUnalignedDQLoadStore(Value *llvmValues)
  {
    Type *llvmType = llvmValues->getType();
    unsigned byteSize = getTypeByteSize(unit, llvmType);

    Type *elemType = llvmType;
    unsigned elemNum = 1;
    if (!isScalarType(llvmType)) {
      VectorType *vectorType = cast<VectorType>(llvmType);
      elemType = vectorType->getElementType();
      elemNum = vectorType->getNumElements();
    }
    const ir::Type type = getType(ctx, elemType);

    ir::Tuple tuple = getValueTuple(llvmValues, elemType, 0, elemNum);
    vector<ir::Register> byteTupleData;
    for (uint32_t elemID = 0; elemID < byteSize; ++elemID) {
      byteTupleData.push_back(ctx.reg(ir::FAMILY_BYTE));
    }
    const ir::Tuple byteTuple = ctx.arrayTuple(&byteTupleData[0], byteSize);

    if (isLoad) {
      shootMessage(ir::TYPE_U8, mPtr, byteTuple, byteSize);
      ctx.BITCAST(type, ir::TYPE_U8, tuple, byteTuple, elemNum, byteSize);
    } else {
      ctx.BITCAST(ir::TYPE_U8, type, byteTuple, tuple, byteSize, elemNum);
      // FIXME: byte scatter does not handle correctly vector store, after fix that,
      //        we can directly use on store instruction like:
      //        ctx.STORE(ir::TYPE_U8, byteTuple, ptr, addrSpace, byteSize, dwAligned, fixedBTI, bti);
      for (uint32_t elemID = 0; elemID < byteSize; elemID++) {
        const ir::Register addr = getOffsetAddress(mPtr, elemID);
        const ir::Tuple value = ctx.arrayTuple(&byteTupleData[elemID], 1);
        shootMessage(ir::TYPE_U8, addr, value, 1);
      }
    }
  }

  template <bool IsLoad, typename T>
  void MemoryInstHelper::emitLoadOrStore(T &I) {
    Value *llvmPtr = I.getPointerOperand();
    Value *llvmValues = getLoadOrStoreValue(I);
    Type *llvmType = llvmValues->getType();
    dwAligned = (I.getAlignment() % 4) == 0;
    addrSpace = addressSpaceLLVMToGen(llvmPtr->getType()->getPointerAddressSpace());
    const ir::Register pointer = writer->getRegister(llvmPtr);
    const ir::RegisterFamily pointerFamily = ctx.getPointerFamily();

    this->isLoad = IsLoad;
    Type *scalarType = llvmType;
    if (!isScalarType(llvmType)) {
      VectorType *vectorType = cast<VectorType>(llvmType);
      scalarType = vectorType->getElementType();
    }

    // calculate bti and pointer operand
    if (legacyMode) {
      Value *bti = writer->getBtiRegister(llvmPtr);
      Value *ptrBase = writer->getPointerBase(llvmPtr);
      ir::Register baseReg = writer->getRegister(ptrBase);
      bool zeroBase = isa<ConstantPointerNull>(ptrBase) ? true : false;

      if (isa<ConstantInt>(bti)) {
        SurfaceIndex = cast<ConstantInt>(bti)->getZExtValue();
        addrSpace = btiToGen(SurfaceIndex);
        mAddressMode = ir::AM_StaticBti;
      } else {
        addrSpace = ir::MEM_MIXED;
        mBTI = writer->getRegister(bti);
        mAddressMode = ir::AM_DynamicBti;
      }
      mPtr = ctx.reg(pointerFamily);

      // FIXME: avoid subtraction zero at this stage is not a good idea,
      // but later ArgumentLower pass need to match exact load/addImm pattern
      // so, I avoid subtracting zero base to satisfy ArgumentLower pass.
      if (!zeroBase)
        ctx.SUB(getType(ctx, llvmPtr->getType()), mPtr, pointer, baseReg);
      else
        mPtr = pointer;
    } else {
      mPtr = pointer;
      SurfaceIndex = 0xff;
      mAddressMode = ir::AM_Stateless;
    }

    unsigned primitiveBits = scalarType->getPrimitiveSizeInBits();
    if (!dwAligned
       && (primitiveBits == 64
          || primitiveBits == 32)
       ) {
      emitUnalignedDQLoadStore(llvmValues);
      return;
    }
    // Scalar is easy. We neednot build register tuples
    if (isScalarType(llvmType) == true) {
      const ir::Type type = getType(ctx, llvmType);
      const ir::Register values = writer->getRegister(llvmValues);
      const ir::Tuple tuple = ctx.arrayTuple(&values, 1);
      shootMessage(type, mPtr, tuple, 1);
    }
    // A vector type requires to build a tuple
    else {
      VectorType *vectorType = cast<VectorType>(llvmType);
      Type *elemType = vectorType->getElementType();

      // We follow OCL spec and support 2,3,4,8,16 elements only
      uint32_t elemNum = vectorType->getNumElements();
      GBE_ASSERTM(elemNum == 2 || elemNum == 3 || elemNum == 4 || elemNum == 8 || elemNum == 16,
                  "Only vectors of 2,3,4,8 or 16 elements are supported");

      // The code is going to be fairly different from types to types (based on
      // size of each vector element)
      const ir::Type type = getType(ctx, elemType);
      const ir::RegisterFamily dataFamily = getFamily(type);

      if(dataFamily == ir::FAMILY_DWORD && addrSpace != ir::MEM_CONSTANT) {
        // One message is enough here. Nothing special to do
        if (elemNum <= 4) {
          ir::Tuple tuple = getValueTuple(llvmValues, elemType, 0, elemNum);
          shootMessage(type, mPtr, tuple, elemNum);
        }
        else {
          emitBatchLoadOrStore(type, elemNum, llvmValues, elemType);
        }
      }
      else if((dataFamily == ir::FAMILY_WORD && (isLoad || elemNum % 2 == 0)) ||
              (dataFamily == ir::FAMILY_BYTE && (isLoad || elemNum % 4 == 0))) {
          emitBatchLoadOrStore(type, elemNum, llvmValues, elemType);
      } else {
        for (uint32_t elemID = 0; elemID < elemNum; elemID++) {
          if(writer->regTranslator.isUndefConst(llvmValues, elemID))
            continue;

          const ir::Register reg = writer->getRegister(llvmValues, elemID);
          int elemSize = getTypeByteSize(unit, elemType);

          ir::Register addr = getOffsetAddress(mPtr, elemID*elemSize);
          const ir::Tuple tuple = ctx.arrayTuple(&reg, 1);
          shootMessage(type, addr, tuple, 1);
        }
      }
    }
  }

  void MemoryInstHelper::shootMessage(ir::Type type, ir::Register offset, ir::Tuple value, unsigned elemNum) {
    if (mAddressMode == ir::AM_DynamicBti) {
      if (isLoad)
        ctx.LOAD(type, value, offset, addrSpace, elemNum, dwAligned, mAddressMode, mBTI);
      else
        ctx.STORE(type, value, offset, addrSpace, elemNum, dwAligned, mAddressMode, mBTI);
    } else {
      if (isLoad)
        ctx.LOAD(type, value, offset, addrSpace, elemNum, dwAligned, mAddressMode, SurfaceIndex);
      else
        ctx.STORE(type, value, offset, addrSpace, elemNum, dwAligned, mAddressMode, SurfaceIndex);
    }
  }
} /* namespace gbe */

