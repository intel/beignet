/*
 * Copyright © 2014 Intel Corporation
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
 */
#include <list>
#include "llvm_includes.hpp"

#include "ir/unit.hpp"
#include "llvm_gen_backend.hpp"
#include "ocl_common_defines.h"

using namespace llvm;

namespace gbe {
  BitCastInst *isInvokeBitcast(Instruction *I) {
    BitCastInst* bt = dyn_cast<BitCastInst>(I);
    if (bt == NULL)
      return NULL;

    Type* type = bt->getOperand(0)->getType();
    if(!type->isPointerTy())
      return NULL;

    PointerType *pointerType = dyn_cast<PointerType>(type);
    Type *pointed = pointerType->getElementType();
    if(!pointed->isFunctionTy())
      return NULL;

    Function *Fn = dyn_cast<Function>(bt->getOperand(0));
    if(Fn == NULL)
      return NULL;

    /* This is a fake, to check the function bitcast is for block or not */
    std::string fnName = Fn->getName();
    if(fnName.find("_invoke") == std::string::npos)
      return NULL;

    return bt;
  }

  void mutateArgAddressSpace(Argument *arg)
  {
    std::list<Value *>WorkList;
    WorkList.push_back(arg);

    while(!WorkList.empty()) {
      Value *v = WorkList.front();

      for (Value::use_iterator iter = v->use_begin(); iter != v->use_end(); ++iter) {
        // After LLVM 3.5, use_iterator points to 'Use' instead of 'User',
        // which is more straightforward.
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 5)
        User *theUser = *iter;
#else
        User *theUser = iter->getUser();
#endif
        // becareful with sub operation
        if (isa<StoreInst>(theUser) || isa<LoadInst>(theUser))
          continue;

        WorkList.push_back(theUser);
      }

      PointerType *ty = dyn_cast<PointerType>(v->getType());
      if(ty == NULL) continue;   //should only one argument, private pointer type
      ty = PointerType::get(ty->getPointerElementType(), 1);
      v->mutateType(ty);
      WorkList.pop_front();
    }
  }

  Function* setFunctionAsKernel(Module *mod, Function *Fn)
  {
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR >= 9)
    LLVMContext &Context = mod->getContext();
    Type *intTy = IntegerType::get(mod->getContext(), 32);
    SmallVector<llvm::Metadata *, 5> kernelMDArgs;

    // MDNode for the kernel argument address space qualifiers.
    SmallVector<llvm::Metadata *, 8> addressQuals;

    // MDNode for the kernel argument access qualifiers (images only).
    SmallVector<llvm::Metadata *, 8> accessQuals;

    // MDNode for the kernel argument type names.
    SmallVector<llvm::Metadata *, 8> argTypeNames;

    // MDNode for the kernel argument base type names.
    SmallVector<llvm::Metadata *, 8> argBaseTypeNames;

    // MDNode for the kernel argument type qualifiers.
    SmallVector<llvm::Metadata *, 8> argTypeQuals;

    // MDNode for the kernel argument names.
    SmallVector<llvm::Metadata *, 8> argNames;

    //Because paramter type changed, so must re-create the invoke function and replace the old one
    std::vector<Type *> ParamTys;
    ValueToValueMapTy VMap;
    for (Function::arg_iterator I = Fn->arg_begin(), E = Fn->arg_end(); I != E; ++I) {
      PointerType *ty = dyn_cast<PointerType>(I->getType());
      if(ty && ty->getAddressSpace() == 0) //Foce set the address space to global
        ty = PointerType::get(ty->getPointerElementType(), 1);
      ParamTys.push_back(ty);
    }
    FunctionType* NewFT = FunctionType::get(Fn->getReturnType(), ParamTys, false);
    Function* NewFn = Function::Create(NewFT, Function::ExternalLinkage, Fn->getName());
    SmallVector<ReturnInst*, 8> Returns;

    Function::arg_iterator NewFnArgIt = NewFn->arg_begin();
    for (Function::arg_iterator I = Fn->arg_begin(), E = Fn->arg_end(); I != E; ++I) {
      std::string ArgName = I->getName();
      NewFnArgIt->setName(ArgName);
      VMap[&*I] = &(*NewFnArgIt++);
    }
    CloneFunctionInto(NewFn, Fn, VMap, /*ModuleLevelChanges=*/true, Returns);

    Fn->setName("__d" + Fn->getName());
    mod->getFunctionList().push_back(NewFn);
    //mod->getOrInsertFunction(NewFn->getName(), NewFn->getFunctionType(),
    //                         NewFn->getAttributes());

    for (Function::arg_iterator I = NewFn->arg_begin(), E = NewFn->arg_end(); I != E; ++I) {
      PointerType *ty = dyn_cast<PointerType>(I->getType());
      //mutate the address space  of all pointer derive from the argmument from private to global
      if(ty && ty->getAddressSpace() == 1)
        mutateArgAddressSpace(&*I);
      //ty = dyn_cast<PointerType>(I->getType());

      addressQuals.push_back(llvm::ConstantAsMetadata::get(ConstantInt::get(intTy, ty->getAddressSpace())));
      accessQuals.push_back(llvm::MDString::get(Context, "none"));
      argTypeNames.push_back(llvm::MDString::get(Context, "char*"));
      argBaseTypeNames.push_back(llvm::MDString::get(Context, "char*"));
      argTypeQuals.push_back(llvm::MDString::get(Context, ""));
      argNames.push_back(llvm::MDString::get(Context, I->getName()));
    }

    //If run to here, llvm version always > 3.9, add the version check just for build.
    NewFn->setMetadata("kernel_arg_addr_space",
                    llvm::MDNode::get(Context, addressQuals));
    NewFn->setMetadata("kernel_arg_access_qual",
                    llvm::MDNode::get(Context, accessQuals));
    NewFn->setMetadata("kernel_arg_type",
                    llvm::MDNode::get(Context, argTypeNames));
    NewFn->setMetadata("kernel_arg_base_type",
                    llvm::MDNode::get(Context, argBaseTypeNames));
    NewFn->setMetadata("kernel_arg_type_qual",
                    llvm::MDNode::get(Context, argTypeQuals));
    NewFn->setMetadata("kernel_arg_name",
                    llvm::MDNode::get(Context, argNames));
    return NewFn;
#else
    assert(0);  //only opencl 2.0 could reach hear.
    return Fn;
#endif
  }

  Instruction* replaceInst(Instruction *I, Value *v)
  {
    //The bitcast is instruction
    if(BitCastInst *bt = dyn_cast<BitCastInst>(&*I)) {
      bt->replaceAllUsesWith(v);
      return bt;
    }
    return NULL;
  }

  void collectDeviceEnqueueInfo(Module *mod, ir::Unit &unit)
  {
    std::set<Instruction*> deadInsnSet;
    std::set<Function*> deadFunctionSet;
    std::map<Value*, std::string> blocks;
    if (getModuleOclVersion(mod) < 200)
      return;

    for (Module::iterator SF = mod->begin(), E = mod->end(); SF != E; ++SF) {
      Function *f = &*SF;
      if (f->isDeclaration()) continue;

      for (inst_iterator I = inst_begin(f), E = inst_end(f); I != E; ++I) {
        if (BitCastInst* bt = isInvokeBitcast(&*I)) {
          /* handle block description, convert the instruction that store block
           * invoke pointer to store the index in the unit's block functions index.*/
          Function *Fn = dyn_cast<Function>(bt->getOperand(0));

          std::string fnName = Fn->getName();
          int index = -1;
          for(size_t i=0; i<unit.blockFuncs.size(); i++) {
            if(unit.blockFuncs[i] == fnName) {
              index = i;
              break;
            }
          }
          if(index == -1){
            unit.blockFuncs.push_back(fnName);
            index = unit.blockFuncs.size() - 1;
          }

          for (Value::use_iterator iter = bt->use_begin(); iter != bt->use_end(); ++iter) {
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 5)
            User *theUser = *iter;
#else
            User *theUser = iter->getUser();
#endif
            if(StoreInst *st = dyn_cast<StoreInst>(theUser)) {
              GetElementPtrInst * gep = dyn_cast<GetElementPtrInst>(st->getPointerOperand());
              if(gep)
                blocks[gep->getOperand(0)] = fnName;
            }
          }

          if(StoreInst* st = dyn_cast<StoreInst>(&*I)) {
            GetElementPtrInst * gep = dyn_cast<GetElementPtrInst>(st->getPointerOperand());
            if(gep)
              blocks[gep->getOperand(0)] = fnName;
          }

          Value *v = Constant::getIntegerValue(bt->getType(), APInt(unit.getPointerSize(), index));
          bt->replaceAllUsesWith(v);
          deadInsnSet.insert(bt);
        }

        if(CallInst *CI = dyn_cast<CallInst>(&*I)) {
          IRBuilder<> builder(CI->getParent(), BasicBlock::iterator(CI));
          if(CI->getCalledFunction() == NULL) {
            //unnamed call function, parse the use to find the define of called function
            SmallVector<Value*, 16> args(CI->op_begin(), CI->op_end()-1);

            Value *v = CI->getCalledValue();
            BitCastInst* bt = dyn_cast<BitCastInst>(v);
            if(bt == NULL)
              continue;

            LoadInst* ld = dyn_cast<LoadInst>(bt->getOperand(0));
            if(ld == NULL)
              continue;

            GetElementPtrInst * gep = dyn_cast<GetElementPtrInst>(ld->getPointerOperand());
            if(gep == NULL)
              continue;

            BitCastInst* fnPointer = dyn_cast<BitCastInst>(gep->getOperand(0));
            if(fnPointer == NULL)
              continue;

            if(BitCastInst* bt = dyn_cast<BitCastInst>(fnPointer->getOperand(0))) {
              std::string fnName = blocks[bt->getOperand(0)];
              Function* f = mod->getFunction(fnName);
              CallInst *newCI = builder.CreateCall(f, args);
              CI->replaceAllUsesWith(newCI);
              deadInsnSet.insert(CI);
              continue;
            }

            //the function is global variable
            if(GlobalVariable* gv = dyn_cast<GlobalVariable>(fnPointer->getOperand(0))) {
              Constant *c = gv->getInitializer();
              ConstantExpr *expr = dyn_cast<ConstantExpr>(c->getOperand(3));
              BitCastInst *bt = dyn_cast<BitCastInst>(expr->getAsInstruction());
              Function* f = dyn_cast<Function>(bt->getOperand(0));
              CallInst *newCI = builder.CreateCall(f, args);
              CI->replaceAllUsesWith(newCI);
              deadInsnSet.insert(CI);
              continue;
            }

            ld = dyn_cast<LoadInst>(fnPointer->getOperand(0));
            if(ld == NULL)
              continue;

            if(GlobalVariable *gv = dyn_cast<GlobalVariable>(ld->getPointerOperand())) {
              ConstantExpr *expr = dyn_cast<ConstantExpr>(gv->getInitializer());
              BitCastInst *bt = dyn_cast<BitCastInst>(expr->getAsInstruction());
              GlobalVariable *block_literal = dyn_cast<GlobalVariable>(bt->getOperand(0));
              Constant *v = block_literal->getInitializer();
              expr = dyn_cast<ConstantExpr>(v->getOperand(3));
              bt = dyn_cast<BitCastInst>(expr->getAsInstruction());
              Function* f = dyn_cast<Function>(bt->getOperand(0));
              CallInst *newCI = builder.CreateCall(f, args);
              CI->replaceAllUsesWith(newCI);
              deadInsnSet.insert(CI);
              continue;
            }

            if(AllocaInst *ai = dyn_cast<AllocaInst>(ld->getPointerOperand())) {
              Value *v = NULL;
              for (Value::use_iterator iter = ai->use_begin(); iter != ai->use_end(); ++iter) {
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 5)
                User *theUser = *iter;
#else
                User *theUser = iter->getUser();
#endif
                if(StoreInst *st = dyn_cast<StoreInst>(theUser)) {
                  bt = dyn_cast<BitCastInst>(st->getValueOperand());
                  if(bt)
                    v = bt->getOperand(0);
                }
              }
              if(blocks.find(v) == blocks.end()) {
                if(GlobalVariable *gv = dyn_cast<GlobalVariable>(v)) {
                  Constant *c = gv->getInitializer();
                  ConstantExpr *expr = dyn_cast<ConstantExpr>(c->getOperand(3));
                  BitCastInst *bt = dyn_cast<BitCastInst>(expr->getAsInstruction());
                  Function* f = dyn_cast<Function>(bt->getOperand(0));
                  blocks[v] = f->getName();
                }
              }

              std::string fnName = blocks[v];
              Function* f = mod->getFunction(fnName);
              CallInst *newCI = builder.CreateCall(f, args);
              CI->replaceAllUsesWith(newCI);
              deadInsnSet.insert(CI);
              continue;
            }
            //can't find the function's define
            assert(0);
          } else {
            //handle enqueue_kernel function call
            Function *fn = CI->getCalledFunction();
            if (fn->getName().find("enqueue_kernel") == std::string::npos)
              continue;

            //block parameter's index, 3 or 6
            int block_index = 3;
            Type *type = CI->getArgOperand(block_index)->getType();
            if(type->isIntegerTy())
                block_index = 6;
            Value *block = CI->getArgOperand(block_index);
            while(isa<BitCastInst>(block))
               block = dyn_cast<BitCastInst>(block)->getOperand(0);
            LoadInst *ld = dyn_cast<LoadInst>(block);
            Value *v = NULL;
            if(ld) {
              Value *block = ld->getPointerOperand();
              for (Value::use_iterator iter = block->use_begin(); iter != block->use_end(); ++iter) {
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 5)
                User *theUser = *iter;
#else
                User *theUser = iter->getUser();
#endif
                if(StoreInst *st = dyn_cast<StoreInst>(theUser)) {
                  BitCastInst *bt = dyn_cast<BitCastInst>(st->getValueOperand());
                  if(bt)
                    v = bt->getOperand(0);
                }
              }
              if(blocks.find(v) == blocks.end()) {
                if(GlobalVariable *gv = dyn_cast<GlobalVariable>(v)) {
                  Constant *c = gv->getInitializer();
                  ConstantExpr *expr = dyn_cast<ConstantExpr>(c->getOperand(3));
                  BitCastInst *bt = dyn_cast<BitCastInst>(expr->getAsInstruction());
                  Function* f = dyn_cast<Function>(bt->getOperand(0));
                  blocks[v] = f->getName();
                }
              }
            } else if(isa<AllocaInst>(block)) {
              v = block;
            }
            std::string fnName = blocks[v];
            Function* f = mod->getFunction(fnName);
            deadFunctionSet.insert(f);
            f = setFunctionAsKernel(mod, f);

            if( fn->isVarArg() ) {
              //enqueue function with slm, convert to __gen_enqueue_kernel_slm call
              //store the slm information to a alloca address.
              int start = block_index + 1;
              int count = CI->getNumArgOperands() - start;
              Type *intTy = IntegerType::get(mod->getContext(), 32);

              AllocaInst *AI = builder.CreateAlloca(intTy, ConstantInt::get(intTy, count));

              for(uint32_t i = start; i < CI->getNumArgOperands(); i++) {
                Value *ptr = builder.CreateGEP(AI, ConstantInt::get(intTy, i-start));
                builder.CreateStore(CI->getArgOperand(i), ptr);
              }
              SmallVector<Value*, 16> args(CI->op_begin(), CI->op_begin() + 3);
              args.push_back(CI->getArgOperand(block_index));
              args.push_back(ConstantInt::get(intTy, count));
              args.push_back(AI);

              std::vector<Type *> ParamTys;
              for (Value** I = args.begin(); I != args.end(); ++I)
                ParamTys.push_back((*I)->getType());
              CallInst* newCI = builder.CreateCall(cast<llvm::Function>(mod->getOrInsertFunction(
                              "__gen_enqueue_kernel_slm", FunctionType::get(intTy, ParamTys, false))), args);
              CI->replaceAllUsesWith(newCI);
              deadInsnSet.insert(CI);
            }
          }
        }
      }
    }

    for (auto it: deadInsnSet) {
      it->eraseFromParent();
    }

    for (auto it: deadFunctionSet) {
      it->eraseFromParent();
    }
  }
};
