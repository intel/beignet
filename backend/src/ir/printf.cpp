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
 * \file sampler.cpp
 *
 */

#include <stdarg.h>
#include "printf.hpp"
#include "ocl_common_defines.h"

namespace gbe
{
  namespace ir
  {
    uint32_t PrintfSet::append(PrintfFmt* fmt, Unit& unit)
    {
      fmts.push_back(*fmt);

      for (auto &f : fmts.back()) {
        if (f.type == PRINTF_SLOT_TYPE_STRING)
          continue;

        slots.push_back(&f);
      }

      /* Update the total size of size. */
      sizeOfSize = slots.back()->state->out_buf_sizeof_offset
                   + getPrintfBufferElementSize(slots.size() - 1);

      return (uint32_t)fmts.size();
    }

    /* ugly here. We can not build the va_list dynamically:(
       And I have tried
       va_list arg; arg = some_ptr;
       This works very OK on 32bits platform but can not even
       pass the compiling in the 64bits platform.
       sizeof(arg) = 4 in 32bits platform but
       sizeof(arg) = 24 in 64bits platform.
       We can not assume the platform here. */
    void vfprintf_wrap(std::string& fmt, vector<int>& contents)
    {
      int* ptr = NULL;
      size_t num = contents.size() < 32 ? contents.size() : 32;
      ptr = (int *)calloc(32, sizeof(int)); //should be enough
      for (size_t i = 0; i < num; i++) {
        ptr[i] = contents[i];
      }

      printf(fmt.c_str(), ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7],
             ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15], ptr[16],
             ptr[17], ptr[18], ptr[19], ptr[20], ptr[21], ptr[22], ptr[23], ptr[24], ptr[25],
             ptr[26], ptr[27], ptr[28], ptr[29], ptr[30], ptr[31]);
      free(ptr);
    }

    void PrintfSet::outputPrintf(void* index_addr, void* buf_addr, size_t global_wk_sz0,
                                 size_t global_wk_sz1, size_t global_wk_sz2)
    {
      size_t i, j, k;
      std::string pf_str;
      vector<int>* contents = NULL;
      for (auto &pf : fmts) {
        for (i = 0; i < global_wk_sz0; i++) {
          for (j = 0; j < global_wk_sz1; j++) {
            for (k = 0; k < global_wk_sz2; k++) {
              int flag = ((int *)index_addr)[k*global_wk_sz0*global_wk_sz1 + j*global_wk_sz0 + i];
              if (flag) {
                pf_str = "";
                contents = new vector<int>();
                for (auto &slot : pf) {
                  if (slot.type == PRINTF_SLOT_TYPE_STRING) {
                    pf_str = pf_str + std::string(slot.str);
                    continue;
                  }
                  assert(slot.type == PRINTF_SLOT_TYPE_STATE);

                  switch (slot.state->conversion_specifier) {
                    case PRINTF_CONVERSION_D:
                    case PRINTF_CONVERSION_I:
                      contents->push_back(((int *)((char *)buf_addr + slot.state->out_buf_sizeof_offset
                                                   * global_wk_sz0 * global_wk_sz1 * global_wk_sz2))
                                          [k*global_wk_sz0*global_wk_sz1 + j*global_wk_sz0 + i]);
                      pf_str = pf_str + std::string("%d");
                      break;
                    default:
                      assert(0);
                      return;
                  }
                }

                vfprintf_wrap(pf_str, *contents);
                delete contents;
              }
            }
          }
        }
      }
    }
  } /* namespace ir */
} /* namespace gbe */

