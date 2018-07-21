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
 */

#include "llvm/Config/llvm-config.h"
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 35
#include <set>

#include "llvm_includes.hpp"

#include "llvm/llvm_gen_backend.hpp"
#include "sys/map.hpp"


using namespace llvm;

namespace gbe {
    class CustomLoopUnroll : public LoopPass
    {
    public:
      static char ID;
      CustomLoopUnroll() :
       LoopPass(ID) {}

      void getAnalysisUsage(AnalysisUsage &AU) const {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
        AU.addRequired<LoopInfoWrapperPass>();
        AU.addPreserved<LoopInfoWrapperPass>();
#else
        AU.addRequired<LoopInfo>();
        AU.addPreserved<LoopInfo>();
#endif
        AU.addRequiredID(LoopSimplifyID);
        AU.addPreservedID(LoopSimplifyID);
        AU.addRequiredID(LCSSAID);
        AU.addPreservedID(LCSSAID);
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
        AU.addRequired<ScalarEvolutionWrapperPass>();
        AU.addPreserved<ScalarEvolutionWrapperPass>();
#else
        AU.addRequired<ScalarEvolution>();
        AU.addPreserved<ScalarEvolution>();
#endif
      // FIXME: Loop unroll requires LCSSA. And LCSSA requires dom info.
      // If loop unroll does not preserve dom info then LCSSA pass on next
      // loop will receive invalid dom info.
      // For now, recreate dom info, if loop is unrolled.
      AU.addPreserved<DominatorTreeWrapperPass>();

      }

      // Returns the value associated with the given metadata node name (for
      // example, "llvm.loop.unroll.count").  If no such named metadata node
      // exists, then nullptr is returned.
      static const MDNode *GetUnrollMetadataValue(const Loop *L,
                                                          StringRef Name) {
        MDNode *LoopID = L->getLoopID();
        if (!LoopID) return nullptr;
        // First operand should refer to the loop id itself.
        assert(LoopID->getNumOperands() > 0 && "requires at least one operand");
        assert(LoopID->getOperand(0) == LoopID && "invalid loop id");
        for (unsigned i = 1, e = LoopID->getNumOperands(); i < e; ++i) {
          const MDNode *MD = dyn_cast<MDNode>(LoopID->getOperand(i));
          if (!MD) continue;
          const MDString *S = dyn_cast<MDString>(MD->getOperand(0));
          if (!S) continue;
          if (Name.equals(S->getString())) {
            return MD;
          }
        }
        return nullptr;
      }

      static unsigned GetUnrollCount(const Loop *L,
                                            StringRef Name) {
        const MDNode *MD = GetUnrollMetadataValue(L, "llvm.loop.unroll.count");
        if (MD) {
          assert(MD->getNumOperands() == 2 &&
                 "Unroll count hint metadata should have two operands.");
          unsigned Count;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
          Count = mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
#else
          Count = cast<ConstantInt>(MD->getOperand(1))->getZExtValue();
#endif
          assert(Count >= 1 && "Unroll count must be positive.");
          return Count;
        }
        return 0;
      }

      void setUnrollID(Loop *L, bool enable) {
        assert(enable);
        LLVMContext &Context = L->getHeader()->getContext();
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
        SmallVector<Metadata *, 2> forceUnroll;
        forceUnroll.push_back(MDString::get(Context, "llvm.loop.unroll.enable"));
        MDNode *forceUnrollNode = MDNode::get(Context, forceUnroll);
        SmallVector<Metadata *, 4> Vals;
        Vals.push_back(NULL);
        Vals.push_back(forceUnrollNode);
#else
        SmallVector<Value *, 2> forceUnroll;
        forceUnroll.push_back(MDString::get(Context, "llvm.loop.unroll.enable"));
        forceUnroll.push_back(ConstantInt::get(Type::getInt1Ty(Context), enable));
        MDNode *forceUnrollNode = MDNode::get(Context, forceUnroll);
        SmallVector<Value *, 4> Vals;
        Vals.push_back(NULL);
        Vals.push_back(forceUnrollNode);
#endif
        MDNode *NewLoopID = MDNode::get(Context, Vals);
        // Set operand 0 to refer to the loop id itself.
        NewLoopID->replaceOperandWith(0, NewLoopID);
        L->setLoopID(NewLoopID);
      }

      static bool hasPrivateLoadStore(Loop *L) {
        const std::vector<Loop*> subLoops = L->getSubLoops();
        std::set<BasicBlock*> subBlocks, blocks;

        for(auto l : subLoops)
          for(auto bb : l->getBlocks())
            subBlocks.insert(bb);
        for(auto bb : L->getBlocks())
          if (subBlocks.find(bb) == subBlocks.end())
            blocks.insert(bb);
        for(auto bb : blocks) {
          for (BasicBlock::iterator inst = bb->begin(), instE = bb->end(); inst != instE; ++inst) {
            unsigned addrSpace = -1;
            if (isa<LoadInst>(*inst)) {
              LoadInst *ld = cast<LoadInst>(&*inst);
              addrSpace = ld->getPointerAddressSpace();
            }
            else if (isa<StoreInst>(*inst)) {
              StoreInst *st = cast<StoreInst>(&*inst);
              addrSpace = st->getPointerAddressSpace();
            }
            if (addrSpace == 0)
              return true;
          }
        }
        return false;
      }
      // If one loop has very large self trip count
      // we don't want to unroll it.
      // self trip count means trip count divide by the parent's trip count. for example
      // for (int i = 0; i < 16; i++) {
      //   for (int j = 0; j < 4; j++) {
      //     for (int k = 0; k < 2; k++) {
      //       ...
      //     }
      //     ...
      //   }
      // The inner loops j and k could be unrolled, but the loop i will not be unrolled.
      // The return value true means the L could be unrolled, otherwise, it could not
      // be unrolled.
      bool handleParentLoops(Loop *L, LPPassManager &LPM) {
        Loop *currL = L;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
        ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
        LoopInfo &loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
#else
        ScalarEvolution *SE = &getAnalysis<ScalarEvolution>();
#endif
        BasicBlock *ExitBlock = currL->getLoopLatch();
        if (!ExitBlock || !L->isLoopExiting(ExitBlock))
          ExitBlock = currL->getExitingBlock();

        unsigned currTripCount = 0;
        bool shouldUnroll = true;
        if (ExitBlock)
          currTripCount = SE->getSmallConstantTripCount(L, ExitBlock);

        if (currTripCount > 32) {
          shouldUnroll = false;
          //Don't change the unrollID if doesn't force unroll.
          //setUnrollID(currL, false);
          return shouldUnroll;
        }

        while(currL) {
          Loop *parentL = currL->getParentLoop();
          unsigned parentTripCount = 0;
          if (parentL) {
            BasicBlock *parentExitBlock = parentL->getLoopLatch();
            if (!parentExitBlock || !parentL->isLoopExiting(parentExitBlock))
              parentExitBlock = parentL->getExitingBlock();

            if (parentExitBlock)
              parentTripCount = SE->getSmallConstantTripCount(parentL, parentExitBlock);
          }
          if (parentTripCount != 0 && currTripCount * parentTripCount > 32) {
            //Don't change the unrollID if doesn't force unroll.
            //setUnrollID(parentL, false);
#if LLVM_VERSION_MAJOR >= 6
            loopInfo.erase(parentL);
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
            loopInfo.markAsRemoved(parentL);
#else
            LPM.deleteLoopFromQueue(parentL);
#endif
            return shouldUnroll;
          }
          currL = parentL;
          currTripCount = parentTripCount * currTripCount;
        }
        return shouldUnroll;
      }

      // Analyze the outermost BBs of this loop, if there are
      // some private load or store, we change it's loop meta data
      // to indicate more aggresive unrolling on it.
      virtual bool runOnLoop(Loop *L, LPPassManager &LPM) {
        const MDNode *Enable = GetUnrollMetadataValue(L, "llvm.loop.unroll.enable");
        if (Enable)
          return false;
        const unsigned Count = GetUnrollCount(L, "llvm.loop.unroll.count");
        if (Count > 0)
          return false;

        if (!handleParentLoops(L, LPM))
          return false;

        if (!hasPrivateLoadStore(L))
          return false;
        setUnrollID(L, true);
        return true;
      }

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
      virtual StringRef getPassName() const
#else
      virtual const char *getPassName() const
#endif
      {
        return "SPIR backend: custom loop unrolling pass";
      }

    };

    char CustomLoopUnroll::ID = 0;

    LoopPass *createCustomLoopUnrollPass() {
      return new CustomLoopUnroll();
    }
} // end namespace
#endif
