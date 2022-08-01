/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "sub_block_allocator.h"

SubBlockAllocatorStruct* SubBlockAllocator::Header() {
  return reinterpret_cast<SubBlockAllocatorStruct*>(&block_->Data()[sizeof(MetadataBlockHeader)]);
}
