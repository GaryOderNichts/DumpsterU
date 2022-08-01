/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "block.h"

#include <boost/format.hpp>
#include "device.h"
#include "device_encryption.h"
#include "utils.h"

Block::BadHash::BadHash(uint32_t block_number)
    : block_number_(block_number), msg_((boost::format("Bad hash for block 0x%08X") % block_number_).str()) {}
char const* Block::BadHash::what() const noexcept {
  return msg_.c_str();
}

Block::Block(const std::shared_ptr<DeviceEncryption>& device,
             uint32_t block_number,
             Block::BlockSize size_category,
             uint32_t iv,
             bool encrypted,
             std::vector<std::byte>&& data)
    : device_(device),
      block_number_(block_number),
      size_category_(size_category),
      iv_(iv),
      encrypted_(encrypted),
      data_(data) {
  Resize(data_.size());
}

void Block::Resize(size_t new_size) {
  // Ensure that block data is aligned to device sectors
  new_size = div_ceil(new_size, device_->GetDevice()->SectorSize()) * device_->GetDevice()->SectorSize();
  if (new_size != data_.size())
    data_.resize(new_size, std::byte{0});
}

void Block::Fetch(bool check_hash) {
  device_->ReadBlock(ToDeviceSector(block_number_), data_, iv_, encrypted_);
  if (check_hash && !device_->CheckHash(data_, Hash()))
    throw Block::BadHash(block_number_);
}

void Block::Flush() {
  device_->CalculateHash(data_, Hash());
  device_->WriteBlock(ToDeviceSector(block_number_), data_, iv_, encrypted_);
}

uint32_t Block::ToDeviceSector(uint32_t block_number) {
  return block_number << (BlockSize::Basic - device_->GetDevice()->Log2SectorSize());
}
