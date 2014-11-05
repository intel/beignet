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
 * \file image.hpp
 *
 */
#ifndef __GBE_IR_IMAGE_HPP__
#define __GBE_IR_IMAGE_HPP__

#include "ir/register.hpp"
#include "ir/instruction.hpp" // for ImageInfoKey
#include "sys/map.hpp"

extern "C" {
  struct ImageInfo;
}

namespace gbe {
namespace ir {

  class Context;
  /*! An image set is a set of images which are defined in kernel args.
   *  We use this set to gather the images here and allocate a unique index
   *  for each individual image. And that individual image could be used
   *  at backend to identify this image's location.
   */
  class ImageSet : public Serializable
  {
  public:
    /*! Append an image argument. */
    void append(Register imageReg, Context *ctx, uint8_t bti);
    /*! Append an image info slot. */
    void appendInfo(ImageInfoKey key, uint32_t offset);
    /*! Append an image info register. */
    Register appendInfo(ImageInfoKey, Context *ctx);
    /*! clear image info. */
    void clearInfo();
    /*! Get the image's index(actual location). */
    uint32_t getIdx(const Register imageReg) const;
    size_t getDataSize(void) { return regMap.size(); }
    size_t getDataSize(void) const { return regMap.size(); }

    int32_t getInfoOffset(ImageInfoKey key) const;
    void getData(struct ImageInfo *imageInfos) const;
    void operator = (const ImageSet& other) {
      regMap.insert(other.regMap.begin(), other.regMap.end());
    }

    bool empty() const { return regMap.empty(); }

    ImageSet(const ImageSet& other) : regMap(other.regMap.begin(), other.regMap.end()) { }
    ImageSet() {}
    ~ImageSet();

    static const uint32_t magic_begin = TO_MAGIC('I', 'M', 'A', 'G');
    static const uint32_t magic_end = TO_MAGIC('G', 'A', 'M', 'I');

    /* format:
       magic_begin     |
       regMap_size     |
       element_1       |
       ........        |
       element_n       |
       indexMap_size   |
       element_1       |
       ........        |
       element_n       |
       magic_end       |
       total_size
    */

    /*! Implements the serialization. */
    virtual size_t serializeToBin(std::ostream& outs);
    virtual size_t deserializeFromBin(std::istream& ins);
    virtual void printStatus(int indent, std::ostream& outs);

  private:
    map<Register, struct ImageInfo *> regMap;
    map<uint32_t, struct ImageInfo *> indexMap;
    map<uint16_t, Register> infoRegMap;
    GBE_CLASS(ImageSet);
  };
} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_IMAGE_HPP__ */
