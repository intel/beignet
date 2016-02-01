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
 *
 */

/**
 * \file printf.cpp
 *
 */

#include <stdarg.h>
#include "printf.hpp"
#include "ir/unit.hpp"

namespace gbe
{
  namespace ir
  {

    pthread_mutex_t PrintfSet::lock = PTHREAD_MUTEX_INITIALIZER;

    static void generatePrintfFmtString(PrintfState& state, std::string& str)
    {
      char num_str[16];
      str = "%";

      if (state.left_justified) {
        str += "-";
      }

      if (state.sign_symbol == 1) {
        str += "+";
      } else if (state.sign_symbol == 2) {
        str += " ";
      }

      if (state.alter_form) {
        str += "#";
      }

      if (state.zero_padding) {
        str += "0";
      }

      if (state.min_width >= 0) {
        snprintf(num_str, 16, "%d", state.min_width);
        str += num_str;
      }

      if (state.precision >= 0) {
        str += ".";
        snprintf(num_str, 16, "%d", state.precision);
        str += num_str;
      }

      switch (state.length_modifier) {
        case PRINTF_LM_HH:
          str += "hh";
          break;
        case PRINTF_LM_H:
          str += "h";
          break;
        case PRINTF_LM_L:
          str += "l";
          break;
        case PRINTF_LM_HL:
          str += "";
          break;
        default:
          assert(state.length_modifier == PRINTF_LM_NONE);
      }
    }

#define PRINT_SOMETHING(target_ty, conv)  do {                          \
      if (!vec_i)                                                       \
        pf_str = pf_str + std::string(#conv);                           \
      printf(pf_str.c_str(), log.getData<target_ty>());                 \
    } while (0)

    static void printOutOneStatement(PrintfSet::PrintfFmt& fmt, PrintfLog& log)
    {
      std::string pf_str = "";
      for (auto& slot : fmt) {
        if (slot.type == PRINTF_SLOT_TYPE_STRING) {
          printf("%s", slot.str.c_str());
          continue;
        }
        assert(slot.type == PRINTF_SLOT_TYPE_STATE);

        generatePrintfFmtString(slot.state, pf_str);

        int vec_num;
        vec_num = slot.state.vector_n > 0 ? slot.state.vector_n : 1;

        for (int vec_i = 0; vec_i < vec_num; vec_i++) {
          if (vec_i)
            printf(",");

          switch (slot.state.conversion_specifier) {
            case PRINTF_CONVERSION_D:
            case PRINTF_CONVERSION_I:
              if (slot.state.length_modifier == PRINTF_LM_L)
                PRINT_SOMETHING(uint64_t, d);
              else
                PRINT_SOMETHING(int, d);
              break;

            case PRINTF_CONVERSION_O:
              if (slot.state.length_modifier == PRINTF_LM_L)
                PRINT_SOMETHING(uint64_t, o);
              else
                PRINT_SOMETHING(int, o);
              break;
            case PRINTF_CONVERSION_U:
              if (slot.state.length_modifier == PRINTF_LM_L)
                PRINT_SOMETHING(uint64_t, u);
              else
                PRINT_SOMETHING(int, u);
              break;
            case PRINTF_CONVERSION_X:
              if (slot.state.length_modifier == PRINTF_LM_L)
                PRINT_SOMETHING(uint64_t, X);
              else
                PRINT_SOMETHING(int, X);
              break;
            case PRINTF_CONVERSION_x:
              if (slot.state.length_modifier == PRINTF_LM_L)
                PRINT_SOMETHING(uint64_t, x);
              else
                PRINT_SOMETHING(int, x);
              break;

            case PRINTF_CONVERSION_C:
              PRINT_SOMETHING(char, c);
              break;

            case PRINTF_CONVERSION_F:
              PRINT_SOMETHING(float, F);
              break;
            case PRINTF_CONVERSION_f:
              PRINT_SOMETHING(float, f);
              break;
            case PRINTF_CONVERSION_E:
              PRINT_SOMETHING(float, E);
              break;
            case PRINTF_CONVERSION_e:
              PRINT_SOMETHING(float, e);
              break;
            case PRINTF_CONVERSION_G:
              PRINT_SOMETHING(float, G);
              break;
            case PRINTF_CONVERSION_g:
              PRINT_SOMETHING(float, g);
              break;
            case PRINTF_CONVERSION_A:
              PRINT_SOMETHING(float, A);
              break;
            case PRINTF_CONVERSION_a:
              PRINT_SOMETHING(float, a);
              break;
            case PRINTF_CONVERSION_P:
              PRINT_SOMETHING(int, p);
              break;

            case PRINTF_CONVERSION_S:
              pf_str = pf_str + "s";
              printf(pf_str.c_str(), slot.state.str.c_str());
              break;

            default:
              assert(0);
              return;
          }
        }

      }
    }

    void PrintfSet::outputPrintf(void* buf_addr)
    {
      LockOutput lock;
      uint32_t totalSZ = ((uint32_t *)buf_addr)[0];
      char* p = (char*)buf_addr + sizeof(uint32_t);

      for (uint32_t parsed = 4; parsed < totalSZ; ) {
        PrintfLog log(p);
        GBE_ASSERT(fmts.find(log.statementNum) != fmts.end());
        printOutOneStatement(fmts[log.statementNum], log);
        parsed += log.size;
        p += log.size;
      }
    }
  } /* namespace ir */
} /* namespace gbe */

