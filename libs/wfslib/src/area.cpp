/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "area.h"

#include "device.h"
#include "device_encryption.h"
#include "directory.h"
#include "metadata_block.h"
#include "structs.h"
#include "wfs.h"

WfsAreaHeader* Area::Data() {
  if (header_block_->BlockNumber() == 0) {
    return reinterpret_cast<WfsAreaHeader*>(&header_block_->Data()[sizeof(MetadataBlockHeader) + sizeof(WfsHeader)]);
  } else {
    return reinterpret_cast<WfsAreaHeader*>(&header_block_->Data()[sizeof(MetadataBlockHeader)]);
  }
}

WfsHeader* Area::WfsData() {
  if (header_block_->BlockNumber() == 0) {
    return reinterpret_cast<WfsHeader*>(&header_block_->Data()[sizeof(MetadataBlockHeader)]);
  } else {
    return NULL;
  }
}

Area::Area(const std::shared_ptr<DeviceEncryption>& device,
           const std::shared_ptr<Area>& root_area,
           const std::shared_ptr<MetadataBlock>& block,
           const std::string& root_directory_name,
           const AttributesBlock& root_directory_attributes)
    : device_(device),
      root_area_(root_area),
      header_block_(block),
      root_directory_name_(root_directory_name),
      root_directory_attributes_(root_directory_attributes) {}

std::shared_ptr<Area> Area::LoadRootArea(const std::shared_ptr<DeviceEncryption>& device) {
  std::shared_ptr<MetadataBlock> block;
  try {
    block = MetadataBlock::LoadBlock(device, 0, Block::BlockSize::Basic, 0);
  } catch (const Block::BadHash&) {
    block = MetadataBlock::LoadBlock(device, 0, Block::BlockSize::Regular, 0);
  }
  return std::make_shared<Area>(
      device, nullptr, block, "",
      AttributesBlock{block, sizeof(MetadataBlockHeader) + offsetof(WfsHeader, root_area_attributes)});
}

std::shared_ptr<Directory> Area::GetDirectory(uint32_t block_number,
                                              const std::string& name,
                                              const AttributesBlock& attributes) {
  // TODO: Cache
  return std::make_shared<Directory>(name, attributes, shared_from_this(), GetMetadataBlock(block_number));
}

std::shared_ptr<Directory> Area::GetRootDirectory() {
  return GetDirectory(Data()->root_directory_block_number.value(), root_directory_name_, root_directory_attributes_);
}

std::shared_ptr<Area> Area::GetArea(uint32_t block_number,
                                    const std::string& root_directory_name,
                                    const AttributesBlock& root_directory_attributes,
                                    Block::BlockSize size) {
  return std::make_shared<Area>(device_, root_area_ ? root_area_ : shared_from_this(),
                                GetMetadataBlock(block_number, size), root_directory_name, root_directory_attributes);
}

std::shared_ptr<MetadataBlock> Area::GetMetadataBlock(uint32_t block_number) {
  return GetMetadataBlock(block_number, static_cast<Block::BlockSize>(Data()->log2_block_size.value()));
}

uint32_t Area::IV(uint32_t block_number) {
  return (Data()->iv.value() ^ (root_area_ ? root_area_.get() : this)->WfsData()->iv.value()) +
         (ToBasicBlockNumber(block_number) << (Block::BlockSize::Basic - device_->GetDevice()->Log2SectorSize()));
}

std::shared_ptr<MetadataBlock> Area::GetMetadataBlock(uint32_t block_number, Block::BlockSize size) {
  return MetadataBlock::LoadBlock(device_, header_block_->BlockNumber() + ToBasicBlockNumber(block_number), size,
                                  IV(block_number));
}

std::shared_ptr<DataBlock> Area::GetDataBlock(uint32_t block_number,
                                              Block::BlockSize size,
                                              uint32_t data_size,
                                              const DataBlock::DataBlockHash& data_hash,
                                              bool encrypted) {
  return DataBlock::LoadBlock(device_, header_block_->BlockNumber() + ToBasicBlockNumber(block_number), size, data_size,
                              IV(block_number), data_hash, encrypted);
}

uint32_t Area::ToBasicBlockNumber(uint32_t block_number) {
  return block_number << (Data()->log2_block_size.value() - Block::BlockSize::Basic);
}

size_t Area::GetDataBlockLog2Size() {
  return Data()->log2_block_size.value();
}

uint32_t Area::BlockNumber(const std::shared_ptr<Block>& block) {
  return (block->BlockNumber() - header_block_->BlockNumber()) >>
         (Data()->log2_block_size.value() - Block::BlockSize::Basic);
}
