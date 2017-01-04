#include "backend/gen_insn_selection.hpp"
#include "backend/gen_insn_selection_output.hpp"
#include "sys/cvar.hpp"
#include "sys/intrusive_list.hpp"
#include <string.h>
#include <iostream>
#include <iomanip>
using namespace std;

namespace gbe
{
  static void outputGenReg(GenRegister& reg, bool dst)
  {
    if (reg.file == GEN_IMMEDIATE_VALUE || reg.file == GEN_GENERAL_REGISTER_FILE) {
      if (reg.file == GEN_IMMEDIATE_VALUE) {
        switch (reg.type) {
          case GEN_TYPE_UD:
          case GEN_TYPE_UW:
          case GEN_TYPE_UB:
          case GEN_TYPE_HF_IMM:
            cout << hex << "0x" << reg.value.ud  << dec;
            break;
          case GEN_TYPE_D:
          case GEN_TYPE_W:
          case GEN_TYPE_B:
            cout << reg.value.d;
            break;
          case GEN_TYPE_V:
            cout << hex << "0x" << reg.value.ud << dec;
            break;
          case GEN_TYPE_UL:
            cout << reg.value.u64;
            break;
          case GEN_TYPE_L:
            cout << reg.value.i64;
            break;
          case GEN_TYPE_F:
            cout << reg.value.f;
            break;
        }
      }else {
        if (reg.negation)
          cout << "-";
        if (reg.absolute)
          cout << "(abs)";
        cout << "%" << reg.value.reg;
        if (reg.subphysical)
          cout << "." << reg.subnr + reg.nr * GEN_REG_SIZE;

        if (dst)
          cout << "<" << GenRegister::hstride_size(reg) << ">";
        else
          cout << "<" << GenRegister::vstride_size(reg) << "," << GenRegister::width_size(reg) << "," << GenRegister::hstride_size(reg) << ">";
      }

      cout << ":";
      switch (reg.type) {
        case GEN_TYPE_UD:
          cout << "UD";
          break;
        case GEN_TYPE_UW:
          cout << "UW";
          break;
        case GEN_TYPE_UB:
          cout << "UB";
          break;
        case GEN_TYPE_HF_IMM:
          cout << "HF";
          break;
        case GEN_TYPE_D:
          cout << "D";
          break;
        case GEN_TYPE_W:
          cout << "W";
          break;
        case GEN_TYPE_B:
          cout << "B";
          break;
        case GEN_TYPE_V:
          cout << "V";
          break;
        case GEN_TYPE_UL:
          cout << "UL";
          break;
        case GEN_TYPE_L:
          cout << "L";
          break;
        case GEN_TYPE_F:
          cout << "F";
          break;
      }
    } else if (reg.file == GEN_ARCHITECTURE_REGISTER_FILE) {
      cout << setw(8) << "arf";
    } else
      assert(!"should not reach here");
  }

#define OP_NAME_LENGTH 512
  void outputSelectionInst(SelectionInstruction &insn) {
    cout<<"["<<insn.ID<<"]";
    if (insn.state.predicate != GEN_PREDICATE_NONE) {
      if (insn.state.physicalFlag == 0)
        cout << "(f" << insn.state.flagIndex << ")\t";
      else
        cout << "(f" << insn.state.flag << "." << insn.state.subFlag << ")\t";
    }
    else
      cout << "    \t";

    char opname[OP_NAME_LENGTH];
    if (insn.isLabel()) {
        cout << "  L" << insn.index << ":" << endl;
        return;
    } else {
      switch (insn.opcode) {
        #define DECL_SELECTION_IR(OP, FAMILY) case SEL_OP_##OP: sprintf(opname, "%s", #OP); break;
        #include "backend/gen_insn_selection.hxx"
        #undef DECL_SELECTION_IR
      }
    }

    if (insn.opcode == SEL_OP_CMP) {
      switch (insn.extra.function) {
        case GEN_CONDITIONAL_LE:
          strcat(opname, ".le");
          break;
        case GEN_CONDITIONAL_L:
          strcat(opname, ".l");
          break;
        case GEN_CONDITIONAL_GE:
          strcat(opname, ".ge");
          break;
        case GEN_CONDITIONAL_G:
          strcat(opname, ".g");
          break;
        case GEN_CONDITIONAL_EQ:
          strcat(opname, ".eq");
          break;
        case GEN_CONDITIONAL_NEQ:
          strcat(opname, ".neq");
          break;
      }
    }

    int n = strlen(opname);
    if(n >= OP_NAME_LENGTH - 20) {
      cout << "opname too long: " << opname << endl;
      return;
    }

    sprintf(&opname[n], "(%d)", insn.state.execWidth);
    cout << left << setw(20) << opname;

    for (int i = 0; i < insn.dstNum; ++i)
    {
      GenRegister dst = insn.dst(i);
      outputGenReg(dst, true);
      cout << "\t";
    }

    cout << ":\t";

    for (int i = 0; i < insn.srcNum; ++i)
    {
      GenRegister src = insn.src(i);
      outputGenReg(src, false);
      cout << "\t";
    }

    cout << endl;
  }

  void outputSelectionIR(GenContext &ctx, Selection* sel, const char* KernelName)
  {
    cout << KernelName <<"'s SELECTION IR begin:" << endl;
    cout << "WARNING: not completed yet, welcome for the FIX!" << endl;
    for (SelectionBlock &block : *sel->blockList) {
      for (SelectionInstruction &insn : block.insnList) {
        outputSelectionInst(insn);
      }
      cout << endl;
    }
    cout <<KernelName << "'s SELECTION IR end." << endl << endl;
  }

}
