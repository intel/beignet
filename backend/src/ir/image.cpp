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
 * \file image.cpp
 *
 */
#include "image.hpp"
#include "context.hpp"
#include "ocl_common_defines.h"
#include "backend/program.h"

namespace gbe {
namespace ir {

  static uint32_t getInfoOffset4Type(struct ImageInfo *imageInfo, int type)
  {
    switch (type) {
      case GetImageInfoInstruction::WIDTH: return imageInfo->wSlot;
      case GetImageInfoInstruction::HEIGHT: return imageInfo->hSlot;
      default:
        NOT_IMPLEMENTED;
    }
    return 0;
  }

  static uint32_t setInfoOffset4Type(struct ImageInfo *imageInfo, int type, uint32_t offset)
  {
    switch (type) {
      case GetImageInfoInstruction::WIDTH: imageInfo->wSlot = offset; break;
      case GetImageInfoInstruction::HEIGHT: imageInfo->hSlot = offset; break;
      default:
        NOT_IMPLEMENTED;
    }
    return 0;
  }

  void ImageSet::appendInfo(ImageInfoKey key, uint32_t offset)
  {
    auto it = indexMap.find(key.index);
    assert(it != indexMap.end());
    struct ImageInfo *imageInfo = it->second;
    setInfoOffset4Type(imageInfo, key.type, offset);
  }

  void ImageSet::append(Register imageReg, Context *ctx)
  {
    ir::FunctionArgument *arg =  ctx->getFunction().getArg(imageReg);
    GBE_ASSERTM(arg && arg->type == ir::FunctionArgument::IMAGE, "Append an invalid reg to image set.");
    GBE_ASSERTM(regMap.find(imageReg) == regMap.end(), "Append the same image reg twice.");

    int32_t id = ctx->getFunction().getArgID(arg);
    struct ImageInfo *imageInfo = GBE_NEW(struct ImageInfo);
    imageInfo->arg_idx = id;
    imageInfo->idx = regMap.size() + gbe_get_image_base_index();
    imageInfo->wSlot = -1;
    imageInfo->hSlot = -1;
    imageInfo->depthSlot = -1;
    imageInfo->dataTypeSlot = -1;
    imageInfo->channelOrderSlot = -1;
    imageInfo->dimOrderSlot = -1;
    regMap.insert(std::make_pair(imageReg, imageInfo));
    indexMap.insert(std::make_pair(imageInfo->idx, imageInfo));
  }

  const int32_t ImageSet::getInfoOffset(ImageInfoKey key) const
  {
    auto it = indexMap.find(key.index);
    if (it == indexMap.end())
      return -1;
    struct ImageInfo *imageInfo = it->second;
    return getInfoOffset4Type(imageInfo, key.type);
  }

  const uint32_t ImageSet::getIdx(const Register imageReg) const
  {
    auto it = regMap.find(imageReg);
    GBE_ASSERT(it != regMap.end());
    return it->second->idx;
  }

  void ImageSet::getData(struct ImageInfo *imageInfos) const {
      for(auto &it : regMap)
        imageInfos[it.second->idx - gbe_get_image_base_index()] = *it.second;
  }

  ImageSet::~ImageSet() {
    for(auto &it : regMap)
      GBE_DELETE(it.second);
  }

} /* namespace ir */
} /* namespace gbe */
