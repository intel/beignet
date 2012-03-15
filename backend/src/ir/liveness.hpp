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
 * \file liveness.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_LIVENESS_HPP__
#define __GBE_IR_LIVENESS_HPP__

#include "sys/map.hpp"
#include "sys/set.hpp"
#include "ir/register.hpp"
#include "ir/function.hpp"

namespace gbe {
namespace ir {

  // Liveness is computed per function
  class Function;

  /*! Compute liveness of each register */
  class Liveness : public NonCopyable
  {
  public:
    Liveness(Function &fn);
    ~Liveness(void);
    /*! Set of variables used upwards in the block (before a definition) */
    typedef set<Register> UEVar;
    /*! Set of variables alive at the exit of the block */
    typedef set<Register> LiveOut;
    /*! Set of variables actually killed in each block */
    typedef set<Register> VarKill;
    /*! Per-block info */
    struct BlockInfo {
      BlockInfo(const BasicBlock &bb) : bb(bb) {}
      const BasicBlock &bb;
      INLINE bool inUpwardUsed(Register reg) const {
        return upwardUsed.find(reg) != upwardUsed.end();
      }
      INLINE bool inLiveOut(Register reg) const {
        return liveOut.find(reg) != liveOut.end();
      }
      INLINE bool inVarKill(Register reg) const {
        return varKill.find(reg) != varKill.end();
      }
      UEVar upwardUsed;
      LiveOut liveOut;
      VarKill varKill;
    };
    /*! Gives for each block the variables alive at entry / exit */
    typedef map<const BasicBlock*, BlockInfo*> Info;
    /*! Return the complete liveness info */
    INLINE const Info &getLiveness(void) const { return liveness; }
    /*! Return the function the liveness was computed on */
    INLINE const Function &getFunction(void) const { return fn; }
  private:
    /*! Store the liveness of all blocks */
    Info liveness;
    /*! Compute the liveness for this function */
    Function &fn;
    /*! Initialize UEVar and VarKill per block */
    void initBlock(const BasicBlock &bb);
    /*! Initialize UEVar and VarKill per instruction */
    void initInstruction(BlockInfo &info, const Instruction &insn);
    /*! Now really compute LiveOut based on UEVar and VarKill */
    void computeLiveOut(void);
    /*! Actually do something for each successor of *all* blocks */
    template <typename T>
    void forEachSuccessor(const T &functor) {
      // Iterate on all blocks
      for (auto it = liveness.begin(); it != liveness.end(); ++it) {
        BlockInfo &info = *it->second;
        const BasicBlock &bb = info.bb;
        const BlockSet set = bb.getSuccessorSet();
        // Iterate over all successors
        for (auto other = set.begin(); other != set.end(); ++other) {
          auto otherInfo = liveness.find(*other);
          GBE_ASSERT(otherInfo != liveness.end() && otherInfo->second != NULL);
          functor(info, *otherInfo->second);
        }
      }
    }
  };

  /*! Output a nice ASCII reprensation of the liveness */
  std::ostream &operator<< (std::ostream &out, const Liveness &liveness);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_LIVENESS_HPP__ */

