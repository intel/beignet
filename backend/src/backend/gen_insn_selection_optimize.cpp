
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
  //helper functions
  static uint32_t CalculateElements(const GenRegister& reg, uint32_t execWidth)
  {
    uint32_t elements = 0;
    uint32_t elementSize = typeSize(reg.type);
    uint32_t width = GenRegister::width_size(reg);
    // reg may be other insn's source, this insn's width don't force large then execWidth.
    //assert(execWidth >= width);
    uint32_t height = execWidth / width;
    uint32_t vstride = GenRegister::vstride_size(reg);
    uint32_t hstride = GenRegister::hstride_size(reg);
    uint32_t base = reg.nr * GEN_REG_SIZE + reg.subnr;
    for (uint32_t i = 0; i < height; ++i) {
      uint32_t offsetInByte = base;
      for (uint32_t j = 0; j < width; ++j) {
        uint32_t offsetInType = offsetInByte / elementSize;
        //it is possible that offsetInType > 32, it doesn't matter even elements is 32 bit.
        //the reseason is that if one instruction span several registers,
        //the other registers' visit pattern is same as first register if the vstride is normal(width * hstride)
        assert(vstride == width * hstride);
        elements |= (1 << offsetInType);
        offsetInByte += hstride * elementSize;
      }
      base += vstride * elementSize;
    }
    return elements;
  }

  class SelOptimizer
  {
  public:
    SelOptimizer(const GenContext& ctx, uint32_t features) : ctx(ctx), features(features) {}
    virtual void run() = 0;
    virtual ~SelOptimizer() {}
  protected:
    const GenContext &ctx;      //in case that we need it
    uint32_t features;
  };

  class SelBasicBlockOptimizer : public SelOptimizer
  {
  public:
    SelBasicBlockOptimizer(const GenContext& ctx,
                           const ir::Liveness::LiveOut& liveout,
                           uint32_t features,
                           SelectionBlock &bb) :
        SelOptimizer(ctx, features), bb(bb), liveout(liveout), optimized(false)
    {
    }
    ~SelBasicBlockOptimizer() {}
    virtual void run();

  private:
    // local copy propagation
    class ReplaceInfo
    {
    public:
      ReplaceInfo(SelectionInstruction& insn,
                  const GenRegister& intermedia,
                  const GenRegister& replacement) :
                  insn(insn), intermedia(intermedia), replacement(replacement)
      {
        assert(insn.opcode == SEL_OP_MOV);
        assert(&(insn.src(0)) == &replacement);
        assert(&(insn.dst(0)) == &intermedia);
        this->elements = CalculateElements(intermedia, insn.state.execWidth);
        replacementOverwritten = false;
      }
      ~ReplaceInfo()
      {
        this->toBeReplaceds.clear();
      }

      SelectionInstruction& insn;
      const GenRegister& intermedia;
      uint32_t elements;
      const GenRegister& replacement;
      set<GenRegister*> toBeReplaceds;
      bool replacementOverwritten;
      GBE_CLASS(ReplaceInfo);
    };
    typedef map<ir::Register, ReplaceInfo*> ReplaceInfoMap;
    ReplaceInfoMap replaceInfoMap;
    void doLocalCopyPropagation();
    void addToReplaceInfoMap(SelectionInstruction& insn);
    void changeInsideReplaceInfoMap(const SelectionInstruction& insn, GenRegister& var);
    void removeFromReplaceInfoMap(const SelectionInstruction& insn, const GenRegister& var);
    void doReplacement(ReplaceInfo* info);
    bool CanBeReplaced(const ReplaceInfo* info, const SelectionInstruction& insn, const GenRegister& var);
    void cleanReplaceInfoMap();

    SelectionBlock &bb;
    const ir::Liveness::LiveOut& liveout;
    bool optimized;
    static const size_t MaxTries = 1;   //the max times of optimization try
  };

  void SelBasicBlockOptimizer::doReplacement(ReplaceInfo* info)
  {
    for (GenRegister* reg : info->toBeReplaceds) {
      GenRegister::propagateRegister(*reg, info->replacement);
    }
    bb.insnList.erase(&(info->insn));
    optimized = true;
  }

  void SelBasicBlockOptimizer::cleanReplaceInfoMap()
  {
    for (auto& pair : replaceInfoMap) {
      ReplaceInfo* info = pair.second;
      doReplacement(info);
      delete info;
    }
    replaceInfoMap.clear();
  }

  void SelBasicBlockOptimizer::removeFromReplaceInfoMap(const SelectionInstruction& insn, const GenRegister& var)
  {
    for (ReplaceInfoMap::iterator pos = replaceInfoMap.begin(); pos != replaceInfoMap.end(); ++pos) {
      ReplaceInfo* info = pos->second;
      if (info->intermedia.reg() == var.reg()) {   //intermedia is overwritten
        if (info->intermedia.quarter == var.quarter && info->intermedia.subnr == var.subnr && info->intermedia.nr == var.nr) {
          // We need to check the if intermedia is fully overwritten, they may be in some prediction state.
          if (CanBeReplaced(info, insn, var))
            doReplacement(info);
        }
        replaceInfoMap.erase(pos);
        delete info;
        return;
      }
      if (info->replacement.reg() == var.reg()) {  //replacement is overwritten
        //there could be more than one replacements (with different physical subnr) overwritten,
        //so do not break here, need to scann the whole map.
        //here is an example:
        // mov %10, %9.0
        // mov %11, %9.1
        // ...
        // mov %9, %8
        //both %9.0 and %9.1 are collected into replacement in the ReplaceInfoMap after the first two insts are scanned.
        //when scan the last inst that %9 is overwritten, we should flag both %9.0 and %9.1 in the map.
        info->replacementOverwritten = true;
      }
    }
  }

  void SelBasicBlockOptimizer::addToReplaceInfoMap(SelectionInstruction& insn)
  {
    assert(insn.opcode == SEL_OP_MOV);
    const GenRegister& src = insn.src(0);
    const GenRegister& dst = insn.dst(0);
    if (src.type != dst.type || src.file != dst.file || src.hstride != dst.hstride)
      return;

    if (liveout.find(dst.reg()) != liveout.end())
      return;

    ReplaceInfo* info = new ReplaceInfo(insn, dst, src);
    replaceInfoMap[dst.reg()] = info;
  }

  bool SelBasicBlockOptimizer::CanBeReplaced(const ReplaceInfo* info, const SelectionInstruction& insn, const GenRegister& var)
  {
    //some conditions here are very strict, while some conditions are very light
    //the reason is that i'm unable to find a perfect condition now in the first version
    //need to refine the conditions when debugging/optimizing real kernels

    if (insn.opcode == SEL_OP_BSWAP) //should remove once bswap issue is fixed
      return false;

    if (insn.isWrite() || insn.isRead()) //register in selection vector
      return false;

    if (features & SIOF_LOGICAL_SRCMOD)
      if ((insn.opcode == SEL_OP_AND || insn.opcode == SEL_OP_NOT || insn.opcode == SEL_OP_OR || insn.opcode == SEL_OP_XOR) &&
            (info->replacement.absolute || info->replacement.negation))
        return false;

    if (features & SIOF_OP_MOV_LONG_REG_RESTRICT && insn.opcode == SEL_OP_MOV) {
      const GenRegister& dst = insn.dst(0);
      if (dst.isint64() && !info->replacement.isint64() && info->elements != CalculateElements(info->replacement, insn.state.execWidth))
        return false;
    }

    if (info->replacementOverwritten)
      return false;

    if (info->insn.state.noMask == 0 && insn.state.noMask == 1)
      return false;

    // If insn is in no prediction state, it will overwrite the info insn.
    if (info->insn.state.predicate != insn.state.predicate && insn.state.predicate != GEN_PREDICATE_NONE)
      return false;

    if (info->insn.state.inversePredicate != insn.state.inversePredicate)
      return false;

    if (info->intermedia.type == var.type && info->intermedia.quarter == var.quarter &&
        info->intermedia.subnr == var.subnr && info->intermedia.nr == var.nr) {
      uint32_t elements = CalculateElements(var, insn.state.execWidth);  //considering width, hstrid, vstrid and execWidth
      if (info->elements == elements)
        return true;
    }

    return false;
  }

  void SelBasicBlockOptimizer::changeInsideReplaceInfoMap(const SelectionInstruction& insn, GenRegister& var)
  {
    ReplaceInfoMap::iterator it = replaceInfoMap.find(var.reg());
    if (it != replaceInfoMap.end()) {    //same ir register
      ReplaceInfo* info = it->second;
      if (CanBeReplaced(info, insn, var)) {
        info->toBeReplaceds.insert(&var);
      } else {
        //if it is the same ir register, but could not be replaced for some reason,
        //that means we could not remove MOV instruction, and so no replacement,
        //so we'll remove the info for this case.
        replaceInfoMap.erase(it);
        delete info;
      }
    }
  }

  void SelBasicBlockOptimizer::doLocalCopyPropagation()
  {
    for (SelectionInstruction &insn : bb.insnList) {
      for (uint8_t i = 0; i < insn.srcNum; ++i)
        changeInsideReplaceInfoMap(insn, insn.src(i));

      for (uint8_t i = 0; i < insn.dstNum; ++i)
        removeFromReplaceInfoMap(insn, insn.dst(i));

      if (insn.opcode == SEL_OP_MOV)
        addToReplaceInfoMap(insn);
    }
    cleanReplaceInfoMap();
  }

  void SelBasicBlockOptimizer::run()
  {
    for (size_t i = 0; i < MaxTries; ++i) {
      optimized = false;

      doLocalCopyPropagation();
      //doOtherLocalOptimization();

      if (!optimized)
        break;      //break since no optimization found at this round
    }
  }

  class SelGlobalOptimizer : public SelOptimizer
  {
  public:
    SelGlobalOptimizer(const GenContext& ctx, uint32_t features) : SelOptimizer(ctx, features) {}
    ~SelGlobalOptimizer() {}
    virtual void run();
  };

  void SelGlobalOptimizer::run()
  {

  }

  void Selection::optimize()
  {
    //do basic block level optimization
    for (SelectionBlock &block : *blockList) {
      SelBasicBlockOptimizer bbopt(getCtx(), getCtx().getLiveOut(block.bb), opt_features, block);
      bbopt.run();
    }

    //do global optimization

  }

  void Selection::addID()
  {
    uint32_t insnID = 0;
    for (auto &block : *blockList)
      for (auto &insn : block.insnList) {
        insn.ID  = insnID;
        insnID += 2;
      }
  }
} /* namespace gbe */
