
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_context.hpp"
#include "ir/function.hpp"
#include "ir/liveness.hpp"
#include "ir/profile.hpp"
#include "sys/cvar.hpp"
#include "sys/vector.hpp"
#include <algorithm>
#include <climits>
#include <map>

namespace gbe
{
  class IfOptimizer
  {
  public:
    IfOptimizer(const GenContext& ctx, SelectionBlock& selblock) : ctx(ctx), selBlock(selblock) {}
    void run();
    bool isSimpleBlock();
    void removeSimpleIfEndif();
    ~IfOptimizer() {}
  protected:
    const GenContext &ctx;      //in case that we need it
    SelectionBlock &selBlock;
    bool optimized;
  };

  bool IfOptimizer::isSimpleBlock() {

      if(selBlock.insnList.size() > 20)
          return false;

      bool if_exits = false;
      bool endif_exits = false;
      for (auto &insn : selBlock.insnList) {
        if (insn.opcode == SEL_OP_IF) {
          if_exits = true;
          continue;
        }
        if(if_exits) {
          GenInstructionState curr = insn.state;
          if (curr.execWidth == 1 || curr.predicate != GEN_PREDICATE_NONE || curr.flagIndex != 0 || (curr.flag == 0 && curr.subFlag == 1)) {
            return false;
          }

          if (insn.opcode == SEL_OP_ELSE) {
            return false;
          }

          if (insn.opcode == SEL_OP_SEL_CMP) {
            return false;
          }
        }

        if (insn.opcode == SEL_OP_ENDIF) {
            endif_exits = true;
          break;
        }
      }

      if (!if_exits || !endif_exits)
        return false;

      return true;
  }

  void IfOptimizer::removeSimpleIfEndif() {
      if(isSimpleBlock()) {
          GenInstructionState curr;
          bool if_find = false;
          for (auto iter = selBlock.insnList.begin(); iter != selBlock.insnList.end(); ) {
            // remove if and endif, change instruction flags.
            SelectionInstruction &insn = *iter;
            if (insn.opcode == SEL_OP_IF && !if_find) {
              iter = selBlock.insnList.erase(&insn);
              if_find = true;
            } else if (insn.opcode == SEL_OP_ENDIF) {
              iter = selBlock.insnList.erase(&insn);
              optimized = true;
            } else {
              if (if_find) {
                insn.state.predicate = GEN_PREDICATE_NORMAL;
                insn.state.flag = 0;
                insn.state.subFlag = 1;
              }
              ++iter;
            }
          }
      }
  }

  void IfOptimizer::run()
  {
      optimized = false;
      removeSimpleIfEndif();
  }

  void Selection::if_opt()
  {
    //do basic block level optimization
    for (SelectionBlock &block : *blockList) {
      IfOptimizer ifopt(getCtx(), block);
      ifopt.run();
    }

  }
} /* namespace gbe */

