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
 * \file constant.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "reloc.hpp"

namespace gbe {
namespace ir {

#define OUT_UPDATE_SZ(elt) SERIALIZE_OUT(elt, outs, ret_size)
#define IN_UPDATE_SZ(elt) DESERIALIZE_IN(elt, ins, total_size)

  /*! Implements the serialization. */
  uint32_t RelocTable::serializeToBin(std::ostream& outs) {
    uint32_t ret_size = 0;
    uint32_t sz = 0;

    OUT_UPDATE_SZ(magic_begin);

    sz = getCount();
    OUT_UPDATE_SZ(sz);
    RelocEntry entry(0, 0);
    for (uint32_t i = 0; i < sz; ++i) {
      entry = entries[i];
      OUT_UPDATE_SZ(entry.refOffset);
      OUT_UPDATE_SZ(entry.defOffset);
    }

    OUT_UPDATE_SZ(magic_end);
    OUT_UPDATE_SZ(ret_size);

    return ret_size;
  }

  uint32_t RelocTable::deserializeFromBin(std::istream& ins) {
    uint32_t total_size = 0;
    uint32_t magic;
    uint32_t refOffset;
    uint32_t defOffset;
    uint32_t sz = 0;

    IN_UPDATE_SZ(magic);
    if (magic != magic_begin)
      return 0;

    IN_UPDATE_SZ(sz); //regMap
    for (uint32_t i = 0; i < sz; i++) {
      IN_UPDATE_SZ(refOffset);
      IN_UPDATE_SZ(defOffset);
      addEntry(refOffset, defOffset);
    }

    IN_UPDATE_SZ(magic);
    if (magic != magic_end)
      return 0;

    uint32_t total_bytes;
    IN_UPDATE_SZ(total_bytes);
    if (total_bytes + sizeof(total_size) != total_size)
      return 0;

    return total_size;
  }

} /* namespace ir */
} /* namespace gbe */

