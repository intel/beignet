/*
 * Copyright © 2017 Intel Corporation
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
 * Author: Guo Yejun <yejun.guo@intel.com>
 */

#include <assert.h>
#include "ir/context.hpp"
#include "ir/value.hpp"
#include "ir/constopt.hpp"
#include "sys/set.hpp"

namespace gbe {
namespace ir {

  class FunctionStructArgConstOffsetFolder : public Context
  {
  public:
    /*! Build the helper structure */
    FunctionStructArgConstOffsetFolder(Unit &unit) : Context(unit) {
      records.clear();
      loadImms.clear();
    }
    /*! Free everything we needed */
    virtual ~FunctionStructArgConstOffsetFolder() {
      for (size_t i = 0; i < records.size(); ++i) {
        delete records[i];
      }
      records.clear();
      loadImms.clear();
    }
    /*! Perform all function arguments substitution if needed */
    void folding(const std::string &name);

  private:
    class Record {  //add dst, arg (kernel struct arg base reg), imm_value
    public:
      Record(Register dst, Register arg, int64_t immv) :
                                        _dst(dst), _arg(arg), _immv(immv) { }
      Register _dst;
      Register _arg;
      int64_t _immv;
    };
    std::vector<Record*> records;
    std::map<Register, LoadImmInstruction*> loadImms; //<ir reg, load reg imm>

    void AddRecord(Register dst, Register arg, int64_t immv) {
      Record* rec = new Record(dst, arg, immv);
      records.push_back(rec);
    }
  };

  void FunctionStructArgConstOffsetFolder::folding(const std::string &name) {
    Function *fn = unit.getFunction(name);
    if (fn == NULL)
      return;

    const uint32_t argNum = fn->argNum();
    for (uint32_t argID = 0; argID < argNum; ++argID) {
      FunctionArgument &arg = fn->getArg(argID);
      if (arg.type != FunctionArgument::STRUCTURE)
        continue;
      AddRecord(arg.reg, arg.reg, 0);
    }

    fn->foreachInstruction([&](Instruction &insn) {
      if (insn.getOpcode() == OP_LOADI) {
        LoadImmInstruction *loadImm = cast<LoadImmInstruction>(&insn);
        if(!loadImm)
          return;

        //to avoid regression, limit for the case: LOADI.int64 %1164 32
        //we can loose the limit if necessary
        if (loadImm->getImmediate().getType() != TYPE_S64 &&
            loadImm->getImmediate().getType() != TYPE_U64)
          return;

        Register dst = insn.getDst();
        loadImms[dst] = loadImm;
        return;
      }

      //we will change imm of loadi directly, so it should not be dst
      for (size_t i = 0; i < insn.getDstNum(); ++i) {
        Register dst = insn.getDst(i);
        assert(loadImms.find(dst) == loadImms.end());
      }

      if (insn.getOpcode() != OP_ADD)
        return;

      Register src0 = insn.getSrc(0);
      Register src1 = insn.getSrc(1);
      Register dst = insn.getDst();

      //check if src0 is derived from kernel struct arg
      std::vector<Record*>::iterator it =
            std::find_if(records.begin(), records.end(), [=](Record* rec){
                                                            return rec->_dst == src0;
                                                            } );
      if (it == records.end())
        return;

      //check if src1 is imm value
      if (loadImms.find(src1) == loadImms.end())
        return;

      Record* rec = *it;
      LoadImmInstruction *loadImm = loadImms[src1];
      Immediate imm = loadImm->getImmediate();
      int64_t newvalue = imm.getIntegerValue() + rec->_immv;

      if (rec->_dst != rec->_arg) {  //directly dervied from arg if they are equal
        //change src0 to be the kernel struct arg
        insn.setSrc(0, rec->_arg);

        //change the value of src1
        ImmediateIndex immIndex = fn->newImmediate(newvalue);
        loadImm->setImmediateIndex(immIndex);
      }
      AddRecord(dst, rec->_arg, newvalue);
    });
  }

  void foldFunctionStructArgConstOffset(Unit &unit, const std::string &functionName) {
    FunctionStructArgConstOffsetFolder folder(unit);
    folder.folding(functionName);
  }

} /* namespace ir */
}
