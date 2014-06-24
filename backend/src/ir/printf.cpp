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
 */

/**
 * \file printf.cpp
 *
 */

#include <stdarg.h>
#include "printf.hpp"

namespace gbe
{
  namespace ir
  {

    pthread_mutex_t PrintfSet::lock = PTHREAD_MUTEX_INITIALIZER;

    uint32_t PrintfSet::append(PrintfFmt* fmt, Unit& unit)
    {
      fmts.push_back(*fmt);

      for (auto &f : fmts.back()) {
        if (f.type == PRINTF_SLOT_TYPE_STRING)
          continue;

        slots.push_back(&f);
      }

      /* Update the total size of size. */
      if (slots.size() > 0)
        sizeOfSize = slots.back()->state->out_buf_sizeof_offset
                     + getPrintfBufferElementSize(slots.size() - 1);

      return (uint32_t)fmts.size();
    }

    static void generatePrintfFmtString(PrintfState& state, std::string& str)
    {
      char num_str[16];
      str += "%";

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
      printf(pf_str.c_str(),                                            \
             ((target_ty *)((char *)buf_addr + slot.state->out_buf_sizeof_offset * \
                            global_wk_sz0 * global_wk_sz1 * global_wk_sz2)) \
             [(k*global_wk_sz0*global_wk_sz1 + j*global_wk_sz0 + i) * vec_num + vec_i]);\
    } while (0)


    void PrintfSet::outputPrintf(void* index_addr, void* buf_addr, size_t global_wk_sz0,
                                 size_t global_wk_sz1, size_t global_wk_sz2)
    {
      LockOutput lock;
      size_t i, j, k;
      std::string pf_str;
      int stmt = 0;

      for (auto &pf : fmts) {
        for (i = 0; i < global_wk_sz0; i++) {
          for (j = 0; j < global_wk_sz1; j++) {
            for (k = 0; k < global_wk_sz2; k++) {

              int flag = ((int *)index_addr)[stmt*global_wk_sz0*global_wk_sz1*global_wk_sz2
                                             + k*global_wk_sz0*global_wk_sz1 + j*global_wk_sz0 + i];
              if (flag) {
                for (auto &slot : pf) {
                  pf_str = "";
                  int vec_num;

                  if (slot.type == PRINTF_SLOT_TYPE_STRING) {
                    printf("%s", slot.str);
                    continue;
                  }
                  assert(slot.type == PRINTF_SLOT_TYPE_STATE);

                  generatePrintfFmtString(*slot.state, pf_str);

                  vec_num = slot.state->vector_n > 0 ? slot.state->vector_n : 1;

                  for (int vec_i = 0; vec_i < vec_num; vec_i++) {
                    if (vec_i)
                      printf(",");

                    switch (slot.state->conversion_specifier) {
                      case PRINTF_CONVERSION_D:
                      case PRINTF_CONVERSION_I:
                        PRINT_SOMETHING(int, d);
                        break;

                      case PRINTF_CONVERSION_O:
                        PRINT_SOMETHING(int, o);
                        break;
                      case PRINTF_CONVERSION_U:
                        PRINT_SOMETHING(int, u);
                        break;
                      case PRINTF_CONVERSION_X:
                        PRINT_SOMETHING(int, X);
                        break;
                      case PRINTF_CONVERSION_x:
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
                        printf(pf_str.c_str(), slot.state->str.c_str());
                        break;

                      default:
                        assert(0);
                        return;
                    }
                  }

                  pf_str = "";
                }
              }
            }
          }
        }
        stmt++;
      }
    }
  } /* namespace ir */
} /* namespace gbe */

