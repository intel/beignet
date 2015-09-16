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

/**
 * \file llvm_barrier_nodup.cpp
 *
 *  This pass is to remove or add noduplicate function attribute for barrier functions.
 *  Basically, we want to set NoDuplicate for those __gen_barrier_xxx functions. But if
 *  a sub function calls those barrier functions, the sub function will not be inlined
 *  in llvm's inlining pass. This is what we don't want. As inlining such a function in
 *  the caller is safe, we just don't want it to duplicate the call. So Introduce this
 *  pass to remove the NoDuplicate function attribute before the inlining pass and restore
 *  it after.
 *  
 */

#include "llvm_includes.hpp"

#include "llvm/llvm_gen_backend.hpp"
#include "sys/map.hpp"


using namespace llvm;

namespace gbe {
    class BarrierNodup : public ModulePass
    {
    public:
      static char ID;
      BarrierNodup(bool nodup) :
        ModulePass(ID), nodup(nodup) {}

      void getAnalysisUsage(AnalysisUsage &AU) const {

      }

      virtual const char *getPassName() const {
        return "SPIR backend: set barrier no duplicate attr";
      }

      virtual bool runOnModule(Module &M)
      {
        using namespace llvm;
        bool changed = false;
        for (auto &F : M) {
          if (F.getName() == "__gen_ocl_barrier_local_and_global" ||
              F.getName() == "__gen_ocl_barrier_local"            ||
              F.getName() == "__gen_ocl_barrier_global") {
            if (nodup) {
              if (!F.hasFnAttribute(Attribute::NoDuplicate)) {
                F.addFnAttr(Attribute::NoDuplicate);
                changed = true;
              }
            } else {
              if (F.hasFnAttribute(Attribute::NoDuplicate)) {
                auto attrs = F.getAttributes();
                F.setAttributes(attrs.removeAttribute(M.getContext(),
                                AttributeSet::FunctionIndex,
                                Attribute::NoDuplicate));
                changed = true;
              }
            }
          }
        }

        return changed;
      }
    private:
      bool nodup;
    };


    ModulePass *createBarrierNodupPass(bool Nodup) {
      return new BarrierNodup(Nodup);
    }

    char BarrierNodup::ID = 0;
} // end namespace
