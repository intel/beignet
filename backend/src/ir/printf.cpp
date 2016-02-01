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
      char *ptr = ((char *)buf_addr + sizeOfSize * global_wk_sz0 * global_wk_sz1 * global_wk_sz2 * n \
                   + slot.state->out_buf_sizeof_offset *                \
                   global_wk_sz0 * global_wk_sz1 * global_wk_sz2);      \
      target_ty* obj_ptr = ((target_ty *)ptr) + (k*global_wk_sz0*global_wk_sz1 + j*global_wk_sz0 + i) * vec_num + vec_i; \
      if ((char *)obj_ptr + sizeof(target_ty) > (char *)buf_addr + output_sz) {            \
        printf("\n\n!!!The printf message is out of range because of the limited buffer, ignore.\n"); \
        return;                                                         \
      }                                                                 \
      printf(pf_str.c_str(),  *obj_ptr);                                \
    } while (0)


    void PrintfSet::outputPrintf(void* index_addr, void* buf_addr, size_t global_wk_sz0,
                                 size_t global_wk_sz1, size_t global_wk_sz2, size_t output_sz)
    {
      LockOutput lock;
    }
  } /* namespace ir */
} /* namespace gbe */

