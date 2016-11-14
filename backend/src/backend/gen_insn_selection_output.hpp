#ifndef __GBE_GEN_INSN_SELECTION_OUTPUT_HPP__
#define __GBE_GEN_INSN_SELECTION_OUTPUT_HPP__

namespace gbe
{
  class Selection;  // Pre ISA code
  class GenContext; // Handle compilation for Gen

  void outputSelectionIR(GenContext &ctx, Selection* sel, const char* KernelName);

} /* namespace gbe */

#endif
