
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

  class ReplaceInfo
  {
  public:
    ReplaceInfo(SelectionInstruction &insn,
                const GenRegister &intermedia,
                const GenRegister &replacement) : insn(insn), intermedia(intermedia), replacement(replacement)
    {
      assert(insn.opcode == SEL_OP_MOV || insn.opcode == SEL_OP_ADD);
      assert(&(insn.dst(0)) == &intermedia);
      this->elements = CalculateElements(intermedia, insn.state.execWidth);
      replacementOverwritten = false;
    }
    ~ReplaceInfo()
    {
      this->toBeReplaceds.clear();
    }

    SelectionInstruction &insn;
    const GenRegister &intermedia;
    uint32_t elements;
    const GenRegister &replacement;
    set<GenRegister *> toBeReplaceds;
    set<SelectionInstruction*> toBeReplacedInsns;
    bool replacementOverwritten;
    GBE_CLASS(ReplaceInfo);
  };

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

    typedef map<ir::Register, ReplaceInfo*> ReplaceInfoMap;
    ReplaceInfoMap replaceInfoMap;
    void doLocalCopyPropagation();
    void addToReplaceInfoMap(SelectionInstruction& insn);
    void changeInsideReplaceInfoMap(const SelectionInstruction& insn, GenRegister& var);
    void removeFromReplaceInfoMap(const SelectionInstruction& insn, const GenRegister& var);
    void doReplacement(ReplaceInfo* info);
    bool CanBeReplaced(const ReplaceInfo* info, const SelectionInstruction& insn, const GenRegister& var);
    void cleanReplaceInfoMap();
    void doNegAddOptimization(SelectionInstruction &insn);

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
    assert(insn.opcode == SEL_OP_MOV || insn.opcode == SEL_OP_ADD);
    GenRegister &src = insn.src(0);
    if (insn.opcode == SEL_OP_ADD) {
      if (src.file == GEN_IMMEDIATE_VALUE)
        src = insn.src(1);
    }

    const GenRegister& dst = insn.dst(0);
    if (src.type != dst.type || src.file != dst.file)
      return;

    if (src.hstride != GEN_HORIZONTAL_STRIDE_0 && src.hstride != dst.hstride )
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

    //the src modifier is not supported by the following instructions
    if(info->replacement.negation || info->replacement.absolute)
    {
      switch(insn.opcode)
      {
        case SEL_OP_MATH:
        {
          switch(insn.extra.function)
          {
            case GEN_MATH_FUNCTION_INT_DIV_QUOTIENT:
            case GEN_MATH_FUNCTION_INT_DIV_REMAINDER:
            case GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER:
              return false;
            default:
              break;
          }

          break;
        }
        case SEL_OP_CBIT:
        case SEL_OP_FBH:
        case SEL_OP_FBL:
        case SEL_OP_BRC:
        case SEL_OP_BRD:
        case SEL_OP_BFREV:
        case SEL_OP_LZD:
        case SEL_OP_HADD:
        case SEL_OP_RHADD:
          return false;
        default:
          break;
      }
    }

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

      doNegAddOptimization(insn);
    }
    cleanReplaceInfoMap();
  }

  /* LLVM transform Mad(a, -b, c) to
     Add b, -b, 0
     Mad val, a, b, c
     for Gen support negtive modifier, mad(a, -b, c) is native suppoted.
     Also it can be used for the same like instruction sequence.
     Do it just like a:  mov b, -b, so it is a Mov operation like LocalCopyPropagation
  */
  void SelBasicBlockOptimizer::doNegAddOptimization(SelectionInstruction &insn) {
    if (insn.opcode == SEL_OP_ADD) {
      GenRegister src0 = insn.src(0);
      GenRegister src1 = insn.src(1);
      if ((src0.negation && src1.file == GEN_IMMEDIATE_VALUE && src1.value.f == 0.0f) ||
          (src1.negation && src0.file == GEN_IMMEDIATE_VALUE && src0.value.f == 0.0f))
        addToReplaceInfoMap(insn);
    }
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

  class SelGlobalImmMovOpt : public SelGlobalOptimizer
  {
  public:
    SelGlobalImmMovOpt(const GenContext& ctx, uint32_t features, intrusive_list<SelectionBlock> *blockList) :
      SelGlobalOptimizer(ctx, features)
      {
        mblockList = blockList;
      }

    virtual void run();

    void addToReplaceInfoMap(SelectionInstruction& insn);
    void doGlobalCopyPropagation();
    bool CanBeReplaced(const ReplaceInfo* info, SelectionInstruction& insn, const GenRegister& var);
    void cleanReplaceInfoMap();
    void doReplacement(ReplaceInfo* info);

  private:
    intrusive_list<SelectionBlock> *mblockList;

    typedef map<ir::Register, ReplaceInfo*> ReplaceInfoMap;
    ReplaceInfoMap replaceInfoMap;

  };

  extern void outputSelectionInst(SelectionInstruction &insn);

  void SelGlobalImmMovOpt::cleanReplaceInfoMap()
  {
    for (auto& pair : replaceInfoMap) {
      ReplaceInfo* info = pair.second;
      doReplacement(info);
      delete info;
    }
    replaceInfoMap.clear();
  }

#define RESULT() switch(insn->opcode) \
            { \
              case SEL_OP_ADD: \
                result = s0 + s1; \
                break; \
              case SEL_OP_MUL: \
                result = s0 * s1; \
                break; \
              case SEL_OP_AND: \
                result = s0 & s1; \
                break; \
              case SEL_OP_OR: \
                result = s0 | s1; \
                break; \
              case SEL_OP_XOR: \
                result = s0 ^ s1; \
                break; \
              default: \
                assert(0); \
                break; \
            }

  void SelGlobalImmMovOpt::doReplacement(ReplaceInfo* info)
  {
    for (GenRegister* reg : info->toBeReplaceds) {
      GenRegister::propagateRegister(*reg, info->replacement);
    }

    //after imm opt, maybe both src are imm, convert it to mov
    for (SelectionInstruction* insn : info->toBeReplacedInsns) {
      GenRegister& src0 = insn->src(0);
      GenRegister& src1 = insn->src(1);
      if (src0.file == GEN_IMMEDIATE_VALUE)
      {
        if (src1.file == GEN_IMMEDIATE_VALUE)
        {
          if (src0.type == GEN_TYPE_F)
          {
            float s0 = src0.value.f;
            if (src0.absolute)
              s0 = fabs(s0);
            if (src0.negation)
              s0 = -s0;

            float s1 = src1.value.f;
            if (src1.absolute)
              s1 = fabs(s1);
            if (src1.negation)
              s1 = -s1;

            float result;
            switch(insn->opcode)
            {
              case SEL_OP_ADD:
                result = s0 + s1;
                break;
              case SEL_OP_MUL:
                result = s0 * s1;
                break;
              default:
                assert(0);
                break;
            }

            insn->src(0) = GenRegister::immf(result);
          }
          else if (src0.type == GEN_TYPE_D && src1.type == GEN_TYPE_D)
          {
            int s0 = src0.value.d;
            if (src0.absolute)
              s0 = abs(s0);
            if (src0.negation)
              s0 = -s0;

            int s1 = src1.value.d;
            if (src1.absolute)
              s1 = abs(s1);
            if (src1.negation)
              s1 = -s1;

            int result;
            RESULT();
            insn->src(0) = GenRegister::immd(result);
            }
            else if(src0.type == GEN_TYPE_UD || src1.type == GEN_TYPE_UD)
            {
              unsigned int s0 = src0.value.ud;
              if (src0.absolute)
                s0 = abs(s0);
              if (src0.negation)
                s0 = -s0;

              unsigned int s1 = src1.value.ud;
              if (src1.absolute)
                s1 = abs(s1);
              if (src1.negation)
                s1 = -s1;

              unsigned int result;
              RESULT();
              insn->src(0) = GenRegister::immud(result);
            }
            else
            {
              assert(0);
            }

            insn->opcode = SEL_OP_MOV;
            insn->srcNum = 1;
        }
        else
        {
          //src0 cant be immediate, so exchange with src1
          GenRegister tmp;
          tmp = src0;
          src0 = src1;
          src1 = tmp;
        }
      }
    }

    info->insn.parent->insnList.erase(&(info->insn));
  }

  void SelGlobalImmMovOpt::addToReplaceInfoMap(SelectionInstruction& insn)
  {
    assert(insn.opcode == SEL_OP_MOV);
    const GenRegister& src = insn.src(0);
    const GenRegister& dst = insn.dst(0);
    if (src.type != dst.type)
      return;

    ReplaceInfo* info = new ReplaceInfo(insn, dst, src);
    replaceInfoMap[dst.reg()] = info;
  }

  bool SelGlobalImmMovOpt::CanBeReplaced(const ReplaceInfo* info, SelectionInstruction& insn, const GenRegister& var)
  {
    if(var.file == GEN_IMMEDIATE_VALUE)
      return false;

    switch(insn.opcode)
    {
      // imm source is supported by these instructions. And
      // the src operators can be exchange in these instructions
      case SEL_OP_ADD:
      case SEL_OP_MUL:
      case SEL_OP_AND:
      case SEL_OP_OR:
      case SEL_OP_XOR:
        break;
      default:
        return false;
    }

    if(info->intermedia.type != var.type)
    {
        assert(info->replacement.file == GEN_IMMEDIATE_VALUE);
        if(!((var.type == GEN_TYPE_D && info->intermedia.type == GEN_TYPE_UD)
          || (var.type == GEN_TYPE_UD && info->intermedia.type == GEN_TYPE_D)))
        {
          return false;
        }
    }

    if (info->intermedia.quarter == var.quarter &&
          info->intermedia.subnr == var.subnr &&
            info->intermedia.nr == var.nr)
    {
      uint32_t elements = CalculateElements(var, insn.state.execWidth);  //considering width, hstrid, vstrid and execWidth
      if (info->elements != elements)
        return false;
    }

#ifdef DEBUG_GLOBAL_IMM_OPT
    outputSelectionInst(insn);
#endif

    return true;
  }

  void SelGlobalImmMovOpt::doGlobalCopyPropagation()
  {
    for(SelectionBlock &block : *mblockList)
    {
      for (SelectionInstruction &insn :block.insnList)
      {
        for (uint8_t i = 0; i < insn.srcNum; ++i)
        {
            ReplaceInfoMap::iterator it = replaceInfoMap.find(insn.src(i).reg());
            if (it != replaceInfoMap.end())
            {
              ReplaceInfo *info = it->second;
              if (CanBeReplaced(info, insn, insn.src(i)))
              {
                info->toBeReplaceds.insert(&insn.src(i));
                info->toBeReplacedInsns.insert(&insn);
              }
              else
              {
                replaceInfoMap.erase(it);
                delete info;
              }
            }
        }

        for (uint8_t i = 0; i < insn.dstNum; ++i)
        {
          ReplaceInfoMap::iterator it = replaceInfoMap.find(insn.dst(i).reg());
          if (it != replaceInfoMap.end())
          {
            ReplaceInfo *info = it->second;
            if(&(info->insn) != &insn)
            {
              replaceInfoMap.erase(it);
              delete info;
            }
          }
        }
      }

      if(replaceInfoMap.empty())
        break;
    }

    cleanReplaceInfoMap();
  }

  void SelGlobalImmMovOpt::run()
  {
    bool canbeOpt = false;

    /*global immediates are set in entry block, the following is example of GEN IR

          decl_input.global %41 dst
            ## 0 output register ##
            ## 0 pushed register
            ## 3 blocks ##
          LABEL $0
            LOADI.uint32 %42 0
            LOADI.uint32 %43 48

          LABEL $1
            MUL.int32 %44 %3   %12
            ADD.int32 %49 %42 %48
            ...
    */
    SelectionBlock &block = *mblockList->begin();
    for(SelectionInstruction &insn : block.insnList)
    {
        GenRegister src0 = insn.src(0);
        if(insn.opcode == SEL_OP_MOV &&
            src0.file == GEN_IMMEDIATE_VALUE &&
            (src0.type == GEN_TYPE_UD || src0.type == GEN_TYPE_D || src0.type == GEN_TYPE_F))
        {
          addToReplaceInfoMap(insn);
          canbeOpt = true;

#ifdef DEBUG_GLOBAL_IMM_OPT
          outputSelectionInst(insn);
#endif
        }
    }

    if(!canbeOpt) return;

    doGlobalCopyPropagation();
  }

  void SelGlobalOptimizer::run()
  {

  }

  BVAR(OCL_GLOBAL_IMM_OPTIMIZATION, true);

  void Selection::optimize()
  {
    //do global imm opt first to make more local opt
    if(OCL_GLOBAL_IMM_OPTIMIZATION)
    {
      SelGlobalImmMovOpt gopt(getCtx(), opt_features, blockList);
      gopt.run();
    }

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
