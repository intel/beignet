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
 * \file printf.hpp
 *
 */
#ifndef __GBE_IR_PRINTF_HPP__
#define __GBE_IR_PRINTF_HPP__

#include <string.h>
#include "sys/map.hpp"
#include "sys/vector.hpp"

namespace gbe
{
  namespace ir
  {
    class Unit;

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

      PrintfState(void) {
        left_justified = 0;
        sign_symbol = 0;
        alter_form = 0;
        zero_padding = 0;
        vector_n = 0;
        min_width = 0;
        precision = 0;
        length_modifier = 0;
        conversion_specifier = 0;
        out_buf_sizeof_offset = 0;
      }

      PrintfState(const PrintfState & other) {
        left_justified = other.left_justified;
        sign_symbol = other.sign_symbol;
        alter_form = other.alter_form;
        zero_padding = other.zero_padding;
        vector_n = other.vector_n;
        min_width = other.min_width;
        precision = other.precision;
        length_modifier = other.length_modifier;
        conversion_specifier = other.conversion_specifier;
        out_buf_sizeof_offset = other.out_buf_sizeof_offset;
        str = other.str;
      }
    };

    enum {
      PRINTF_SLOT_TYPE_NONE,
      PRINTF_SLOT_TYPE_STRING,
      PRINTF_SLOT_TYPE_STATE
    };

    struct PrintfSlot {
      uint32_t type;
      std::string str;
      PrintfState state;

      PrintfSlot(void) {
        type = PRINTF_SLOT_TYPE_NONE;
      }

      PrintfSlot(std::string& s) : str(s) {
        type = PRINTF_SLOT_TYPE_STRING;
      }

      PrintfSlot(PrintfState& st) {
        type = PRINTF_SLOT_TYPE_STATE;
        state = st;
      }

      PrintfSlot(const PrintfSlot & other) {
        if (other.type == PRINTF_SLOT_TYPE_STRING) {
          type = PRINTF_SLOT_TYPE_STRING;
          str = other.str;
        } else if (other.type == PRINTF_SLOT_TYPE_STATE) {
          type = PRINTF_SLOT_TYPE_STATE;
          state = other.state;
        } else {
          type = PRINTF_SLOT_TYPE_NONE;
        }
      }

      ~PrintfSlot(void) {
      }
    };

    struct PrintfLog {
      uint32_t magic;  // 0xAABBCCDD as magic for ASSERT.
      uint32_t size;  // Size of this printf log, include header.
      uint32_t statementNum; // which printf within one kernel.
      const char* content;

      PrintfLog(const char* p) {
        GBE_ASSERT(*((uint32_t *)p) == 0xAABBCCDD);
        magic = *((uint32_t *)p);
        p += sizeof(uint32_t);
        size = *((uint32_t *)p);
        p += sizeof(uint32_t);
        statementNum = *((uint32_t *)p);
        p += sizeof(uint32_t);
        content = p;
      }

      template <typename T>
      T getData(void) {
        T D = *((T *)content);
        content += sizeof(T);
        return D;
      }
    };

    class Context;

    class PrintfSet //: public Serializable
    {
    public:
      PrintfSet(const PrintfSet& other) {
        fmts = other.fmts;
        btiBuf = other.btiBuf;
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

      void append(uint32_t num, PrintfFmt* fmt) {
        GBE_ASSERT(fmts.find(num) == fmts.end());
        fmts.insert(std::pair<uint32_t, PrintfFmt>(num, *fmt));
      }

      uint32_t getPrintfNum(void) const {
        return fmts.size();
      }

      void setBufBTI(uint8_t b)      { btiBuf = b; }
      uint8_t getBufBTI() const      { return btiBuf; }

      uint32_t getPrintfBufferElementSize(uint32_t i) {
        PrintfSlot slot;
        int vec_num = 1;
        if (slot.state.vector_n > 0) {
          vec_num = slot.state.vector_n;
        }

        assert(vec_num > 0 && vec_num <= 16);

        switch (slot.state.conversion_specifier) {
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

      void outputPrintf(void* buf_addr);

    private:
      std::map<uint32_t, PrintfFmt> fmts;
      friend struct LockOutput;
      uint8_t btiBuf;
      static pthread_mutex_t lock;
      GBE_CLASS(PrintfSet);
    };
  } /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_PRINTF_HPP__ */
