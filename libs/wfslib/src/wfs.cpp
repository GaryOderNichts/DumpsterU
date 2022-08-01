/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "wfs.h"

#include <bit>
#include <filesystem>
#include "area.h"
#include "device_encryption.h"
#include "directory.h"
#include "file_device.h"
#include "metadata_block.h"
#include "structs.h"

Wfs::Wfs(const std::shared_ptr<Device>& device, const std::span<std::byte>& key)
    : device_(std::make_shared<DeviceEncryption>(device, key)) {
  // Read first area
  root_area_ = Area::LoadRootArea(device_);
}

std::shared_ptr<WfsItem> Wfs::GetObject(const std::string& filename) {
  if (filename == "/")
    return GetDirectory("/");
  std::filesystem::path path(filename);
  auto dir = GetDirectory(path.parent_path().string());
  if (!dir)
    return nullptr;
  return dir->GetObject(path.filename().string());
}

std::shared_ptr<File> Wfs::GetFile(const std::string& filename) {
  std::filesystem::path path(filename);
  auto dir = GetDirectory(path.parent_path().string());
  if (!dir)
    return nullptr;
  return dir->GetFile(path.filename().string());
}

std::shared_ptr<Directory> Wfs::GetDirectory(const std::string& filename) {
  std::filesystem::path path(filename);
  std::shared_ptr<Directory> current_directory = root_area_->GetRootDirectory();

  for (auto& part : path) {
    // the first part is "/"
    if (part == "/")
      continue;
    current_directory = current_directory->GetDirectory(part.string());
    if (!current_directory)
      return current_directory;
  }
  return current_directory;
}

void Wfs::DetectDeviceSectorSizeAndCount(const std::shared_ptr<FileDevice>& device, const std::span<std::byte>& key) {
  // The encryption of the blocks depends on the device sector size and count, which builds the IV
  // We are going to find out the correct first 0x10 bytes, and than xor it with what we read to find out the correct IV
  // From that IV we extract the sectors count and sector size of the device.
  // Bytes 0-4 are going to be correct because the first 4 bytes of the IV is the block size, which we read from the
  // first block. The other bytes are part of the hash, so we will get them once we calculate the hash. So once we get
  // the correct hash we find the correct IV. So it will fail hash check Let's read the first block first, ignore the
  // hash and check the wfs version, and read the block size But lets set the sectors size to 9 and the sector size to
  // 0x10, because this is the max we are going to read right now
  device->SetSectorsCount(0x10);
  device->SetLog2SectorSize(9);
  auto enc_device = std::make_shared<DeviceEncryption>(device, key);
  auto block = MetadataBlock::LoadBlock(enc_device, 0, Block::BlockSize::Basic, 0, false);
  auto wfs_header = reinterpret_cast<WfsHeader*>(&block->Data()[sizeof(MetadataBlockHeader)]);
  if (wfs_header->version.value() != 0x01010800)
    throw std::runtime_error("Unexpected WFS version (bad key?)");
  auto block_size = Block::BlockSize::Basic;
  if (!(wfs_header->root_area_attributes.flags.value() & Attributes::Flags::AREA_SIZE_BASIC) &&
      (wfs_header->root_area_attributes.flags.value() & Attributes::Flags::AREA_SIZE_REGULAR))
    block_size = Block::BlockSize::Regular;
  // Now lets read it again, this time with the correct block size
  block = MetadataBlock::LoadBlock(enc_device, 0, block_size, 0, false);
  uint32_t xored_sectors_count, xored_sector_size;
  // The two last dwords of the IV is the sectors count and sector size, right now it is xored with our fake sector size
  // and sector count, and with the hash
  auto data = block->Data();
  auto first_4_dwords = reinterpret_cast<boost::endian::big_uint32_buf_t*>(data.data());
  xored_sectors_count = first_4_dwords[2].value();
  xored_sector_size = first_4_dwords[3].value();
  // Lets calculate the hash of the block
  enc_device->CalculateHash(data, {&data[offsetof(MetadataBlockHeader, hash)], enc_device->DIGEST_SIZE});
  // Now xor it with the real hash
  xored_sectors_count ^= first_4_dwords[2].value();
  xored_sector_size ^= first_4_dwords[3].value();
  // And xor it with our fake sectors count and block size
  xored_sectors_count ^= 0x10;
  xored_sector_size ^= 1 << 9;
  if (std::popcount(xored_sector_size) != 1) {
    // Not pow of 2
    throw std::runtime_error("Wfs: Failed to detect sector size and sectors count");
  }
  device->SetLog2SectorSize(std::bit_width(xored_sector_size) - 1);
  device->SetSectorsCount(xored_sectors_count);
  // Now try to fetch block again, this time check the hash, it will raise exception
  try {
    block = MetadataBlock::LoadBlock(enc_device, 0, block_size, 0, true);
  } catch (const Block::BadHash&) {
    throw std::runtime_error("Wfs: Failed to detect sector size and sectors count");
  }
}
