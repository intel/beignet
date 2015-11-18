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

//===-- ExpandUtils.cpp - Helper functions for expansion passes -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm_includes.hpp"

#include "llvm_gen_backend.hpp"

using namespace llvm;
namespace llvm {

  Instruction *PhiSafeInsertPt(Use *U) {
    Instruction *InsertPt = cast<Instruction>(U->getUser());
    if (PHINode *PN = dyn_cast<PHINode>(InsertPt)) {
      // We cannot insert instructions before a PHI node, so insert
      // before the incoming block's terminator.  This could be
      // suboptimal if the terminator is a conditional.
      InsertPt = PN->getIncomingBlock(*U)->getTerminator();
    }
    return InsertPt;
  }

  void PhiSafeReplaceUses(Use *U, Value *NewVal) {
    User *UR = U->getUser();
    if (PHINode *PN = dyn_cast<PHINode>(UR)) {
      // A PHI node can have multiple incoming edges from the same
      // block, in which case all these edges must have the same
      // incoming value.
      BasicBlock *BB = PN->getIncomingBlock(*U);
      for (unsigned I = 0; I < PN->getNumIncomingValues(); ++I) {
        if (PN->getIncomingBlock(I) == BB)
          PN->setIncomingValue(I, NewVal);
      }
    } else {
      UR->replaceUsesOfWith(U->get(), NewVal);
    }
  }

  Function *RecreateFunction(Function *Func, FunctionType *NewType) {
    Function *NewFunc = Function::Create(NewType, Func->getLinkage());
    NewFunc->copyAttributesFrom(Func);
    Func->getParent()->getFunctionList().insert(ilist_iterator<Function>(Func), NewFunc);
    NewFunc->takeName(Func);
    NewFunc->getBasicBlockList().splice(NewFunc->begin(),
                                        Func->getBasicBlockList());
    Func->replaceAllUsesWith(
        ConstantExpr::getBitCast(NewFunc,
                                 Func->getFunctionType()->getPointerTo()));
    return NewFunc;
  }
}
