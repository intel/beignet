/*
 * Copyright © 2012 Intel Corporation
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
 */

/**
 * \file llvm_printf_parser.cpp
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

#include "llvm_includes.hpp"

#include "llvm/llvm_gen_backend.hpp"
#include "sys/map.hpp"
#include "ir/printf.hpp"
#include "ir/unit.hpp"

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
    state->min_width = -1;
    state->precision = -1;
    state->length_modifier = 0;
    state->conversion_specifier = PRINTF_CONVERSION_INVALID;
    state->out_buf_sizeof_offset = -1;

    fmt = begin;

    if (*fmt != '%')
      return -1;

#define FMT_PLUS_PLUS do {                                  \
      if (fmt + 1 <= end) fmt++;                             \
      else {                                                \
        printf("Error, line: %d, fmt > end\n", __LINE__);   \
        return -1;                                          \
      }                                                     \
    }  while(0)

    FMT_PLUS_PLUS;

    // parse the flags.
    while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '#' || *fmt == '0')
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
      if (state->min_width < 0)
        state->min_width = 0;
      state->min_width = state->min_width * 10 + (*fmt - '0');
      FMT_PLUS_PLUS;
    }

    // The precision
    if (*fmt == '.') {
      FMT_PLUS_PLUS;
      state->precision = 0;
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
        case '3':
        case '4':
        case '8':
          state->vector_n = *fmt - '0';
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
        CONVERSION_SPEC_AND_RET('s', S)
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
      if (p + 1 < end && *(p + 1) == '%') { // %%
        p += 2;
        goto again;
      }

      if (p != begin) {
        std::string s(begin, size_t(p - begin));
        printf_fmt->push_back(PrintfSlot(s));
      }

      if (p == end) // finish
        break;

      /* Now parse the % start conversion_specifier. */
      ret_char = __parse_printf_state(p, end, &rend, &state);
      if (ret_char < 0)
        goto error;

      printf_fmt->push_back(state);
      num++;

      if (rend == end)
        break;

      begin = rend;
    }

#if 0
    {
      int j = 0;
      for (auto &s : printf_fmt->first) {
        j++;
        if (s.type == PRINTF_SLOT_TYPE_STATE) {
          fprintf(stderr, "---- %d ---: state : \n", j);
          fprintf(stderr, "		     left_justified : %d\n", s.state->left_justified);
          fprintf(stderr, "		     sign_symbol: %d\n", s.state->sign_symbol);
          fprintf(stderr, "		     alter_form : %d\n", s.state->alter_form);
          fprintf(stderr, "		     zero_padding : %d\n", s.state->zero_padding);
          fprintf(stderr, "		     vector_n : %d\n", s.state->vector_n);
          fprintf(stderr, "		     min_width : %d\n", s.state->min_width);
          fprintf(stderr, "		     precision : %d\n", s.state->precision);
          fprintf(stderr, "		     length_modifier : %d\n", s.state->length_modifier);
          fprintf(stderr, "		     conversion_specifier : %d\n", s.state->conversion_specifier);
        } else if (s.type == PRINTF_SLOT_TYPE_STRING) {
          fprintf(stderr, "---- %d ---: string :  %s\n", j, s.str);
        }
      }
    }
#endif

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
    Module* module;
    IRBuilder<>* builder;
    Type* intTy;
    ir::Unit &unit;

    PrintfParser(ir::Unit &unit) : FunctionPass(ID),
      unit(unit)
    {
      module = NULL;
      builder = NULL;
      intTy = NULL;
    }

    bool parseOnePrintfInstruction(CallInst * call);
    bool generateOneParameterInst(PrintfSlot& slot, Value* arg, Value*& new_arg);

    virtual const char *getPassName() const
    {
      return "Printf Parser";
    }

    virtual bool runOnFunction(llvm::Function &F);
  };

  bool PrintfParser::parseOnePrintfInstruction(CallInst * call)
  {
    CallSite CS(call);
    CallSite::arg_iterator CI_FMT = CS.arg_begin();
    int param_num = 0;

    llvm::Constant* arg0 = dyn_cast<llvm::ConstantExpr>(*CI_FMT);
    if(!arg0) {
      return false;
    }
    llvm::Constant* arg0_ptr = dyn_cast<llvm::Constant>(arg0->getOperand(0));
    if (!arg0_ptr) {
      return false;
    }

    ConstantDataSequential* fmt_arg = dyn_cast<ConstantDataSequential>(arg0_ptr->getOperand(0));
    if (!fmt_arg || !fmt_arg->isCString()) {
      return false;
    }

    std::string fmt = fmt_arg->getAsCString();
    if (fmt.size() == 0)
      return false;

    PrintfSet::PrintfFmt* printf_fmt = NULL;

    if (!(printf_fmt = parser_printf_fmt((char *)fmt.c_str(), param_num))) {//at lease print something
      printf("Warning: Parse the printf inst %s failed, no output for it\n", fmt.c_str());
      return false;
    }

    /* iff parameter more than %, error. */
    /* str_fmt arg0 arg1 ... NULL */
    if (param_num + 2 != static_cast<int>(call->getNumOperands())) {
      delete printf_fmt;
      printf("Warning: Parse the printf inst %s failed, parameters do not match the %% number, no output for it\n",
             fmt.c_str());
      return false;
    }

    /* Insert some conversion if types do not match. */
    builder->SetInsertPoint(call);
    int i = 1;
    for (auto &s : *printf_fmt) {
      if (s.type == PRINTF_SLOT_TYPE_STRING)
        continue;

      assert(i < static_cast<int>(call->getNumOperands()) - 1);
      Value* new_arg = NULL;
      Value *arg = call->getOperand(i);
      if (generateOneParameterInst(s, arg, new_arg) == false) {
        delete printf_fmt;
        printf("Warning: Parse the printf inst %s failed, the %d parameter format is wrong, no output for it\n",
               fmt.c_str(), i);
        return false;
      }

      if (new_arg) { // replace the according argument.
        call->setArgOperand(i, new_arg);
      }
      ++i;
    }

    GBE_ASSERT(unit.printfs.find(call) == unit.printfs.end());
    unit.printfs.insert(std::pair<llvm::CallInst*, PrintfSet::PrintfFmt*>(call, printf_fmt));
    return true;
  }

  bool PrintfParser::runOnFunction(llvm::Function &F)
  {
    bool hasPrintf = false;
    switch (F.getCallingConv()) {
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 2
      case CallingConv::PTX_Device:
        return false;
      case CallingConv::PTX_Kernel:
#else
      case CallingConv::C:
      case CallingConv::Fast:
      case CallingConv::SPIR_KERNEL:
#endif
        break;
      default:
        GBE_ASSERTM(false, "Unsupported calling convention");
    }

    module = F.getParent();
    intTy = IntegerType::get(module->getContext(), 32);

    // As we inline all function calls, so skip non-kernel functions
    bool bKernel = isKernelFunction(F);
    if(!bKernel) return false;

    builder = new IRBuilder<>(module->getContext());

    llvm::GlobalValue* gFun = module->getNamedValue("printf");
    if(gFun) {
      gFun->setName("__gen_ocl_printf_stub");
    }
    llvm::GlobalValue* gFun2 = module->getNamedValue("puts");
    if(gFun2 ) {
      gFun2->setName("__gen_ocl_puts_stub");
    }

    /* First find printfs and caculate all slots size of one loop. */
    for (llvm::Function::iterator B = F.begin(), BE = F.end(); B != BE; B++) {
      for (BasicBlock::iterator instI = B->begin(),
           instE = B->end(); instI != instE; ++instI) {

        llvm::CallInst* call = dyn_cast<llvm::CallInst>(instI);
        if (!call) {
          continue;
        }

        llvm::Function * callFunc = call->getCalledFunction();
        if(!callFunc) {
          continue;
        }

        if ( callFunc->getIntrinsicID() != 0)
          continue;

        Value *Callee = call->getCalledValue();
        const std::string fnName = Callee->getName();

        if (fnName != "__gen_ocl_printf_stub" && fnName != "__gen_ocl_puts_stub")
          continue;

        if (!parseOnePrintfInstruction(call)) {
          // Just skip this printf instruction.
          continue;
        }

        hasPrintf = true;
      }
    }

    delete builder;
    return hasPrintf;
  }

  bool PrintfParser::generateOneParameterInst(PrintfSlot& slot, Value* arg, Value*& new_arg)
  {
    assert(slot.type == PRINTF_SLOT_TYPE_STATE);
    assert(builder);

    /* Check whether the arg match the format specifer. If needed, some
       conversion need to be applied. */
    switch (arg->getType()->getTypeID()) {
      case Type::IntegerTyID: {
        bool sign = false;
        switch (slot.state.conversion_specifier) {
          case PRINTF_CONVERSION_I:
          case PRINTF_CONVERSION_D:
            sign = true;
          case PRINTF_CONVERSION_O:
          case PRINTF_CONVERSION_U:
          case PRINTF_CONVERSION_x:
          case PRINTF_CONVERSION_X:
            if (slot.state.length_modifier == PRINTF_LM_L) { /* we would rather print long. */
              if (arg->getType() != Type::getInt64Ty(module->getContext())) {
                new_arg = builder->CreateIntCast(arg, Type::getInt64Ty(module->getContext()), sign);
              }
            } else {
              /* If the bits change, we need to consider the signed. */
              if (arg->getType() != Type::getInt32Ty(module->getContext())) {
                new_arg = builder->CreateIntCast(arg, Type::getInt32Ty(module->getContext()), sign);
              }
            }
            return true;

          case PRINTF_CONVERSION_C:
            /* Int to Char, add a conversion. */
            new_arg = builder->CreateIntCast(arg, Type::getInt8Ty(module->getContext()), false);
            return true;

          case PRINTF_CONVERSION_F:
          case PRINTF_CONVERSION_f:
          case PRINTF_CONVERSION_E:
          case PRINTF_CONVERSION_e:
          case PRINTF_CONVERSION_G:
          case PRINTF_CONVERSION_g:
          case PRINTF_CONVERSION_A:
          case PRINTF_CONVERSION_a:
            printf("Warning: Have a float parameter for %%d like specifier, take care of it\n");
            new_arg = builder->CreateSIToFP(arg, Type::getFloatTy(module->getContext()));
            return true;

          case PRINTF_CONVERSION_S:
            /* Here, the case is printf("xxx%s", 0); we should output the null. */
            slot.state.str = "(null)";
            return true;

          default:
            return false;
        }

        break;
      }

      case Type::DoubleTyID:
      case Type::FloatTyID: {
        /* llvm 3.6 will give a undef value for NAN. */
        if (dyn_cast<llvm::UndefValue>(arg)) {
          APFloat nan = APFloat::getNaN(APFloat::IEEEsingle, false);
          new_arg = ConstantFP::get(module->getContext(), nan);
        }

        /* Because the printf is a variable parameter function, it does not have the
           function prototype, so the compiler will always promote the arg to the
           longest precise type for float. So here, we can always find it is double. */
        switch (slot.state.conversion_specifier) {
          case PRINTF_CONVERSION_I:
          case PRINTF_CONVERSION_D:
            /* Float to Int, add a conversion. */
            printf("Warning: Have a int parameter for %%f like specifier, take care of it\n");
            new_arg = builder->CreateFPToSI(arg, Type::getInt32Ty(module->getContext()));
            return true;

          case PRINTF_CONVERSION_O:
          case PRINTF_CONVERSION_U:
          case PRINTF_CONVERSION_x:
          case PRINTF_CONVERSION_X:
            /* Float to uint, add a conversion. */
            printf("Warning: Have a uint parameter for %%f like specifier, take care of it\n");
            new_arg = builder->CreateFPToUI(arg, Type::getInt32Ty(module->getContext()));
            return true;

          case PRINTF_CONVERSION_F:
          case PRINTF_CONVERSION_f:
          case PRINTF_CONVERSION_E:
          case PRINTF_CONVERSION_e:
          case PRINTF_CONVERSION_G:
          case PRINTF_CONVERSION_g:
          case PRINTF_CONVERSION_A:
          case PRINTF_CONVERSION_a:
            new_arg = builder->CreateFPCast(arg, Type::getFloatTy(module->getContext()));
            return true;

          default:
            return false;
        }

        break;
      }

      /* %p and %s */
      case Type::PointerTyID:
        switch (slot.state.conversion_specifier) {
          case PRINTF_CONVERSION_S: {
            llvm::Constant* arg0 = dyn_cast<llvm::ConstantExpr>(arg);
            if(!arg0) {
              return false;
            }
            llvm::Constant* arg0_ptr = dyn_cast<llvm::Constant>(arg0->getOperand(0));
            if (!arg0_ptr) {
              return false;
            }

            ConstantDataSequential* fmt_arg = dyn_cast<ConstantDataSequential>(arg0_ptr->getOperand(0));
            if (!fmt_arg || !fmt_arg->isCString()) {
              return false;
            }
            slot.state.str = fmt_arg->getAsCString();
            return true;
          }
          case PRINTF_CONVERSION_P: {
            new_arg = builder->CreatePtrToInt(arg, Type::getInt32Ty(module->getContext()));
            return true;
          }
          default:
            return false;
        }

        break;

      case Type::VectorTyID: {
        Type* vect_type = arg->getType();
        Type* elt_type = vect_type->getVectorElementType();
        int vec_num = vect_type->getVectorNumElements();
        bool sign = false;

        if (vec_num != slot.state.vector_n) {
          printf("Error The printf vector number is not match!\n");
          return false;
        }

        switch (slot.state.conversion_specifier) {
          case PRINTF_CONVERSION_I:
          case PRINTF_CONVERSION_D:
            sign = true;
          case PRINTF_CONVERSION_O:
          case PRINTF_CONVERSION_U:
          case PRINTF_CONVERSION_x:
          case PRINTF_CONVERSION_X: {
            if (elt_type->getTypeID() != Type::IntegerTyID) {
              printf("Do not support type conversion between float and int in vector printf!\n");
              return false;
            }

            Type* elt_dst_type = NULL;
            if (slot.state.length_modifier == PRINTF_LM_L) {
              elt_dst_type = Type::getInt64Ty(elt_type->getContext());
            } else {
              elt_dst_type = Type::getInt32Ty(elt_type->getContext());
            }

            /* If the bits change, we need to consider the signed. */
            if (elt_type != elt_dst_type) {
              Value *II = NULL;
              for (int i = 0; i < vec_num; i++) {
                Value *vec = II ? II : UndefValue::get(VectorType::get(elt_dst_type, vec_num));
                Value *cv = ConstantInt::get(Type::getInt32Ty(elt_type->getContext()), i);
                Value *org = builder->CreateExtractElement(arg, cv);
                Value *cvt = builder->CreateIntCast(org, elt_dst_type, sign);
                II = builder->CreateInsertElement(vec, cvt, cv);
              }
              new_arg = II;
            }

            return true;
          }

          case PRINTF_CONVERSION_F:
          case PRINTF_CONVERSION_f:
          case PRINTF_CONVERSION_E:
          case PRINTF_CONVERSION_e:
          case PRINTF_CONVERSION_G:
          case PRINTF_CONVERSION_g:
          case PRINTF_CONVERSION_A:
          case PRINTF_CONVERSION_a:
            if (elt_type->getTypeID() != Type::DoubleTyID && elt_type->getTypeID() != Type::FloatTyID) {
              printf("Do not support type conversion between float and int in vector printf!\n");
              return false;
            }

            if (elt_type->getTypeID() != Type::FloatTyID) {
              Value *II = NULL;
              for (int i = 0; i < vec_num; i++) {
                Value *vec = II ? II : UndefValue::get(VectorType::get(Type::getFloatTy(elt_type->getContext()), vec_num));
                Value *cv = ConstantInt::get(Type::getInt32Ty(elt_type->getContext()), i);
                Value *org = builder->CreateExtractElement(arg, cv);
                Value* cvt  = builder->CreateFPCast(org, Type::getFloatTy(module->getContext()));
                II = builder->CreateInsertElement(vec, cvt, cv);
              }
              new_arg = II;
            }

            return true;

          default:
            return false;
        }
      }

      default:
        return false;
    }

    return false;
  }

  FunctionPass* createPrintfParserPass(ir::Unit &unit)
  {
    return new PrintfParser(unit);
  }
  char PrintfParser::ID = 0;

} // end namespace
