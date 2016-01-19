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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/**
 * \file reloc.cpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_RELOC_HPP__
#define __GBE_IR_RELOC_HPP__

#include "sys/vector.hpp"
#include <string.h>

namespace gbe {
namespace ir {


  /*! Complete unit of compilation. It contains a set of functions and a set of
   *  RelocEntry the functions may refer to.
   */
  struct RelocEntry {
    RelocEntry(unsigned int rO, unsigned int dO):
      refOffset(rO),
      defOffset(dO) {}

    unsigned int refOffset;
    unsigned int defOffset;
  };

  class RelocTable : public NonCopyable, public Serializable
  {
    public:
      void addEntry(unsigned refOffset, unsigned defOffset) {
        entries.push_back(RelocEntry(refOffset, defOffset));
      }
      RelocTable() : Serializable() {}
      RelocTable(const RelocTable& other) : Serializable(other),
                                            entries(other.entries) {}
      uint32_t getCount() { return entries.size(); }
      void getData(char *p) {
        if (entries.size() > 0 && p)
          memcpy(p, entries.data(), entries.size()*sizeof(RelocEntry));
      }
    static const uint32_t magic_begin = TO_MAGIC('R', 'E', 'L', 'C');
    static const uint32_t magic_end = TO_MAGIC('C', 'L', 'E', 'R');

    /* format:
       magic_begin       |
       reloc_table_size  |
       entry_0_refOffset |
       entry_0_defOffset |
       entry_1_refOffset |
       entry_1_defOffset |
       ........         |
       entry_n_refOffset |
       entry_n_defOffset |
       magic_end       |
       total_size
    */

    /*! Implements the serialization. */
    virtual uint32_t serializeToBin(std::ostream& outs);
    virtual uint32_t deserializeFromBin(std::istream& ins);
    private:
      vector<RelocEntry> entries;
      GBE_CLASS(RelocTable);
  };

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_RELOC_HPP__ */

