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
 * \file context.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_CONTEXT_HPP__
#define __GBE_CONTEXT_HPP__

#include "sys/platform.hpp"
#include "sys/set.hpp"
#include "sys/map.hpp"
#include "ir/instruction.hpp"
#include "backend/program.h"
#include <string>

namespace gbe {
namespace ir {

  class Unit;        // Contains the complete program
  class Function;    // We compile a function into a kernel
  class Liveness;    // Describes liveness of each ir function register
  class FunctionDAG; // Describes the instruction dependencies

} /* namespace ir */
} /* namespace gbe */

namespace gbe
{
  struct Kernel; // we build this structure

  /*! Structure that keeps track of allocation in the register file. This is
   *  actually needed by Context (and not only by GenContext) because both
   *  simulator and hardware have to deal with constant pushing which uses the
   *  register file
   *
   *  Since Gen is pretty flexible, we just maintain a free list for the
   *  register file (as a classical allocator) and coalesce blocks when required
   */
  class RegisterFileAllocator
  {
  public:
    RegisterFileAllocator(void);
    ~RegisterFileAllocator(void);

    /*! Allocate some memory in the register file. Return 0 if out-of-memory. By
     *  the way, zero is not a valid offset since r0 is always preallocated by
     *  the hardware. Note that we always use the left most block when
     *  allocating, so it makes sense for constant pushing
     */
    int16_t allocate(int16_t size, int16_t alignment);

    /*! Free the given register file piece */
    void deallocate(int16_t offset);

  private:
    /*! May need to make that run-time in the future */
    static const int16_t RegisterFileSize = 4*KB;

    /*! Double chained list of free spaces */
    struct Block {
      Block(int16_t offset, int16_t size) :
        prev(NULL), next(NULL), offset(offset), size(size) {}
      Block *prev, *next; //!< Previous and next free blocks
      int16_t offset;        //!< Where the free block starts
      int16_t size;          //!< Size of the free block
    };

    /*! Try to coalesce two blocks (left and right). They must be in that order.
     *  If the colascing was done, the left block is deleted
     */
    void coalesce(Block *left, Block *right);
    /*! Head of the free list */
    Block *head;
    /*! Handle free list element allocation */
    DECL_POOL(Block, blockPool);
    /*! Track allocated memory blocks <offset, size> */
    map<int16_t, int16_t> allocatedBlocks;
  };

  /*! Context is the helper structure to build the Gen ISA or simulation code
   *  from GenIR
   */
  class Context : public NonCopyable
  {
  public:
    /*! Create a new context. name is the name of the function we want to
     *  compile
     */
    Context(const ir::Unit &unit, const std::string &name);
    /*! Release everything needed */
    virtual ~Context(void);
    /*! Compile the code */
    Kernel *compileKernel(void);
    /*! Tells if the labels is used */
    INLINE bool isLabelUsed(ir::LabelIndex index) const {
      return usedLabels.contains(index);
    }
    /*! Tells if the register is used */
    bool isRegUsed(const ir::Register &reg) const;
    /*! Indicate if a register is scalar or not */
    bool isScalarReg(const ir::Register &reg) const;
    /*! Get the kernel we are currently compiling */
    INLINE Kernel *getKernel(void) const { return this->kernel; }
    /*! Get the function we are currently compiling */
    INLINE const ir::Function &getFunction(void) const { return this->fn; }
    /*! Get the target label index for the given instruction */
    INLINE ir::LabelIndex getLabelIndex(const ir::Instruction *insn) const {
      GBE_ASSERT(JIPs.find(insn) != JIPs.end());
      return JIPs.find(insn)->second;
    }
    /*! Only GOTO and some LABEL instructions may have JIPs */
    INLINE bool hasJIP(const ir::Instruction *insn) const {
      return JIPs.find(insn) != JIPs.end();
    }
    /*! Allocate some memory in the register file */
    INLINE int16_t allocate(int16_t size, int16_t alignment) {
      return raFile.allocate(size, alignment);
    }
    /*! Deallocate previously allocated memory */
    INLINE void deallocate(int16_t offset) { raFile.deallocate(offset); }
  protected:
    /*! Look if a stack is needed and allocate it */
    void buildStack(void);
    /*! Build the curbe patch list for the given kernel */
    void buildPatchList(void);
    /*! Build the list of arguments to set to launch the kernel */
    void buildArgList(void);
    /*! Build the sets of used labels */
    void buildUsedLabels(void);
    /*! Build JIPs for each branch and possibly labels. Can be different from
     *  the branch target due to unstructured branches
     */
    void buildJIPs(void);
    /*! Build the instruction stream */
    virtual void emitCode(void) = 0;
    /*! Allocate a new empty kernel */
    virtual Kernel *allocateKernel(void) = 0;
    /*! Insert a new entry with the given size in the Curbe. Return the offset
     *  of the entry
     */
    void newCurbeEntry(gbe_curbe_type value, uint32_t subValue, uint32_t size, uint32_t alignment = 0);
    /*! Provide for each branch and label the label index target */
    typedef map<const ir::Instruction*, ir::LabelIndex> JIPMap;
    const ir::Unit &unit;           //!< Unit that contains the kernel
    const ir::Function &fn;         //!< Function to compile
    std::string name;               //!< Name of the kernel to compile
    Kernel *kernel;                 //!< Kernel we are building
    ir::Liveness *liveness;         //!< Liveness info for the variables
    ir::FunctionDAG *dag;           //!< Graph of values on the function
    set<ir::LabelIndex> usedLabels; //!< Set of all used labels
    JIPMap JIPs;                    //!< Where to jump all labels / branches
    RegisterFileAllocator raFile;   //!< Handle register allocation / deallocation
    uint32_t simdWidth;             //!< Number of lanes per HW threads
  };

} /* namespace gbe */

#endif /* __GBE_CONTEXT_HPP__ */

