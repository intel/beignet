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
 */

/**
 * \file llvm_printf.cpp
 *
 * When there are printf functions existing, we have something to do here.
 * Because the GPU's feature, it is relatively hard to parse and caculate the
 * printf's format string. OpenCL 1.2 restrict the format string to be a
 * constant string and can be decided at compiling time. So we add a pass here
 * to parse the format string and check whether the parameters is valid.
 * If all are valid, we will generate the according instruction to store the
 * parameter content into the printf buffer. And if something is invalid, a
 * warning is generated and the printf instruction is skipped in order to avoid
 * GPU error. We also keep the relationship between the printf format and printf
 * content in GPU's printf buffer here, and use the system's C standard printf to
 * print the content after kernel executed.
 */
#include <stdio.h>
#include <stdlib.h>

#include "llvm/Config/llvm-config.h"
#if LLVM_VERSION_MINOR <= 2
#include "llvm/Function.h"
#include "llvm/InstrTypes.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Module.h"
#else
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#endif  /* LLVM_VERSION_MINOR <= 2 */
#include "llvm/Pass.h"
#if LLVM_VERSION_MINOR <= 1
#include "llvm/Support/IRBuilder.h"
#elif LLVM_VERSION_MINOR == 2
#include "llvm/IRBuilder.h"
#else
#include "llvm/IR/IRBuilder.h"
#endif /* LLVM_VERSION_MINOR <= 1 */

#if LLVM_VERSION_MINOR >= 5
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CFG.h"
#else
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CFG.h"
#endif

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Attributes.h"

#include "llvm/llvm_gen_backend.hpp"
#include "sys/map.hpp"
#include "ir/printf.hpp"

using namespace llvm;

namespace gbe
{
  using namespace ir;

  /* Return the conversion_specifier if succeed, -1 if failed. */
  static char __parse_printf_state(char *begin, char *end, char** rend, PrintfState * state)
  {
    const char *fmt;
    state->left_justified = 0;
    state->sign_symbol = 0; //0 for nothing, 1 for sign, 2 for space.
    state->alter_form = 0;
    state->zero_padding = 0;
    state->vector_n = 0;
    state->min_width = 0;
    state->precision = 0;
    state->length_modifier = 0;
    state->conversion_specifier = PRINTF_CONVERSION_INVALID;
    state->out_buf_sizeof_offset = -1;

    fmt = begin;

    if (*fmt != '%')
      return -1;

#define FMT_PLUS_PLUS do {                                  \
      if (fmt + 1 < end) fmt++;                             \
      else {                                                \
        printf("Error, line: %d, fmt > end\n", __LINE__);   \
        return -1;                                          \
      }                                                     \
    }  while(0)

    FMT_PLUS_PLUS;

    // parse the flags.
    switch (*fmt) {
      case '-':
        /* The result of the conversion is left-justified within the field. */
        state->left_justified = 1;
        FMT_PLUS_PLUS;
        break;
      case '+':
        /* The result of a signed conversion always begins with a plus or minus sign. */
        state->sign_symbol = 1;
        FMT_PLUS_PLUS;
        break;
      case ' ':
        /* If the first character of a signed conversion is not a sign, or if a signed
           conversion results in no characters, a space is prefixed to the result.
           If the space and + flags both appear,the space flag is ignored. */
        if (state->sign_symbol == 0) state->sign_symbol = 2;
        FMT_PLUS_PLUS;
        break;
      case '#':
        /*The result is converted to an alternative form. */
        state->alter_form = 1;
        FMT_PLUS_PLUS;
        break;
      case '0':
        if (!state->left_justified) state->zero_padding = 1;
        FMT_PLUS_PLUS;
        break;
      default:
        break;
    }

    // The minimum field width
    while ((*fmt >= '0') && (*fmt <= '9')) {
      state->min_width = state->min_width * 10 + (*fmt - '0');
      FMT_PLUS_PLUS;
    }

    // The precision
    if (*fmt == '.') {
      FMT_PLUS_PLUS;
      while (*fmt >= '0' && *fmt <= '9') {
        state->precision = state->precision * 10 + (*fmt - '0');
        FMT_PLUS_PLUS;
      }
    }

    // handle the vector specifier.
    if (*fmt == 'v') {
      FMT_PLUS_PLUS;
      switch (*fmt) {
        case '2':
          state->vector_n = 2;
          FMT_PLUS_PLUS;
          break;
        case '3':
          state->vector_n = 3;
          FMT_PLUS_PLUS;
          break;
        case '4':
          state->vector_n = 4;
          FMT_PLUS_PLUS;
          break;
        case '8':
          state->vector_n = 8;
          FMT_PLUS_PLUS;
          break;
        case '1':
          FMT_PLUS_PLUS;
          if (*fmt == '6') {
            state->vector_n = 16;
            FMT_PLUS_PLUS;
          } else
            return -1;
          break;
        default:
          //Wrong vector, error.
          return -1;
      }
    }

    // length modifiers
    if (*fmt == 'h') {
      FMT_PLUS_PLUS;
      if (*fmt == 'h') { //hh
        state->length_modifier = PRINTF_LM_HH;
        FMT_PLUS_PLUS;
      } else if (*fmt == 'l') { //hl
        state->length_modifier = PRINTF_LM_HL;
        FMT_PLUS_PLUS;
      } else { //h
        state->length_modifier = PRINTF_LM_H;
      }
    } else if (*fmt == 'l') {
      state->length_modifier = PRINTF_LM_L;
      FMT_PLUS_PLUS;
    }

#define CONVERSION_SPEC_AND_RET(XXX, xxx)                           \
    case XXX:                                                       \
      state->conversion_specifier = PRINTF_CONVERSION_##xxx;        \
      FMT_PLUS_PLUS;                                                \
      *rend = (char *)fmt;                                          \
      return XXX;                                                   \
      break;

    // conversion specifiers
    switch (*fmt) {
        CONVERSION_SPEC_AND_RET('d', D)
        CONVERSION_SPEC_AND_RET('i', I)
        CONVERSION_SPEC_AND_RET('o', O)
        CONVERSION_SPEC_AND_RET('u', U)
        CONVERSION_SPEC_AND_RET('x', x)
        CONVERSION_SPEC_AND_RET('X', X)
        CONVERSION_SPEC_AND_RET('f', f)
        CONVERSION_SPEC_AND_RET('F', F)
        CONVERSION_SPEC_AND_RET('e', e)
        CONVERSION_SPEC_AND_RET('E', E)
        CONVERSION_SPEC_AND_RET('g', g)
        CONVERSION_SPEC_AND_RET('G', G)
        CONVERSION_SPEC_AND_RET('a', a)
        CONVERSION_SPEC_AND_RET('A', A)
        CONVERSION_SPEC_AND_RET('c', C)
        CONVERSION_SPEC_AND_RET('s', A)
        CONVERSION_SPEC_AND_RET('p', P)

        // %% has been handled

      default:
        return -1;
    }
  }

  static PrintfSet::PrintfFmt* parser_printf_fmt(char* format, int& num)
  {
    char* begin;
    char* end;
    char* p;
    char ret_char;
    char* rend;
    PrintfState state;
    PrintfSet::PrintfFmt* printf_fmt = new PrintfSet::PrintfFmt();

    p = format;
    begin = format;
    end = format + strlen(format);

    /* Now parse it. */
    while (*begin) {
      p = begin;

again:
      while (p < end && *p != '%') {
        p++;
      }
      if (p < end && p + 1 == end) { // String with % at end.
        printf("string end with %%\n");
        goto error;
      }
      if (*(p + 1) == '%') { // %%
        p += 2;
        goto again;
      }

      if (p != begin) {
        std::string s = std::string(begin, size_t(p - begin));
        printf_fmt->push_back(PrintfSlot(s.c_str()));
      }

      if (p == end) // finish
        break;

      /* Now parse the % start conversion_specifier. */
      ret_char = __parse_printf_state(p, end, &rend, &state);
      if (ret_char < 0)
        goto error;

      printf_fmt->push_back(&state);

      if (rend == end)
        break;

      begin = rend;
    }

    for (auto &s : *printf_fmt) {
      if (s.type == PRINTF_SLOT_TYPE_STATE) {
        num++;
#if 0
        printf("---- %d ---: state : \n", j);
        printf("		     left_justified : %d\n", s.state->left_justified);
        printf("		     sign_symbol: %d\n", s.state->sign_symbol);
        printf("		     alter_form : %d\n", s.state->alter_form);
        printf("		     zero_padding : %d\n", s.state->zero_padding);
        printf("		     vector_n : %d\n", s.state->vector_n);
        printf("		     min_width : %d\n", s.state->min_width);
        printf("		     precision : %d\n", s.state->precision);
        printf("		     length_modifier : %d\n", s.state->length_modifier);
        printf("		     conversion_specifier : %d\n", s.state->conversion_specifier);
#endif
      } else if (s.type == PRINTF_SLOT_TYPE_STRING) {
        //printf("---- %d ---: string :  %s\n", j, s.str);
      }
    }

    return printf_fmt;

error:
    printf("error format string.\n");
    delete printf_fmt;
    return NULL;
  }

  class PrintfParser : public FunctionPass
  {
  public:
    static char ID;
    typedef std::pair<Instruction*, bool> PrintfInst;
    std::vector<PrintfInst> deadprintfs;
    Module* module;
    IRBuilder<>* builder;
    Type* intTy;
    Value* pbuf_ptr;
    Value* index_buf_ptr;
    int out_buf_sizeof_offset;
    static map<CallInst*, PrintfSet::PrintfFmt*> printfs;
    int printf_num;

    PrintfParser(void) : FunctionPass(ID) {
      module = NULL;
      builder = NULL;
      intTy = NULL;
      out_buf_sizeof_offset = 0;
      printfs.clear();
      pbuf_ptr = NULL;
      index_buf_ptr = NULL;
      printf_num = 0;
    }

    ~PrintfParser(void) {
      for (auto &s : printfs) {
        delete s.second;
        s.second = NULL;
      }
      printfs.clear();
    }


    bool parseOnePrintfInstruction(CallInst *& call);
    int generateOneParameterInst(PrintfSlot& slot, Value& arg);

    virtual const char *getPassName() const {
      return "Printf Parser";
    }

    virtual bool runOnFunction(llvm::Function &F);
  };

  bool PrintfParser::parseOnePrintfInstruction(CallInst *& call)
  {
    CallSite CS(call);
    CallSite::arg_iterator CI_FMT = CS.arg_begin();
    int param_num = 0;

    llvm::Constant* arg0 = dyn_cast<llvm::ConstantExpr>(*CI_FMT);
    llvm::Constant* arg0_ptr = dyn_cast<llvm::Constant>(arg0->getOperand(0));
    if (!arg0_ptr) {
      return false;
    }

    ConstantDataSequential* fmt_arg = dyn_cast<ConstantDataSequential>(arg0_ptr->getOperand(0));
    if (!fmt_arg || !fmt_arg->isCString()) {
      return false;
    }

    std::string fmt = fmt_arg->getAsCString();

    PrintfSet::PrintfFmt* printf_fmt = NULL;

    if (!(printf_fmt = parser_printf_fmt((char *)fmt.c_str(), param_num))) {//at lease print something
      return false;
    }

    /* iff parameter more than %, error. */
    /* str_fmt arg0 arg1 ... NULL */
    if (param_num + 2 < static_cast<int>(call->getNumOperands())) {
      delete printf_fmt;
      return false;
    }

    /* FIXME: Because the OpenCL language do not support va macro, and we do not want
       to introduce the va_list, va_start and va_end into our code, we just simulate
       the function calls to caculate the offset caculation here. */
    CallInst* group_id_2 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                             "__gen_ocl_get_group_id2",
                             IntegerType::getInt32Ty(module->getContext()),
                             NULL)));
    CallInst* group_id_1 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                             "__gen_ocl_get_group_id1",
                             IntegerType::getInt32Ty(module->getContext()),
                             NULL)));
    CallInst* group_id_0 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                             "__gen_ocl_get_group_id0",
                             IntegerType::getInt32Ty(module->getContext()),
                             NULL)));

    CallInst* global_size_2 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                                "__gen_ocl_get_global_size2",
                                IntegerType::getInt32Ty(module->getContext()),
                                NULL)));
    CallInst* global_size_1 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                                "__gen_ocl_get_global_size1",
                                IntegerType::getInt32Ty(module->getContext()),
                                NULL)));
    CallInst* global_size_0 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                                "__gen_ocl_get_global_size0",
                                IntegerType::getInt32Ty(module->getContext()),
                                NULL)));

    CallInst* local_id_2 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                             "__gen_ocl_get_local_id2",
                             IntegerType::getInt32Ty(module->getContext()),
                             NULL)));
    CallInst* local_id_1 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                             "__gen_ocl_get_local_id1",
                             IntegerType::getInt32Ty(module->getContext()),
                             NULL)));
    CallInst* local_id_0 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                             "__gen_ocl_get_local_id0",
                             IntegerType::getInt32Ty(module->getContext()),
                             NULL)));

    CallInst* local_size_2 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                               "__gen_ocl_get_local_size2",
                               IntegerType::getInt32Ty(module->getContext()),
                               NULL)));
    CallInst* local_size_1 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                               "__gen_ocl_get_local_size1",
                               IntegerType::getInt32Ty(module->getContext()),
                               NULL)));
    CallInst* local_size_0 = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                               "__gen_ocl_get_local_size0",
                               IntegerType::getInt32Ty(module->getContext()),
                               NULL)));
    Value* op0 = NULL;
    Value* val = NULL;
    /* offset = ((local_id_2 + local_size_2 * group_id_2) * (global_size_1 * global_size_0)
       + (local_id_1 + local_size_1 * group_id_1) * global_size_0
       + (local_id_0 + local_size_0 * group_id_0)) * sizeof(type)  */

    // local_size_2 * group_id_2
    val = builder->CreateMul(local_size_2, group_id_2);
    // local_id_2 + local_size_2 * group_id_2
    val = builder->CreateAdd(local_id_2, val);
    // global_size_1 * global_size_0
    op0 = builder->CreateMul(global_size_1, global_size_0);
    // (local_id_2 + local_size_2 * group_id_2) * (global_size_1 * global_size_0)
    Value* offset1 = builder->CreateMul(val, op0);
    // local_size_1 * group_id_1
    val = builder->CreateMul(local_size_1, group_id_1);
    // local_id_1 + local_size_1 * group_id_1
    val = builder->CreateAdd(local_id_1, val);
    // (local_id_1 + local_size_1 * group_id_1) * global_size_0
    Value* offset2 = builder->CreateMul(val, global_size_0);
    // local_size_0 * group_id_0
    val = builder->CreateMul(local_size_0, group_id_0);
    // local_id_0 + local_size_0 * group_id_0
    val = builder->CreateAdd(local_id_0, val);
    // The total sum
    val = builder->CreateAdd(val, offset1);
    Value* offset = builder->CreateAdd(val, offset2);

    /////////////////////////////////////////////////////
    /* calculate index address.
       index_addr = (index_offset + offset )* sizeof(int) + index_buf_ptr
       index_offset = global_size_2 * global_size_1 * global_size_0 * printf_num */

    // global_size_2 * global_size_1
    op0 = builder->CreateMul(global_size_2, global_size_1);
    // global_size_2 * global_size_1 * global_size_0
    Value* glXg2Xg3 = builder->CreateMul(op0, global_size_0);
    Value* index_offset = builder->CreateMul(glXg2Xg3, ConstantInt::get(intTy, printf_num));
    // index_offset + offset
    op0 = builder->CreateAdd(index_offset, offset);
    // (index_offset + offset)* sizeof(int)
    op0 = builder->CreateMul(op0, ConstantInt::get(intTy, sizeof(int)));
    // Final index address = index_buf_ptr + (index_offset + offset)* sizeof(int)
    op0 = builder->CreateAdd(op0, index_buf_ptr);
    Value* index_addr = builder->CreateIntToPtr(op0, Type::getInt32PtrTy(module->getContext(), 1));
    builder->CreateStore(ConstantInt::get(intTy, 1), index_addr);// The flag

    int i = 1;
    Value* data_addr = NULL;
    for (auto &s : *printf_fmt) {
      if (s.type == PRINTF_SLOT_TYPE_STRING)
        continue;

      assert(i < static_cast<int>(call->getNumOperands()) - 1);

      int sizeof_size = generateOneParameterInst(s, *call->getOperand(i));
      if (!sizeof_size) {
        printf("Printf: %d, parameter %d may have no result because some error\n",
               printf_num, i - 1);
        continue;
      }

      /////////////////////////////////////////////////////
      /* Calculate the data address.
      data_addr = data_offset + pbuf_ptr + offset * sizeof(specify)
      data_offset = global_size_2 * global_size_1 * global_size_0 * out_buf_sizeof_offset

      //global_size_2 * global_size_1 * global_size_0 * out_buf_sizeof_offset */
      op0 = builder->CreateMul(glXg2Xg3, ConstantInt::get(intTy, out_buf_sizeof_offset));
      //offset * sizeof(specify)
      val = builder->CreateMul(offset, ConstantInt::get(intTy, sizeof_size));
      //data_offset + pbuf_ptr
      op0 = builder->CreateAdd(op0, pbuf_ptr);
      op0 = builder->CreateAdd(op0, val);
      data_addr = builder->CreateIntToPtr(op0, Type::getInt32PtrTy(module->getContext(), 1));
      builder->CreateStore(call->getOperand(i), data_addr);
      s.state->out_buf_sizeof_offset = out_buf_sizeof_offset;
      out_buf_sizeof_offset += sizeof_size;
      i++;
    }

    CallInst* printf_inst = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                              "__gen_ocl_printf", Type::getVoidTy(module->getContext()),
                              NULL)));
    assert(printfs[printf_inst] == NULL);
    printfs[printf_inst] = printf_fmt;
    printf_num++;
    return true;
  }

  bool PrintfParser::runOnFunction(llvm::Function &F)
  {
    bool changed = false;
    switch (F.getCallingConv()) {
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 2
      case CallingConv::PTX_Device:
        return false;
      case CallingConv::PTX_Kernel:
#else
      case CallingConv::C:
#endif
        break;
      default:
        GBE_ASSERTM(false, "Unsupported calling convention");
    }

    module = F.getParent();
    intTy = IntegerType::get(module->getContext(), 32);
    builder = new IRBuilder<>(module->getContext());

    // As we inline all function calls, so skip non-kernel functions
    bool bKernel = isKernelFunction(F);
    if(!bKernel) return false;

    /* Iter the function and find printf. */
    for (llvm::Function::iterator B = F.begin(), BE = F.end(); B != BE; B++) {
      for (BasicBlock::iterator instI = B->begin(),
           instE = B->end(); instI != instE; ++instI) {

        llvm::CallInst* call = dyn_cast<llvm::CallInst>(instI);
        if (!call) {
          continue;
        }

        if (call->getCalledFunction()->getIntrinsicID() != 0)
          continue;

        Value *Callee = call->getCalledValue();
        const std::string fnName = Callee->getName();

        if (fnName != "__gen_ocl_printf_stub")
          continue;

        changed = true;

        builder->SetInsertPoint(call);

        if (!pbuf_ptr) {
          /* alloc a new buffer ptr to collect the print output. */
          pbuf_ptr = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                                           "__gen_ocl_printf_get_buf_addr", Type::getInt32Ty(module->getContext()),
                                           NULL)));
        }
        if (!index_buf_ptr) {
          /* alloc a new buffer ptr to collect the print valid index. */
          index_buf_ptr = builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
                                                "__gen_ocl_printf_get_index_buf_addr", Type::getInt32Ty(module->getContext()),
                                                NULL)));
        }

        deadprintfs.push_back(PrintfInst(cast<Instruction>(call),parseOnePrintfInstruction(call)));
      }
    }

    /* Replace the instruction's operand if using printf's return value. */
    for (llvm::Function::iterator B = F.begin(), BE = F.end(); B != BE; B++) {
      for (BasicBlock::iterator instI = B->begin(),
           instE = B->end(); instI != instE; ++instI) {

        for (unsigned i = 0; i < instI->getNumOperands(); i++) {
          for (auto &prf : deadprintfs) {
            if (instI->getOperand(i) == prf.first) {

              if (prf.second == true) {
                instI->setOperand(i, ConstantInt::get(intTy, 0));
              } else {
                instI->setOperand(i, ConstantInt::get(intTy, -1));
              }
            }
          }
        }
      }
    }

    /* Kill the dead printf instructions. */
    for (auto &prf : deadprintfs) {
      prf.first->dropAllReferences();
      if (prf.first->use_empty())
        prf.first->eraseFromParent();
    }

    delete builder;

    return changed;
  }

  int PrintfParser::generateOneParameterInst(PrintfSlot& slot, Value& arg)
  {
    assert(slot.type == PRINTF_SLOT_TYPE_STATE);
    assert(builder);

    /* Check whether the arg match the format specifer. If needed, some
       conversion need to be applied. */
    switch (arg.getType()->getTypeID()) {
      case Type::IntegerTyID: {
        switch (slot.state->conversion_specifier) {
          case PRINTF_CONVERSION_I:
          case PRINTF_CONVERSION_D:
            /* Int to Int, just store. */
            return sizeof(int);
            break;

          default:
            return 0;
        }
      }
      break;

      default:
        return 0;
    }

    return 0;
  }

  map<CallInst*, PrintfSet::PrintfFmt*> PrintfParser::printfs;

  void* getPrintfInfo(CallInst* inst)
  {
    if (PrintfParser::printfs[inst])
      return (void*)PrintfParser::printfs[inst];
    return NULL;
  }

  FunctionPass* createPrintfParserPass()
  {
    return new PrintfParser();
  }
  char PrintfParser::ID = 0;

} // end namespace
