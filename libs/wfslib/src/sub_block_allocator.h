/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <memory>
#include "metadata_block.h"
#include "structs.h"

class MetadataBlock;
struct SubBlockAllocatorStruct;

class SubBlockAllocator {
 public:
  SubBlockAllocator(const std::shared_ptr<MetadataBlock>& block) : block_(block) {}

  template <typename T>
  T* GetNode(uint16_t offset) {
    return reinterpret_cast<T*>(&block_->Data()[offset]);
  }

  template <typename T>
  T* GetRootNode() {
    return GetNode<T>(Header()->root.value());
  }

 private:
  std::shared_ptr<MetadataBlock> block_;

  // TODO uint16_t Alloc(uint16_t size);
  // TODO void Free(uint16_t offset);

  SubBlockAllocatorStruct* Header();
};
