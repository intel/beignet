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
#include "constant.hpp"

namespace gbe {
namespace ir {

  void ConstantSet::append(const char *data,
                           const std::string &name,
                           uint32_t size,
                           uint32_t alignment)
  {
    const uint32_t offset = ALIGN(this->data.size(), alignment);
    const uint32_t padding = offset - this->data.size();
    const Constant constant(name, size, alignment, offset);
    constants.push_back(constant);
    for (uint32_t i = 0; i < padding; ++i) this->data.push_back(0);
    for (uint32_t i = 0; i < size; ++i) this->data.push_back(data[i]);
  }

#define OUT_UPDATE_SZ(elt) SERIALIZE_OUT(elt, outs, ret_size)
#define IN_UPDATE_SZ(elt) DESERIALIZE_IN(elt, ins, total_size)

  size_t ConstantSet::serializeToBin(std::ostream& outs) {
    size_t ret_size = 0;

    OUT_UPDATE_SZ(magic_begin);

    /* output the const data. */
    OUT_UPDATE_SZ((data.size()*sizeof(char)));
    if(data.size() > 0) {
      outs.write(data.data(), data.size()*sizeof(char));
      ret_size += data.size()*sizeof(char);
    }

    OUT_UPDATE_SZ(constants.size());
    for (auto const &cnst : constants) {
      size_t bytes = sizeof(cnst.getName().size())        //name length self
                     + cnst.getName().size()*sizeof(char) //name
                     + sizeof(cnst.getSize())             //size
                     + sizeof(cnst.getAlignment())        //alignment
                     + sizeof(cnst.getOffset());	        //offset
      OUT_UPDATE_SZ(bytes);

      OUT_UPDATE_SZ(cnst.getName().size());
      outs.write(cnst.getName().c_str(), cnst.getName().size());
      ret_size += sizeof(char)*cnst.getName().size();
      OUT_UPDATE_SZ(cnst.getSize());
      OUT_UPDATE_SZ(cnst.getAlignment());
      OUT_UPDATE_SZ(cnst.getOffset());
    }

    OUT_UPDATE_SZ(magic_end);
    OUT_UPDATE_SZ(ret_size);

    return ret_size;
  }

  size_t ConstantSet::deserializeFromBin(std::istream& ins) {
    size_t total_size = 0;
    size_t global_data_sz = 0;
    size_t const_num;
    uint32_t magic;

    IN_UPDATE_SZ(magic);
    if (magic != magic_begin)
      return 0;

    IN_UPDATE_SZ(global_data_sz);
    for (size_t i = 0; i < global_data_sz; i++) {
      char elt;
      IN_UPDATE_SZ(elt);
      data.push_back(elt);
    }

    IN_UPDATE_SZ(const_num);
    for (size_t i = 0; i < const_num; i++) {
      size_t bytes;
      IN_UPDATE_SZ(bytes);

      size_t name_len;
      IN_UPDATE_SZ(name_len);

      char* c_name = new char[name_len+1];
      ins.read(c_name, name_len);
      total_size += sizeof(char)*name_len;
      c_name[name_len] = 0;

      uint32_t size, align, offset;
      IN_UPDATE_SZ(size);
      IN_UPDATE_SZ(align);
      IN_UPDATE_SZ(offset);

      ir::Constant constant(c_name, size, align, offset);
      constants.push_back(constant);

      delete[] c_name;

      /* Saint check */
      if (bytes != sizeof(name_len) + sizeof(char)*name_len + sizeof(size)
              + sizeof(align) + sizeof(offset))
        return 0;
    }

    IN_UPDATE_SZ(magic);
    if (magic != magic_end)
      return 0;

    size_t total_bytes;
    IN_UPDATE_SZ(total_bytes);
    if (total_bytes + sizeof(total_size) != total_size)
      return 0;

    return total_size;
  }

} /* namespace ir */
} /* namespace gbe */

