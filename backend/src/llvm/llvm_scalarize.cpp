/**
 * \file llvm_scalarize.cpp
 *
 * This file is derived from:
 *  https://code.google.com/p/lunarglass/source/browse/trunk/Core/Passes/Transforms/Scalarize.cpp?r=903
 */

//===- Scalarize.cpp - Scalarize LunarGLASS IR ----------------------------===//
//
// LunarGLASS: An Open Modular Shader Compiler Architecture
// Copyright (C) 2010-2014 LunarG, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//     Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//     Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//
//     Neither the name of LunarG Inc. nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//===----------------------------------------------------------------------===//
//
// Author: Michael Ilseman, LunarG
//
//===----------------------------------------------------------------------===//
//
// Scalarize the IR.
//   * Loads of uniforms become multiple loadComponent calls
//
//   * Reads/writes become read/writeComponent calls
//
//   * Component-wise operations become multiple ops over each component
//
//   * Texture call become recomponsed texture calls
//
//   * Vector ops disappear, with their users referring to the scalarized
//   * components
//
//===----------------------------------------------------------------------===//

#include "llvm_includes.hpp"

#include "llvm/llvm_gen_backend.hpp"
#include "sys/map.hpp"

using namespace llvm;

namespace gbe {

  struct VectorValues {
    VectorValues() : vals()
    { }

    void setComponent(int c, llvm::Value* val)
    {
      assert(c >= 0 && c < 32 && "Out of bounds component");
      vals[c] = val;
    }
    llvm::Value* getComponent(int c)
    {
      assert(c >= 0 && c < 32 && "Out of bounds component");
      assert(vals[c] && "Requesting non-existing component");
      return vals[c];
    }

    // {Value* x, Value* y, Value* z, Value* w}
    llvm::Value* vals[32];
  };

  class Scalarize : public FunctionPass {

  public:
    // Standard pass stuff
    static char ID;

    Scalarize() : FunctionPass(ID)
    {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 35
      initializeDominatorTreeWrapperPassPass(*PassRegistry::getPassRegistry());
#else
      initializeDominatorTreePass(*PassRegistry::getPassRegistry());
#endif
    }

    virtual bool runOnFunction(Function&);
    void print(raw_ostream&, const Module* = 0) const;
    virtual void getAnalysisUsage(AnalysisUsage&) const;

  protected:
    // An instruction is valid post-scalarization iff it is fully scalar or it
    // is a gla_loadn
    bool isValid(const Instruction*);

    // Take an instruction that produces a vector, and scalarize it
    bool scalarize(Instruction*);
    bool scalarizePerComponent(Instruction*);
    bool scalarizeBitCast(BitCastInst *);
    bool scalarizeFuncCall(CallInst *);
    bool scalarizeLoad(LoadInst*);
    bool scalarizeStore(StoreInst*);
    //bool scalarizeIntrinsic(IntrinsicInst*);
    bool scalarizeExtract(ExtractElementInst*);
    bool scalarizeInsert(InsertElementInst*);
    bool scalarizeShuffleVector(ShuffleVectorInst*);
    bool scalarizePHI(PHINode*);
    void scalarizeArgs(Function& F);
    // ...

    // Helpers to make the actual multiple scalar calls, one per
    // component. Updates the given VectorValues's components with the new
    // Values.
    void makeScalarizedCalls(Function*, ArrayRef<Value*>, int numComponents, VectorValues&);

    void makePerComponentScalarizedCalls(Instruction*, ArrayRef<Value*>);

    // Makes a scalar form of the given instruction: replaces the operands
    // and chooses a correct return type
    Instruction* createScalarInstruction(Instruction* inst, ArrayRef<Value*>);

    // Gather the specified components in the given values. Returns the
    // component if the given value is a vector, or the scalar itself.
    void gatherComponents(int component, ArrayRef<Value*> args, SmallVectorImpl<Value*>& componentArgs);

    // Get the assigned component for that value. If the value is a scalar,
    // returns the scalar. If it's a constant, returns that component. If
    // it's an instruction, returns the vectorValues of that instruction for
    // that component
    Value* getComponent(int component, Value*);

    // Used for assertion purposes. Whether we can get the component out with
    // a getComponent call
    bool canGetComponent(Value*);

    // Used for assertion purposes. Whether for every operand we can get
    // components with a getComponent call
    bool canGetComponentArgs(User*);

    // Delete the instruction in the deadList
    void dce();


    int GetConstantInt(const Value* value);
    bool IsPerComponentOp(const Instruction* inst);
    bool IsPerComponentOp(const Value* value);

    //these function used to add extract and insert instructions when load/store etc.
    void extractFromVector(Value* insn);
    Value* InsertToVector(Value* insn, Value* vecValue);

    Type* GetBasicType(Value* value) {
      return GetBasicType(value->getType());
    }

    Type* GetBasicType(Type* type) {
      if(!type)
        return type;
      switch(type->getTypeID()) {
      case Type::VectorTyID:
      case Type::ArrayTyID:
        return GetBasicType(type->getContainedType(0));
      default:
        break;
      }
      return type;
    }

    int GetComponentCount(const Type* type)  {
      if (type && type->getTypeID() == Type::VectorTyID)
        return llvm::dyn_cast<VectorType>(type)->getNumElements();
      else
        return 1;
    }

    int GetComponentCount(const Value* value) {
      return GetComponentCount(value ? value->getType() : NULL);
    }

    /* set to insert new instructions after the specified instruction.*/
    void setAppendPoint(Instruction *insn)  {
      BasicBlock::iterator next(insn);
      builder->SetInsertPoint(&*++next);
    }

    DenseMap<Value*, VectorValues> vectorVals;
    struct VecValElement{
      VecValElement(VectorValues *v, uint32_t i) : vecVals(v), id(i) {}
      VectorValues *vecVals;
      uint32_t id;
    };
    DenseMap<Value*, SmallVector<VecValElement, 16>> usedVecVals;

    void setComponent(VectorValues &vecVals, uint32_t c, llvm::Value* val) {
      vecVals.setComponent(c, val);
      usedVecVals[val].push_back(VecValElement(&vecVals, c));
    }

    void replaceAllUsesOfWith(Instruction* from, Instruction* to);

    Module* module;
    IRBuilder<>* builder;

    Type* intTy;
    Type* floatTy;

    std::vector<Instruction*> deadList;

    // List of vector phis that were not completely scalarized because some
    // of their operands hadn't before been visited (i.e. loop variant
    // variables)
    SmallVector<PHINode*, 16> incompletePhis;

    // Map for alloca vec uesd for Extractelememt < vec, alloca >
    std::map<Value*, Value*> vectorAlloca;
  };

  Value* Scalarize::getComponent(int component, Value* v)
  {
    assert(canGetComponent(v) && "getComponent called on unhandled vector");

    if (v && v->getType() && v->getType()->isVectorTy()) {
      if (ConstantDataVector* c = dyn_cast<ConstantDataVector>(v)) {
        return c->getElementAsConstant(component);
      } else if (ConstantVector* c = dyn_cast<ConstantVector>(v)) {
        return c->getOperand(component);
      } else if (isa<ConstantAggregateZero>(v)) {
        return Constant::getNullValue(GetBasicType(v));
      } else if (isa<UndefValue>(v)) {
        return UndefValue::get(GetBasicType(v));
      } else {
        return vectorVals[v].getComponent(component);
      }
    } else {
      return v;
    }
  }

  bool IsPerComponentOp(const llvm::Value* value)
  {
    const llvm::Instruction* inst = llvm::dyn_cast<const llvm::Instruction>(value);
    return inst && IsPerComponentOp(inst);
  }

  bool Scalarize::IsPerComponentOp(const Instruction* inst)
  {
    if (const IntrinsicInst* intr = dyn_cast<const IntrinsicInst>(inst))
    {
        const Intrinsic::ID intrinsicID = (Intrinsic::ID) intr->getIntrinsicID();
        switch (intrinsicID) {
          default: return false;
          case Intrinsic::sqrt:
          case Intrinsic::ceil:
          case Intrinsic::trunc:
          case Intrinsic::fmuladd:
              return true;
        }
    }

    if (inst->isTerminator())
        return false;

    switch (inst->getOpcode()) {

    // Cast ops are only per-component if they cast back to the same vector
    // width
    case Instruction::Trunc:
    case Instruction::ZExt:
    case Instruction::SExt:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::UIToFP:
    case Instruction::SIToFP:
    case Instruction::FPTrunc:
    case Instruction::FPExt:
    case Instruction::PtrToInt:
    case Instruction::IntToPtr:
    case Instruction::BitCast:
      return GetComponentCount(inst->getOperand(0)) == GetComponentCount(inst);

    // Vector ops
    case Instruction::InsertElement:
    case Instruction::ExtractElement:
    case Instruction::ShuffleVector:

    // Ways of accessing/loading/storing vectors
    case Instruction::ExtractValue:
    case Instruction::InsertValue:

    // Memory ops
    case Instruction::Alloca:
    case Instruction::Load:
    case Instruction::Store:
    case Instruction::GetElementPtr:
    // Phis are a little special. We consider them not to be per-component
    // because the mechanism of choice is a single value (what path we took to
    // get here), and doesn't choose per-component (as select would). The caller
    // should know to handle phis specially
    case Instruction::PHI:
    // Call insts, conservatively are no per-component
    case Instruction::Call:
    // Misc
    case Instruction::LandingPad:  //--- 3.0
    case Instruction::VAArg:
      return false;
    } // end of switch (inst->getOpcode())

    return true;
  }
  int Scalarize::GetConstantInt(const Value* value)
  {
    const ConstantInt *constantInt = dyn_cast<ConstantInt>(value);

    // this might still be a constant expression, rather than a numeric constant,
    // e.g., expression with undef's in it, so it was not folded
    if (! constantInt)
      NOT_IMPLEMENTED; //gla::UnsupportedFunctionality("non-simple constant");

    return constantInt->getValue().getSExtValue();
  }
  bool Scalarize::canGetComponent(Value* v)
  {
    if (v && v->getType() && v->getType()->isVectorTy()) {
      if (isa<ConstantDataVector>(v) || isa<ConstantVector>(v) || isa<ConstantAggregateZero>(v) || isa<UndefValue>(v)) {
        return true;
      } else {
        assert((isa<Instruction>(v) || isa<Argument>(v)) && "Non-constant non-instuction?");
        return vectorVals.count(v);
      }
    } else {
      return true;
    }
  }

  bool Scalarize::canGetComponentArgs(User* u)
  {
    if (PHINode* phi = dyn_cast<PHINode>(u)) {
      for (unsigned int i = 0; i < phi->getNumIncomingValues(); ++i)
        if (!canGetComponent(phi->getIncomingValue(i)))
          return false;
    } else {
      for (User::op_iterator i = u->op_begin(), e = u->op_end(); i != e; ++i)
        if (!canGetComponent(*i))
          return false;
    }
    return true;
  }

  void Scalarize::gatherComponents(int component, ArrayRef<Value*> args, SmallVectorImpl<Value*>& componentArgs)
  {
    componentArgs.clear();
    for (ArrayRef<Value*>::iterator i = args.begin(), e = args.end(); i != e; ++i)
      componentArgs.push_back(getComponent(component, *i));
  }

  Instruction* Scalarize::createScalarInstruction(Instruction* inst, ArrayRef<Value*> args)
  {
    // TODO: Refine the below into one large switch

    unsigned op = inst->getOpcode();
    if (inst->isCast()) {
      assert(args.size() == 1 && "incorrect number of arguments for cast op");
      return CastInst::Create((Instruction::CastOps)op, args[0], GetBasicType(inst));
    }

    if (inst->isBinaryOp()) {
      assert(args.size() == 2 && "incorrect number of arguments for binary op");
      return BinaryOperator::Create((Instruction::BinaryOps)op, args[0], args[1]);
    }

    if (PHINode* phi = dyn_cast<PHINode>(inst)) {
      PHINode* res = PHINode::Create(GetBasicType(inst), phi->getNumIncomingValues());

      // Loop over pairs of operands: [Value*, BasicBlock*]
      for (unsigned int i = 0; i < args.size(); i++) {
        BasicBlock* bb = phi->getIncomingBlock(i); //dyn_cast<BasicBlock>(args[i+1]);
        //assert(bb && "Non-basic block incoming block?");
        res->addIncoming(args[i], bb);
      }

      return res;
    }

    if (CmpInst* cmpInst = dyn_cast<CmpInst>(inst)) {
      assert(args.size() == 2 && "incorrect number of arguments for comparison");
      return CmpInst::Create(cmpInst->getOpcode(), cmpInst->getPredicate(), args[0], args[1]);
    }

    if (isa<SelectInst>(inst)) {
      assert(args.size() == 3 && "incorrect number of arguments for select");
      return SelectInst::Create(args[0], args[1], args[2]);
    }

    if (IntrinsicInst* intr = dyn_cast<IntrinsicInst>(inst)) {
      if (! IsPerComponentOp(inst))
        NOT_IMPLEMENTED; //gla::UnsupportedFunctionality("Scalarize instruction on a non-per-component intrinsic");

      // TODO: Assumption is that all per-component intrinsics have all their
      // arguments be overloadable. Need to find some way to assert on this
      // assumption. This is due to how getDeclaration operates; it only takes
      // a list of types that fit overloadable slots.
      SmallVector<Type*, 8> tys(1, GetBasicType(inst->getType()));

      // Call instructions have the decl as a last argument, so skip it
      SmallVector<Value*, 8> _args;

      for (ArrayRef<Value*>::iterator i = args.begin(), e = args.end() - 1; i != e; ++i) {
        tys.push_back(GetBasicType((*i)->getType()));
        _args.push_back(*i);
      }

      Function* f = Intrinsic::getDeclaration(module, intr->getIntrinsicID(), tys);
      return CallInst::Create(f, _args);
    }

    NOT_IMPLEMENTED; //gla::UnsupportedFunctionality("Currently unsupported instruction: ", inst->getOpcode(),
                     //             inst->getOpcodeName());
    return 0;

  }


  void Scalarize::makeScalarizedCalls(Function* f, ArrayRef<Value*> args, int count, VectorValues& vVals)
  {
    assert(count > 0 && count <= 32 && "invalid number of vector components");
    for (int i = 0; i < count; ++i) {
      Value* res;
      SmallVector<Value*, 8> callArgs(args.begin(), args.end());
      callArgs.push_back(ConstantInt::get(intTy, i));

      res = builder->CreateCall(f, callArgs);
      setComponent(vVals, i, res);
    }
  }

  void Scalarize::makePerComponentScalarizedCalls(Instruction* inst, ArrayRef<Value*> args)
  {
    int count = GetComponentCount(inst);
    assert(count > 0 && count <= 32 && "invalid number of vector components");
    assert((inst->getNumOperands() == args.size() || isa<PHINode>(inst))
           && "not enough arguments passed for instruction");

    VectorValues& vVals = vectorVals[inst];

    for (int i = 0; i < count; ++i) {
      // Set this component of each arg
      SmallVector<Value*, 8> callArgs(args.size(), 0);
      gatherComponents(i, args, callArgs);

      Instruction* res = createScalarInstruction(inst, callArgs);

      setComponent(vVals, i, res);
      builder->Insert(res);
    }
  }

  bool Scalarize::isValid(const Instruction* inst)
  {
    // The result
    if (inst->getType()->isVectorTy())
        return false;

    // The arguments
    for (Instruction::const_op_iterator i = inst->op_begin(), e = inst->op_end(); i != e; ++i) {
      const Value* v = (*i);
      assert(v);
      if (v->getType()->isVectorTy())
        return false;
    }

    return true;
  }

  bool Scalarize::scalarize(Instruction* inst)
  {
    if (isValid(inst))
        return false;

    assert(! vectorVals.count(inst) && "We've already scalarized this somehow?");
    assert((canGetComponentArgs(inst) || isa<PHINode>(inst)) &&
           "Scalarizing an op whose arguments haven't been scalarized ");
    builder->SetInsertPoint(inst);

    if (IsPerComponentOp(inst))
      return scalarizePerComponent(inst);

    //not Per Component bitcast, for example <2 * i8> -> i16, handle it in backend
    if (BitCastInst* bt = dyn_cast<BitCastInst>(inst))
      return scalarizeBitCast(bt);

    if (LoadInst* ld = dyn_cast<LoadInst>(inst))
      return scalarizeLoad(ld);

    if (CallInst* call = dyn_cast<CallInst>(inst))
      return scalarizeFuncCall(call);

    if (ExtractElementInst* extr = dyn_cast<ExtractElementInst>(inst))
      return scalarizeExtract(extr);

    if (InsertElementInst* ins = dyn_cast<InsertElementInst>(inst))
      return scalarizeInsert(ins);

    if (ShuffleVectorInst* sv = dyn_cast<ShuffleVectorInst>(inst))
      return scalarizeShuffleVector(sv);

    if (PHINode* phi = dyn_cast<PHINode>(inst))
      return scalarizePHI(phi);

    if (isa<ExtractValueInst>(inst) || isa<InsertValueInst>(inst))
      // TODO: need to come up with a struct/array model for scalarization
      NOT_IMPLEMENTED; //gla::UnsupportedFunctionality("Scalarizing struct/array ops");

    if (StoreInst* st = dyn_cast<StoreInst>(inst))
      return scalarizeStore(st);

    NOT_IMPLEMENTED; //gla::UnsupportedFunctionality("Currently unhandled instruction ", inst->getOpcode(), inst->getOpcodeName());
    return false;
  }

  bool Scalarize::scalarizeShuffleVector(ShuffleVectorInst* sv)
  {
    //     %res = shuffleVector <n x ty> %foo, <n x ty> bar, <n x i32> <...>
    // ==> nothing (just make a new VectorValues with the new components)
    VectorValues& vVals = vectorVals[sv];

    int size = GetComponentCount(sv);

    Value* Op0 = sv->getOperand(0);
    if(!Op0)
      return false;

    int srcSize = GetComponentCount(Op0->getType());

    for (int i = 0; i < size; ++i) {
      int select = sv->getMaskValue(i);

      if (select < 0) {
        setComponent(vVals, i, UndefValue::get(GetBasicType(Op0)));
        continue;
      }

      // Otherwise look up the corresponding component from the correct
      // source.
      Value* selectee;
      if (select < srcSize) {
        selectee = sv->getOperand(0);
      } else {
        // Choose from the second operand
        select -= srcSize;
        selectee = sv->getOperand(1);
      }

      setComponent(vVals, i, getComponent(select, selectee));
    }

    return true;
  }

  bool Scalarize::scalarizePerComponent(Instruction* inst)
  {
    //     dst  = op <n x ty> %foo, <n x ty> %bar
    // ==> dstx = op ty %foox, ty %barx
    //     dsty = op ty %fooy, ty %bary
    //     ...

    SmallVector<Value*, 16> args(inst->op_begin(), inst->op_end());

    makePerComponentScalarizedCalls(inst, args);

    return true;
  }

  bool Scalarize::scalarizePHI(PHINode* phi)
  {
    //     dst = phi <n x ty> [ %foo, %bb1 ], [ %bar, %bb2], ...
    // ==> dstx = phi ty [ %foox, %bb1 ], [ %barx, %bb2], ...
    //     dsty = phi ty [ %fooy, %bb1 ], [ %bary, %bb2], ...

    // If the scalar values are all known up-front, then just make the full
    // phinode now. If they are not yet known (phinode for a loop variant
    // variable), then deferr the arguments until later

    if (canGetComponentArgs(phi)) {
      SmallVector<Value*, 8> args(phi->op_begin(), phi->op_end());
      makePerComponentScalarizedCalls(phi, args);
    } else {
      makePerComponentScalarizedCalls(phi, ArrayRef<Value*>());
      incompletePhis.push_back(phi);
    }

    return true;
  }

  void Scalarize::extractFromVector(Value* insn) {
    VectorValues& vVals = vectorVals[insn];

    for (int i = 0; i < GetComponentCount(insn); ++i) {
      Value *cv = ConstantInt::get(intTy, i);
      Value *EI = builder->CreateExtractElement(insn, cv);
      setComponent(vVals, i, EI);
    }
  }

  Value* Scalarize::InsertToVector(Value * insn, Value* vecValue) {
    //VectorValues& vVals = vectorVals[writeValue];

    //add fake insert instructions to avoid removed
    Value *II = NULL;
    for (int i = 0; i < GetComponentCount(vecValue); ++i) {
      Value *vec = II ? II : UndefValue::get(vecValue->getType());
      Value *cv = ConstantInt::get(intTy, i);
      II = builder->CreateInsertElement(vec, getComponent(i, vecValue), cv);
    }

    return II;
  }

  bool Scalarize::scalarizeFuncCall(CallInst* call) {
    if (Function *F = call->getCalledFunction()) {
      if (F->getIntrinsicID() != 0) {   //Intrinsic functions
        const Intrinsic::ID intrinsicID = (Intrinsic::ID) F->getIntrinsicID();

        switch (intrinsicID) {
          default: GBE_ASSERTM(false, "Unsupported Intrinsic");
          case Intrinsic::sqrt:
          case Intrinsic::ceil:
          case Intrinsic::trunc:
          {
            scalarizePerComponent(call);
          }
          break;
        }
      } else {
        Value *Callee = call->getCalledValue();
        const std::string fnName = Callee->getName();
        auto genIntrinsicID = intrinsicMap.find(fnName);

        // Get the function arguments
        CallSite CS(call);
        CallSite::arg_iterator CI = CS.arg_begin() + 1;

        switch (genIntrinsicID) {
          case GEN_OCL_NOT_FOUND:
          default: break;
          case GEN_OCL_READ_IMAGE_I:
          case GEN_OCL_READ_IMAGE_UI:
          case GEN_OCL_READ_IMAGE_F:
          {
            ++CI;
            if ((*CI)->getType()->isVectorTy()) 
              *CI = InsertToVector(call, *CI);
            setAppendPoint(call);
            extractFromVector(call);
            break;
          }
          case GEN_OCL_WRITE_IMAGE_I:
          case GEN_OCL_WRITE_IMAGE_UI:
          case GEN_OCL_WRITE_IMAGE_F:
          {
            if ((*CI)->getType()->isVectorTy()) 
              *CI = InsertToVector(call, *CI);
            ++CI;
            *CI = InsertToVector(call, *CI);
            break;
          }
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE2:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE4:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_IMAGE8:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE2:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE4:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE8:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_IMAGE16:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UC_IMAGE:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UC_IMAGE2:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UC_IMAGE4:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UC_IMAGE8:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UC_IMAGE16:
          {
            ++CI;
            ++CI;
            if ((*CI)->getType()->isVectorTy())
              *CI = InsertToVector(call, *CI);
            break;
          }
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM2:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM4:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_UI_MEM8:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM2:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM4:
          case GEN_OCL_SUB_GROUP_BLOCK_WRITE_US_MEM8:
          {
            if ((*CI)->getType()->isVectorTy())
              *CI = InsertToVector(call, *CI);
            break;
          }
          case GEN_OCL_VME:
          case GEN_OCL_IME:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM2:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM4:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_MEM8:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE2:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE4:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UI_IMAGE8:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM2:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM4:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_MEM8:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE2:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE4:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE8:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_US_IMAGE16:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UC_IMAGE2:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UC_IMAGE4:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UC_IMAGE8:
          case GEN_OCL_SUB_GROUP_BLOCK_READ_UC_IMAGE16:
            setAppendPoint(call);
            extractFromVector(call);
            break;
          case GEN_OCL_PRINTF:
            for (; CI != CS.arg_end(); ++CI)
              if ((*CI)->getType()->isVectorTy())
                *CI = InsertToVector(call, *CI);
            break;
        }
      }
    }
    return false;
  }

  bool Scalarize::scalarizeBitCast(BitCastInst* bt)
  {
    if(bt->getOperand(0)->getType()->isVectorTy())
      bt->setOperand(0, InsertToVector(bt, bt->getOperand(0)));
    if(bt->getType()->isVectorTy()) {
      setAppendPoint(bt);
      extractFromVector(bt);
    }

    return false;
  }

  bool Scalarize::scalarizeLoad(LoadInst* ld)
  {
    setAppendPoint(ld);
    extractFromVector(ld);
    return false;
  }

  bool Scalarize::scalarizeStore(StoreInst* st) {
    st->setOperand(0, InsertToVector(st, st->getValueOperand()));
    return false;
  }

  void Scalarize::replaceAllUsesOfWith(Instruction* from, Instruction* to) {
    GBE_ASSERT(from != NULL);
    if (from == to)
      return;
    for (auto &it : usedVecVals[from])
      setComponent(*(it.vecVals), it.id, to);
    usedVecVals[from].clear();
    from->replaceAllUsesWith(to);
  }

  bool Scalarize::scalarizeExtract(ExtractElementInst* extr)
  {
    //     %res = extractelement <n X ty> %foo, %i
    // ==> nothing (just use %foo's %ith component instead of %res)

    if (! isa<Constant>(extr->getOperand(1))) {
        // TODO: Variably referenced components. Probably handle/emulate through
        // a series of selects.
        //NOT_IMPLEMENTED; //gla::UnsupportedFunctionality("Variably referenced vector components");
        //TODO: This is a implement for the non-constant index, we use an allocated new vector
        //to store the need vector elements.
        Value* foo = extr->getOperand(0);
        Type* fooTy = foo ? foo->getType() : NULL;

        Value* Alloc;
        if(vectorAlloca.find(foo) == vectorAlloca.end())
        {
          BasicBlock &entry = extr->getParent()->getParent()->getEntryBlock();
          BasicBlock::iterator bbIter = entry.begin();
          while (isa<AllocaInst>(bbIter)) ++bbIter;

          IRBuilder<> allocBuilder(&entry);
          allocBuilder.SetInsertPoint(&*bbIter);

          Alloc = allocBuilder.CreateAlloca(fooTy, nullptr, "");
          for (int i = 0; i < GetComponentCount(foo); ++i)
          {
            Value* foo_i = getComponent(i, foo);
            assert(foo_i && "There is unhandled vector component");
            Value* idxs_i[] = {ConstantInt::get(intTy,0), ConstantInt::get(intTy,i)};
            Value* storePtr_i = builder->CreateGEP(Alloc, idxs_i);
            builder->CreateStore(foo_i, storePtr_i);
          }
          vectorAlloca[foo] = Alloc;
        }
        else Alloc = vectorAlloca[foo];

        Value* Idxs[] = {ConstantInt::get(intTy,0), extr->getOperand(1)};
        Value* getPtr = builder->CreateGEP(Alloc, Idxs);
        Value* loadComp = builder->CreateLoad(getPtr);
        extr->replaceAllUsesWith(loadComp);
        return true;
    }
    //if (isa<Argument>(extr->getOperand(0)))
    //  return false;
    else{
      int component = GetConstantInt(extr->getOperand(1));
      Value* v = getComponent(component, extr->getOperand(0));
      if(extr == v)
        return false;
      replaceAllUsesOfWith(dyn_cast<Instruction>(extr), dyn_cast<Instruction>(v));

      return true;
    }
  }

  bool Scalarize::scalarizeInsert(InsertElementInst* ins)
  {
    //     %res = insertValue <n x ty> %foo, %i
    // ==> nothing (just make a new VectorValues with the new component)

    if (! isa<Constant>(ins->getOperand(2))) {
      // TODO: Variably referenced components. Probably handle/emulate through
      // a series of selects.
      NOT_IMPLEMENTED;   //gla::UnsupportedFunctionality("Variably referenced vector components");
    }

    int component = GetConstantInt(ins->getOperand(2));

    VectorValues& vVals = vectorVals[ins];
    for (int i = 0; i < GetComponentCount(ins); ++i) {
      setComponent(vVals, i, i == component ? ins->getOperand(1)
                   : getComponent(i, ins->getOperand(0)));
    }

    return true;
  }

  void Scalarize::scalarizeArgs(Function& F)  {
    if (F.arg_empty())
      return;
    ReversePostOrderTraversal<Function*> rpot(&F);
    BasicBlock::iterator instI = (*rpot.begin())->begin();
    Instruction* instVal = &*instI;
    if(instVal == nullptr)
      return;
    builder->SetInsertPoint(instVal);

    Function::arg_iterator I = F.arg_begin(), E = F.arg_end();

    for (; I != E; ++I) {
      Type *type = I->getType();

      if(type->isVectorTy())
        extractFromVector(&*I);
    }
    return;
  }

  bool Scalarize::runOnFunction(Function& F)
  {
    switch (F.getCallingConv()) {
    case CallingConv::C:
    case CallingConv::Fast:
    case CallingConv::SPIR_KERNEL:
      break;
    default:
      GBE_ASSERTM(false, "Unsupported calling convention");
    }

    // As we inline all function calls, so skip non-kernel functions
    bool bKernel = isKernelFunction(F);
    if(!bKernel) return false;

    bool changed = false;
    module = F.getParent();
    intTy = IntegerType::get(module->getContext(), 32);
    floatTy = Type::getFloatTy(module->getContext());
    builder = new IRBuilder<>(module->getContext());

    scalarizeArgs(F);
    typedef ReversePostOrderTraversal<Function*> RPOTType;
    RPOTType rpot(&F);
    for (RPOTType::rpo_iterator bbI = rpot.begin(), bbE = rpot.end(); bbI != bbE; ++bbI) {
      for (BasicBlock::iterator instI = (*bbI)->begin(), instE = (*bbI)->end(); instI != instE; ++instI) {
        bool scalarized = scalarize(&*instI);
        if (scalarized) {
          changed = true;
          // TODO: uncomment when done
          deadList.push_back(&*instI);
        }
      }
    }

    // Fill in the incomplete phis
    for (SmallVectorImpl<PHINode*>::iterator phiI = incompletePhis.begin(), phiE = incompletePhis.end();
       phiI != phiE; ++phiI) {
      assert(canGetComponentArgs(*phiI) && "Phi's operands never scalarized");
      // Fill in each component of this phi
      VectorValues& vVals = vectorVals[*phiI];
      for (int c = 0; c < GetComponentCount(*phiI); ++c) {
        PHINode* compPhi = dyn_cast<PHINode>(vVals.getComponent(c));
        assert(compPhi && "Vector phi got scalarized to non-phis?");

        // Loop over pairs of operands: [Value*, BasicBlock*]
        for (unsigned int i = 0; i < (*phiI)->getNumOperands(); i++) {
          BasicBlock* bb = (*phiI)->getIncomingBlock(i);
          assert(bb && "Non-basic block incoming block?");
          compPhi->addIncoming(getComponent(c, (*phiI)->getOperand(i)), bb);
        }
      }
    }

    dce();
    incompletePhis.clear();
    vectorVals.clear();
    usedVecVals.clear();
    vectorAlloca.clear();

    delete builder;
    builder = 0;

    return changed;
  }

  void Scalarize::dce()
  {
    //two passes delete for some phinode
    for (std::vector<Instruction*>::reverse_iterator i = deadList.rbegin(), e = deadList.rend(); i != e; ++i) {
      (*i)->dropAllReferences();
      if((*i)->use_empty()) {
        (*i)->eraseFromParent();
        (*i) = NULL;
      }
    }
    for (std::vector<Instruction*>::reverse_iterator i = deadList.rbegin(), e = deadList.rend(); i != e; ++i) {
      if((*i) && (*i)->getParent())
        (*i)->eraseFromParent();
    }
    deadList.clear();
  }

  void Scalarize::getAnalysisUsage(AnalysisUsage& AU) const
  {
  }

  void Scalarize::print(raw_ostream&, const Module*) const
  {
      return;
  }
  FunctionPass* createScalarizePass()
  {
    return new Scalarize();
  }
  char Scalarize::ID = 0;

} // end namespace
