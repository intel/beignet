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

#define PRINT_SOMETHING(target_ty, conv)  do {                          \
      pf_str = pf_str + std::string(#conv);                             \
      printf(pf_str.c_str(),                                            \
             ((target_ty *)((char *)buf_addr + slot.state->out_buf_sizeof_offset * \
                            global_wk_sz0 * global_wk_sz1 * global_wk_sz2)) \
             [k*global_wk_sz0*global_wk_sz1 + j*global_wk_sz0 + i]);    \
      pf_str = "";                                                      \
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
              int flag = ((int *)index_addr)[stmt*global_wk_sz0*global_wk_sz1*global_wk_sz2 + k*global_wk_sz0*global_wk_sz1 + j*global_wk_sz0 + i];
              if (flag) {
                pf_str = "";
                for (auto &slot : pf) {
                  if (slot.type == PRINTF_SLOT_TYPE_STRING) {
                    pf_str = pf_str + std::string(slot.str);
                    continue;
                  }
                  assert(slot.type == PRINTF_SLOT_TYPE_STATE);

                  switch (slot.state->conversion_specifier) {
                    case PRINTF_CONVERSION_D:
                    case PRINTF_CONVERSION_I:
                      PRINT_SOMETHING(int, %d);
                      break;
                    case PRINTF_CONVERSION_C:
                      PRINT_SOMETHING(char, %c);
                      break;

                    case PRINTF_CONVERSION_F:
                    case PRINTF_CONVERSION_f:
                      if (slot.state->conversion_specifier == PRINTF_CONVERSION_F)
                        PRINT_SOMETHING(float, %F);
                      else
                        PRINT_SOMETHING(float, %f);
                      break;

                    default:
                      assert(0);
                      return;
                  }
                }

                if (pf_str != "") {
                  printf("%s", pf_str.c_str());
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

