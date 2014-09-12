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
 * \file function.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_FUNCTION_HPP__
#define __GBE_IR_FUNCTION_HPP__

#include "ir/immediate.hpp"
#include "ir/register.hpp"
#include "ir/instruction.hpp"
#include "ir/profile.hpp"
#include "ir/sampler.hpp"
#include "ir/printf.hpp"
#include "ir/image.hpp"
#include "sys/vector.hpp"
#include "sys/set.hpp"
#include "sys/map.hpp"
#include "sys/alloc.hpp"

#include <ostream>

namespace gbe {
namespace ir {

  /*! Commonly used in the CFG */
  typedef set<BasicBlock*> BlockSet;
  class Unit; // Function belongs to a unit

  /*! Function basic blocks really belong to a function since:
   *  1 - registers used in the basic blocks belongs to the function register
   *      file
   *  2 - branches point to basic blocks of the same function
   */
  class BasicBlock : public NonCopyable, public intrusive_list<Instruction>
  {
  public:
    /*! Empty basic block */
    BasicBlock(Function &fn);
    /*! Releases all the instructions */
    ~BasicBlock(void);
    /*! Append a new instruction at the end of the stream */
    void append(Instruction &insn);
    /*! Get the parent function */
    Function &getParent(void) { return fn; }
    const Function &getParent(void) const { return fn; }
    /*! Get the next and previous allocated block */
    BasicBlock *getNextBlock(void) const { return this->nextBlock; }
    BasicBlock *getPrevBlock(void) const { return this->prevBlock; }
    /*! Get / set the first and last instructions */
    Instruction *getFirstInstruction(void) const;
    Instruction *getLastInstruction(void) const;
    /*! Get successors and predecessors */
    const BlockSet &getSuccessorSet(void) const { return successors; }
    const BlockSet &getPredecessorSet(void) const { return predecessors; }
    /*! Get the label index of this block */
    LabelIndex getLabelIndex(void) const;
    /*! Apply the given functor on all instructions */
    template <typename T>
    INLINE void foreach(const T &functor) {
      auto it = this->begin();
      while (it != this->end()) {
        auto curr = it++;
        functor(*curr);
      }
    }
    set <Register> undefPhiRegs;
    set <Register> definedPhiRegs;
  private:
    friend class Function; //!< Owns the basic blocks
    BlockSet predecessors; //!< Incoming blocks
    BlockSet successors;   //!< Outgoing blocks
    BasicBlock *nextBlock; //!< Block allocated just after this one
    BasicBlock *prevBlock; //!< Block allocated just before this one
    Function &fn;          //!< Function the block belongs to
    GBE_CLASS(BasicBlock);
  };

  /*! In fine, function input arguments can be pushed from the constant
   *  buffer if they are structures. Other arguments can be images (textures)
   *  and will also require special treatment.
   */
  struct FunctionArgument {
    enum Type {
      GLOBAL_POINTER    = 0, // __global
      CONSTANT_POINTER  = 1, // __constant
      LOCAL_POINTER     = 2, // __local
      VALUE             = 3, // int, float
      STRUCTURE         = 4, // struct foo
      IMAGE             = 5,  // image*d_t
      SAMPLER           = 6
    };

    struct InfoFromLLVM { // All the info about passed by llvm, using -cl-kernel-arg-info
      uint32_t addrSpace;
      std::string typeName;
      std::string accessQual;
      std::string typeQual;
      std::string argName; // My different from arg->getName()
    };

    /*! Create a function input argument */
    INLINE FunctionArgument(Type type, Register reg, uint32_t size, const std::string &name, uint32_t align, InfoFromLLVM& info, uint8_t bti) :
      type(type), reg(reg), size(size), align(align), name(name), info(info), bti(bti) { }

    Type type;     //!< Gives the type of argument we have
    Register reg;  //!< Holds the argument
    uint32_t size; //!< == sizeof(void*) for ptr, sizeof(elem) for the rest
    uint32_t align; //!< address alignment for the argument
    const std::string name; //!< Holds the function name for IR output
    InfoFromLLVM info;  //!< Holds the llvm passed info
    uint8_t bti; //!< binding table index
    GBE_STRUCT(FunctionArgument); // Use custom allocator
  };

  /*! Maps the pushed register to the function argument */
  struct PushLocation {
    INLINE PushLocation(const Function &fn, uint32_t argID, uint32_t offset) :
      fn(fn), argID(argID), offset(offset) {}
    /*! Get the pushed virtual register */
    Register getRegister(void) const;
    const Function &fn;       //!< Function it belongs to
    uint32_t argID;           //!< Function argument
    uint32_t offset;          //!< Offset in the function argument
    GBE_STRUCT(PushLocation); // Use custom allocator
  };

  /*! For maps and sets */
  INLINE bool operator< (const PushLocation &arg0, const PushLocation &arg1) {
    if (arg0.argID != arg1.argID) return arg0.argID < arg1.argID;
    return arg0.offset < arg1.offset;
  }

  /*! CFG loops */
  struct Loop : public NonCopyable
  {
  public:
    Loop(const vector<LabelIndex> &in, const vector<std::pair<LabelIndex, LabelIndex>> &exit) :
    bbs(in), exits(exit) {}
    vector<LabelIndex> bbs;
    vector<std::pair<LabelIndex, LabelIndex>> exits;
    GBE_STRUCT(Loop);
  };

  /*! A function is :
   *  - a register file
   *  - a set of basic block layout into a CGF
   *  - input arguments
   */
  class Function : public NonCopyable
  {
  public:
    /*! Map of all pushed registers */
    typedef map<Register, PushLocation> PushMap;
    /*! Map of all pushed location (i.e. part of function argument) */
    typedef map<PushLocation, Register> LocationMap;
    /*! Create an empty function */
    Function(const std::string &name, const Unit &unit, Profile profile = PROFILE_OCL);
    /*! Release everything *including* the basic block pointers */
    ~Function(void);
    /*! Get the function profile */
    INLINE Profile getProfile(void) const { return profile; }
    /*! Get a new valid register */
    INLINE Register newRegister(RegisterFamily family, bool uniform = false) {
      return this->file.append(family, uniform);
    }
    /*! Get the function name */
    const std::string &getName(void) const { return name; }
    /*! When set, we do not have choice any more in the back end for it */
    INLINE void setSimdWidth(uint32_t width) { simdWidth = width; }
    /*! Get the SIMD width (0 if not forced) */
    uint32_t getSimdWidth(void) const { return simdWidth; }
    /*! Extract the register from the register file */
    INLINE RegisterData getRegisterData(Register reg) const { return file.get(reg); }
    /*! set a register to uniform or nonuniform type. */
    INLINE void setRegisterUniform(Register reg, bool uniform) { file.setUniform(reg, uniform); }
    /*! return true if the specified regsiter is uniform type */
    INLINE bool isUniformRegister(Register reg) { return file.isUniform(reg); }
    /*! Get the register family from the register itself */
    INLINE RegisterFamily getRegisterFamily(Register reg) const {
      return this->getRegisterData(reg).family;
    }
    /*! Get the register from the tuple vector */
    INLINE Register getRegister(Tuple ID, uint32_t which) const {
      return file.get(ID, which);
    }
    /*! Set the register from the tuple vector */
    INLINE void setRegister(Tuple ID, uint32_t which, Register reg) {
      file.set(ID, which, reg);
    }
    /*! Get the register file */
    INLINE const RegisterFile &getRegisterFile(void) const { return file; }
    /*! Get the given value ie immediate from the function */
    INLINE const Immediate &getImmediate(ImmediateIndex ID) const {
      return immediates[ID];
    }
    /*! Create a new immediate and returns its index */
    INLINE ImmediateIndex newImmediate(const Immediate &imm) {
      const ImmediateIndex index(this->immediateNum());
      this->immediates.push_back(imm);
      return index;
    }
    /*! Fast allocation / deallocation of instructions */
    DECL_POOL(Instruction, insnPool);
    /*! Get input argument */
    INLINE const FunctionArgument &getArg(uint32_t ID) const {
      GBE_ASSERT(args[ID] != NULL);
      return *args[ID];
    }
    INLINE FunctionArgument &getArg(uint32_t ID) {
      GBE_ASSERT(args[ID] != NULL);
      return *args[ID];
    }

    /*! Get arg ID. */
    INLINE int32_t getArgID(FunctionArgument *requestArg) {
      for (uint32_t ID = 0; ID < args.size(); ID++)
      {
        if ( args[ID] == requestArg )
          return ID;
      }
      GBE_ASSERTM(0, "Failed to get a valid argument ID.");
      return -1;
    }

    /*! Get the number of pushed registers */
    INLINE uint32_t pushedNum(void) const { return pushMap.size(); }
    /*! Get the pushed data location for the given register */
    INLINE const PushLocation *getPushLocation(Register reg) const {
      auto it = pushMap.find(reg);
      if (it == pushMap.end())
        return NULL;
      else
        return &it->second;
    }
    /*! Get the map of pushed registers */
    const PushMap &getPushMap(void) const { return this->pushMap; }
    /*! Get the map of pushed registers */
    const LocationMap &getLocationMap(void) const { return this->locationMap; }
    /*! Get input argument from the register (linear research). Return NULL if
     *  this is not an input argument
     */
    INLINE const FunctionArgument *getArg(const Register &reg) const {
      for (auto arg : args) if (arg->reg == reg) return arg;
      return NULL;
    }

    INLINE FunctionArgument *getArg(const Register &reg) {
      for (auto arg : args) if (arg->reg == reg) return arg;
      return NULL;
    }

    /*! Get output register */
    INLINE Register getOutput(uint32_t ID) const { return outputs[ID]; }
    /*! Get the argument location for the pushed register */
    INLINE const PushLocation &getPushLocation(Register reg) {
      GBE_ASSERT(pushMap.contains(reg) == true);
      return pushMap.find(reg)->second;
    }
    /*! Says if this is the top basic block (entry point) */
    bool isEntryBlock(const BasicBlock &bb) const;
    /*! Get function the entry point block */
    const BasicBlock &getTopBlock(void) const;
    /*! Get the last block */
    const BasicBlock &getBottomBlock(void) const;
    /*! Get the last block */
    BasicBlock &getBottomBlock(void);
    /*! Get block from its label */
    const BasicBlock &getBlock(LabelIndex label) const;
    /*! Get the label instruction from its label index */
    const LabelInstruction *getLabelInstruction(LabelIndex index) const;
    /*! Return the number of instructions of the largest basic block */
    uint32_t getLargestBlockSize(void) const;
    /*! Get the first index of the special registers and number of them */
    uint32_t getFirstSpecialReg(void) const;
    uint32_t getSpecialRegNum(void) const;
    /*! Indicate if the given register is a special one (like localID in OCL) */
    bool isSpecialReg(const Register &reg) const;
    /*! Create a new label (still not bound to a basic block) */
    LabelIndex newLabel(void);
    /*! Create the control flow graph */
    void computeCFG(void);
    /*! Sort labels in increasing orders (top block has the smallest label) */
    void sortLabels(void);
    /*! check empty Label. */
    void checkEmptyLabels(void);
    /*! Get the pointer family */
    RegisterFamily getPointerFamily(void) const;
    /*! Number of registers in the register file */
    INLINE uint32_t regNum(void) const { return file.regNum(); }
    /*! Number of register tuples in the register file */
    INLINE uint32_t tupleNum(void) const { return file.tupleNum(); }
    /*! Number of labels in the function */
    INLINE uint32_t labelNum(void) const { return labels.size(); }
    /*! Number of immediate values in the function */
    INLINE uint32_t immediateNum(void) const { return immediates.size(); }
    /*! Get the number of argument register */
    INLINE uint32_t argNum(void) const { return args.size(); }
    /*! Get the number of output register */
    INLINE uint32_t outputNum(void) const { return outputs.size(); }
    /*! Number of blocks in the function */
    INLINE uint32_t blockNum(void) const { return blocks.size(); }
    /*! Output an immediate value in a stream */
    void outImmediate(std::ostream &out, ImmediateIndex index) const;
    /*! Apply the given functor on all basic blocks */
    template <typename T>
    INLINE void foreachBlock(const T &functor) const {
      for (auto block : blocks) functor(*block);
    }
    /*! Apply the given functor on all instructions */
    template <typename T>
    INLINE void foreachInstruction(const T &functor) const {
      for (auto block : blocks) block->foreach(functor);
    }
    /*! Does it use SLM */
    INLINE bool getUseSLM(void) const { return this->useSLM; }
    /*! Change the SLM config for the function */
    INLINE bool setUseSLM(bool useSLM) { return this->useSLM = useSLM; }
    /*! get SLM size needed for local variable inside kernel function */
    INLINE uint32_t getSLMSize(void) const { return this->slmSize; }
    /*! set slm size needed for local variable inside kernel function */
    INLINE void setSLMSize(uint32_t size) { this->slmSize = size; }
    /*! Get sampler set in this function */
    SamplerSet* getSamplerSet(void) const {return samplerSet; }
    /*! Get image set in this function */
    ImageSet* getImageSet(void) const {return imageSet; }
    /*! Get printf set in this function */
    PrintfSet* getPrintfSet(void) const {return printfSet; }
    /*! Set required work group size. */
    void setCompileWorkGroupSize(size_t x, size_t y, size_t z) { compileWgSize[0] = x; compileWgSize[1] = y; compileWgSize[2] = z; }
    /*! Get required work group size. */
    const size_t *getCompileWorkGroupSize(void) const {return compileWgSize;}
    /*! Set function attributes string. */
    void setFunctionAttributes(const std::string& functionAttributes) {  this->functionAttributes= functionAttributes; }
    /*! Get function attributes string. */
    const std::string& getFunctionAttributes(void) const {return this->functionAttributes;}
    /*! Get stack size. */
    INLINE const uint32_t getStackSize(void) const { return this->stackSize; }
    /*! Push stack size. */
    INLINE void pushStackSize(uint32_t step) { this->stackSize += step; }
    /*! add the loop info for later liveness analysis */
    void addLoop(const vector<LabelIndex> &bbs, const vector<std::pair<LabelIndex, LabelIndex>> &exits);
    INLINE const vector<Loop * > &getLoops() { return loops; }
    /*! Get surface starting address register from bti */
    Register getSurfaceBaseReg(uint8_t bti) const;
    void appendSurface(uint8_t bti, Register reg);
  private:
    friend class Context;           //!< Can freely modify a function
    std::string name;               //!< Function name
    const Unit &unit;               //!< Function belongs to this unit
    vector<FunctionArgument*> args; //!< Input registers of the function
    vector<Register> outputs;       //!< Output registers of the function
    vector<BasicBlock*> labels;     //!< Each label points to a basic block
    vector<Immediate> immediates;   //!< All immediate values in the function
    vector<BasicBlock*> blocks;     //!< All chained basic blocks
    vector<Loop *> loops;           //!< Loops info of the function
    map<uint8_t, Register> btiRegMap;//!< map bti to surface base address
    RegisterFile file;              //!< RegisterDatas used by the instructions
    Profile profile;                //!< Current function profile
    PushMap pushMap;                //!< Pushed function arguments (reg->loc)
    LocationMap locationMap;        //!< Pushed function arguments (loc->reg)
    uint32_t simdWidth;             //!< 8 or 16 if forced, 0 otherwise
    bool useSLM;                    //!< Is SLM required?
    uint32_t slmSize;               //!< local variable size inside kernel function
    uint32_t stackSize;             //!< stack size for private memory.
    SamplerSet *samplerSet;         //!< samplers used in this function.
    ImageSet* imageSet;             //!< Image set in this function's arguments..
    PrintfSet *printfSet;           //!< printfSet store the printf info.
    size_t compileWgSize[3];        //!< required work group size specified by
                                    //   __attribute__((reqd_work_group_size(X, Y, Z))).
    std::string functionAttributes; //!< function attribute qualifiers combined.
    GBE_CLASS(Function);            //!< Use custom allocator
  };

  /*! Output the function string in the given stream */
  std::ostream &operator<< (std::ostream &out, const Function &fn);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_FUNCTION_HPP__ */

