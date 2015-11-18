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
 */

// Imported from pNaCl project
// Copyright (c) 2003-2014 University of Illinois at Urbana-Champaign.
// All rights reserved.
//
// Developed by:
//
//    LLVM Team
//
//    University of Illinois at Urbana-Champaign
//
//    http://llvm.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimers.
//
//   * Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimers in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the names of the LLVM Team, University of Illinois at
//      Urbana-Champaign, nor the names of its contributors may be used to
//      endorse or promote products derived from this Software without specific
//      prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.

//===- ExpandConstantExpr.cpp - Convert ConstantExprs to Instructions------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License.
//
//===----------------------------------------------------------------------===//
//
// This pass expands out ConstantExprs into Instructions.
//
// Note that this only converts ConstantExprs that are referenced by
// Instructions.  It does not convert ConstantExprs that are used as
// initializers for global variables.
//
// This simplifies the language so that the PNaCl translator does not
// need to handle ConstantExprs as part of a stable wire format for
// PNaCl.
//
//===----------------------------------------------------------------------===//

#include <map>
#include "llvm_includes.hpp"
#include "llvm_gen_backend.hpp"

using namespace llvm;

static bool expandInstruction(Instruction *Inst);

namespace {
  // This is a FunctionPass because our handling of PHI nodes means
  // that our modifications may cross BasicBlocks.
  struct ExpandConstantExpr : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    ExpandConstantExpr() : FunctionPass(ID) {
    }

    virtual bool runOnFunction(Function &Func);
  };
}

char ExpandConstantExpr::ID = 0;

static Value *expandConstantExpr(Instruction *InsertPt, ConstantExpr *Expr) {
  Instruction *NewInst = Expr->getAsInstruction();
  NewInst->insertBefore(InsertPt);
  NewInst->setName("expanded");
  expandInstruction(NewInst);
  return NewInst;
}

// For a constant vector, it may contain some constant expressions.
// We need to expand each expressions then recreate this vector by
// using InsertElement instruction. Thus we can eliminate all the
// constant expressions.
static Value *expandConstantVector(Instruction *InsertPt, ConstantVector *CV) {
  int elemNum = CV->getType()->getNumElements();
  Type *IntTy = IntegerType::get(CV->getContext(), 32);

  BasicBlock::iterator InsertPos(InsertPt);
  IRBuilder<> IRB(&*InsertPos);
  Value *vec = UndefValue::get(CV->getType());
  for (int i = 0; i < elemNum; i++) {
    Value *idx = ConstantInt::get(IntTy, i);
    if (dyn_cast<ConstantVector>(CV->getOperand(i)))
      vec = IRB.CreateInsertElement(vec, expandConstantVector(InsertPt, dyn_cast<ConstantVector>(CV->getOperand(i))), idx);
    else if (dyn_cast<ConstantExpr>(CV->getOperand(i)))
      vec = IRB.CreateInsertElement(vec, expandConstantExpr(InsertPt, dyn_cast<ConstantExpr>(CV->getOperand(i))), idx);
    else
      vec = IRB.CreateInsertElement(vec, CV->getOperand(i), idx);
  }
  return vec;
}

// Whether a constant vector contains constant expression which need to expand.
static bool needExpand(ConstantVector *CV) {
  int elemNum = CV->getType()->getNumElements();
  for (int i = 0; i < elemNum; i++) {
    Constant *C = CV->getOperand(i);
    if (dyn_cast<ConstantExpr>(C))
      return true;
    if (dyn_cast<ConstantVector>(C))
      if (needExpand(dyn_cast<ConstantVector>(C)))
        return true;
  }
  return false;
}

static bool expandInstruction(Instruction *Inst) {
  // A landingpad can only accept ConstantExprs, so it should remain
  // unmodified.
  if (isa<LandingPadInst>(Inst))
    return false;

  bool Modified = false;
  for (unsigned OpNum = 0; OpNum < Inst->getNumOperands(); OpNum++) {
    if (ConstantExpr *Expr =
        dyn_cast<ConstantExpr>(Inst->getOperand(OpNum))) {
      Modified = true;
      Use *U = &Inst->getOperandUse(OpNum);
      PhiSafeReplaceUses(U, expandConstantExpr(PhiSafeInsertPt(U), Expr));
    }
    else {
      ConstantVector *CV = dyn_cast<ConstantVector>(Inst->getOperand(OpNum));
      if (CV && needExpand(CV)) {
        Modified = true;
        Use *U = &Inst->getOperandUse(OpNum);
        PhiSafeReplaceUses(U, expandConstantVector(PhiSafeInsertPt(U), CV));
      }
    }
  }
  return Modified;
}

bool ExpandConstantExpr::runOnFunction(Function &Func) {
  bool Modified = false;
  for (llvm::Function::iterator BB = Func.begin(), E = Func.end();
       BB != E;
       ++BB) {
    for (BasicBlock::InstListType::iterator Inst = BB->begin(), E = BB->end();
         Inst != E;
         ++Inst) {
      Modified |= expandInstruction(&*Inst);
    }
  }
  return Modified;
}

FunctionPass *llvm::createExpandConstantExprPass() {
  return new ExpandConstantExpr();
}
