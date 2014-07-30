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
 * \file printf.hpp
 *
 */
#ifndef __GBE_IR_PRINTF_HPP__
#define __GBE_IR_PRINTF_HPP__

#include <string.h>
#include "sys/map.hpp"
#include "sys/vector.hpp"
#include "unit.hpp"

namespace gbe
{
  namespace ir
  {

    /* Things about printf info. */
    enum {
      PRINTF_LM_NONE,
      PRINTF_LM_HH,
      PRINTF_LM_H,
      PRINTF_LM_L,
      PRINTF_LM_HL,
    };

    enum {
      PRINTF_CONVERSION_INVALID,
      PRINTF_CONVERSION_D,
      PRINTF_CONVERSION_I,
      PRINTF_CONVERSION_O,
      PRINTF_CONVERSION_U,
      PRINTF_CONVERSION_X,
      PRINTF_CONVERSION_x,
      PRINTF_CONVERSION_F,
      PRINTF_CONVERSION_f,
      PRINTF_CONVERSION_E,
      PRINTF_CONVERSION_e,
      PRINTF_CONVERSION_G,
      PRINTF_CONVERSION_g,
      PRINTF_CONVERSION_A,
      PRINTF_CONVERSION_a,
      PRINTF_CONVERSION_C,
      PRINTF_CONVERSION_S,
      PRINTF_CONVERSION_P
    };

    struct PrintfState {
      char left_justified;
      char sign_symbol; //0 for nothing, 1 for sign, 2 for space.
      char alter_form;
      char zero_padding;
      char vector_n;
      int min_width;
      int precision;
      int length_modifier;
      char conversion_specifier;
      int out_buf_sizeof_offset;  // Should *global_total_size to get the full offset.
      std::string str;            //if %s, the string store here.
    };

    enum {
      PRINTF_SLOT_TYPE_NONE,
      PRINTF_SLOT_TYPE_STRING,
      PRINTF_SLOT_TYPE_STATE
    };

    struct PrintfSlot {
      int type;
      union {
        char* str;
        PrintfState* state;
        void *ptr;
      };

      PrintfSlot(void) {
        type = PRINTF_SLOT_TYPE_NONE;
        ptr = NULL;
      }

      PrintfSlot(const char * s) {
        type = PRINTF_SLOT_TYPE_STRING;
        int len = strlen(s);
        str = (char*)malloc((len + 1) * sizeof(char));
        memcpy(str, s, (len + 1) * sizeof(char));
        str[len] = 0;
      }

      PrintfSlot(PrintfState * st) {
        type = PRINTF_SLOT_TYPE_STATE;
        state = (PrintfState *)malloc(sizeof(PrintfState));
        memcpy(state, st, sizeof(PrintfState));
      }

      PrintfSlot(const PrintfSlot & other) {
        if (other.type == PRINTF_SLOT_TYPE_STRING) {
          int len = strlen(other.str);
          str = (char*)malloc((len + 1) * sizeof(char));
          memcpy(str, other.str, (len + 1) * sizeof(char));
          str[len] = 0;
          type = PRINTF_SLOT_TYPE_STRING;
        } else if (other.type == PRINTF_SLOT_TYPE_STATE) {
          type = PRINTF_SLOT_TYPE_STATE;
          state = (PrintfState *)malloc(sizeof(PrintfState));
          memcpy(state, other.state, sizeof(PrintfState));
        } else {
          type = PRINTF_SLOT_TYPE_NONE;
          ptr = NULL;
        }
      }

      PrintfSlot(PrintfSlot && other) {
        void *p = other.ptr;
        type = other.type;
        other.ptr = ptr;
        ptr = p;
      }

      ~PrintfSlot(void) {
        if (ptr)
          free(ptr);
      }
    };

    class Context;

    class PrintfSet //: public Serializable
    {
    public:
      PrintfSet(const PrintfSet& other) {
        for (auto &f : other.fmts) {
          fmts.push_back(f);
        }

        for (auto &s : other.slots) {
          slots.push_back(s);
        }

        sizeOfSize = other.sizeOfSize;
        btiBuf = other.btiBuf;
        btiIndexBuf = other.btiIndexBuf;
      }

      PrintfSet(void) = default;

      struct LockOutput {
        LockOutput(void) {
          pthread_mutex_lock(&lock);
        }

        ~LockOutput(void) {
          pthread_mutex_unlock(&lock);
        }
      };

      typedef vector<PrintfSlot> PrintfFmt;
      uint32_t append(PrintfFmt* fmt, Unit &unit);

      uint32_t getPrintfNum(void) const {
        return fmts.size();
      }

      uint32_t getPrintfSizeOfSize(void) const {
        return sizeOfSize;
      }

      void setBufBTI(uint8_t b)      { btiBuf = b; }
      void setIndexBufBTI(uint8_t b) { btiIndexBuf = b; }
      uint8_t getBufBTI() const      { return btiBuf; }
      uint8_t getIndexBufBTI() const { return btiIndexBuf; }

      uint32_t getPrintfBufferElementSize(uint32_t i) {
        PrintfSlot* slot = slots[i];
        int vec_num = 1;
        if (slot->state->vector_n > 0) {
          vec_num = slot->state->vector_n;
        }

        assert(vec_num > 0 && vec_num <= 16);

        switch (slot->state->conversion_specifier) {
          case PRINTF_CONVERSION_I:
          case PRINTF_CONVERSION_D:
          case PRINTF_CONVERSION_O:
          case PRINTF_CONVERSION_U:
          case PRINTF_CONVERSION_X:
          case PRINTF_CONVERSION_x:
          case PRINTF_CONVERSION_P:
          /* Char will be aligned to sizeof(int) here. */
          case PRINTF_CONVERSION_C:
            return (uint32_t)(sizeof(int) * vec_num);
          case PRINTF_CONVERSION_E:
          case PRINTF_CONVERSION_e:
          case PRINTF_CONVERSION_F:
          case PRINTF_CONVERSION_f:
          case PRINTF_CONVERSION_G:
          case PRINTF_CONVERSION_g:
          case PRINTF_CONVERSION_A:
          case PRINTF_CONVERSION_a:
            return (uint32_t)(sizeof(float) * vec_num);
          case PRINTF_CONVERSION_S:
            return (uint32_t)0;
          default:
            break;
        }
        assert(0);
        return 0;
      }

      void outputPrintf(void* index_addr, void* buf_addr, size_t global_wk_sz0,
                        size_t global_wk_sz1, size_t global_wk_sz2);

    private:
      vector<PrintfFmt> fmts;
      vector<PrintfSlot*> slots;
      uint32_t sizeOfSize; // Total sizeof size.
      friend struct LockOutput;
      uint8_t btiBuf;
      uint8_t btiIndexBuf;
      static pthread_mutex_t lock;
      GBE_CLASS(PrintfSet);
    };
  } /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_PRINTF_HPP__ */
