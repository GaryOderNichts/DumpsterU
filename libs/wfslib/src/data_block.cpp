/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "data_block.h"

#include "device_encryption.h"
#include "metadata_block.h"
#include "structs.h"

DataBlock::DataBlock(const std::shared_ptr<DeviceEncryption>& device,
                     uint32_t block_number,
                     Block::BlockSize size_category,
                     uint32_t data_size,
                     uint32_t iv,
                     const DataBlockHash& data_hash,
                     bool encrypted)
    : Block(device, block_number, size_category, iv, encrypted, {data_size, std::byte{0}}), data_hash_(data_hash) {}

void DataBlock::Flush() {
  Block::Flush();
  // TODO: Write now we write two blocks for each block written, we need some caching with option to commit changes
  data_hash_.block->Flush();
}

std::shared_ptr<DataBlock> DataBlock::LoadBlock(const std::shared_ptr<DeviceEncryption>& device,
                                                uint32_t block_number,
                                                Block::BlockSize size_category,
                                                uint32_t data_size,
                                                uint32_t iv,
                                                const DataBlockHash& data_hash,
                                                bool encrypted) {
  auto block = std::make_shared<DataBlock>(device, block_number, size_category, data_size, iv, data_hash, encrypted);
  if (data_size) {
    // Fetch block only if have data
    block->Fetch();
  }
  return block;
}

std::span<std::byte> DataBlock::Hash() {
  return {&data_hash_.block->Data()[data_hash_.hash_offset], device_->DIGEST_SIZE};
}
