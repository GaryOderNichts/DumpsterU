/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "Wfs.h"
#include "DeviceEncryption.h"
#include "Area.h"
#include "Directory.h"
#include "FileDevice.h"
#include "MetadataBlock.h"
#include "Structs.h"

#include <boost/filesystem.hpp>


Wfs::Wfs(const std::shared_ptr<Device>& device, std::vector<uint8_t>& key) : device(std::make_shared<DeviceEncryption>(device, key)) {
	// Read first area
	this->root_area = Area::LoadRootArea(this->device);
}

std::shared_ptr<WfsItem> Wfs::GetObject(const std::string& filename) {
	if (filename == "/") return GetDirectory("/");
	boost::filesystem::path path(filename);
	auto dir = GetDirectory(path.parent_path().string());
	if (!dir) return std::shared_ptr<WfsItem>();
	return dir->GetObject(path.filename().string());
}

std::shared_ptr<File> Wfs::GetFile(const std::string& filename) {
	boost::filesystem::path path(filename);
	auto dir = GetDirectory(path.parent_path().string());
	if (!dir) return std::shared_ptr<File>();
	return dir->GetFile(path.filename().string());
}

std::shared_ptr<Directory> Wfs::GetDirectory(const std::string& filename) {
	boost::filesystem::path path(filename);
	std::shared_ptr<Directory> current_directory = this->root_area->GetRootDirectory();

	for (auto& part : path) {
		// the first part is "/"
		if (part == "/") continue;
		current_directory = current_directory->GetDirectory(part.string());
		if (!current_directory) return current_directory;
	}
	return current_directory;
}

void Wfs::DetectDeviceSectorSizeAndCount(const std::shared_ptr<FileDevice>& device, const std::vector<uint8_t>& key) {
	// The encryption of the blocks depends on the device sector size and count, which builds the IV
	// We are going to find out the correct first 0x10 bytes, and than xor it with what we read to find out the correct IV
	// From that IV we extract the sectors count and sector size of the device.
	// Bytes 0-4 are going to be correct because the first 4 bytes of the IV is the block size, which we read from the first block. 
	// The other bytes are part of the hash, so we will get them once we calculate the hash. So once we get the correct hash we find the correct IV.
	// So it will fail hash check
	// Let's read the first block first, ignore the hash and check the wfs version, and read the block size
	// But lets set the sectors size to 9 and the sector size to 0x10, because this is the max we are going to read right now
	device->SetSectorsCount(0x10);
	device->SetLog2SectorSize(9);
	auto enc_device = std::make_shared<DeviceEncryption>(device, key);
	auto block = MetadataBlock::LoadBlock(enc_device, 0, Block::BlockSize::Basic, 0, false);
	auto wfs_header = reinterpret_cast<WfsHeader *>(&block->GetData()[sizeof(MetadataBlockHeader)]);
	if (wfs_header->version.value() != 0x01010800)
		throw std::runtime_error("Unexpected WFS version (bad key?)");
	auto block_size = Block::BlockSize::Basic;
	if (!(wfs_header->root_area_attributes.flags.value() & wfs_header->root_area_attributes.Flags::AREA_SIZE_BASIC) && (wfs_header->root_area_attributes.flags.value() & wfs_header->root_area_attributes.Flags::AREA_SIZE_REGULAR))
		block_size = Block::BlockSize::Regular;
	// Now lets read it again, this time with the correct block size
	block = MetadataBlock::LoadBlock(enc_device, 0, block_size, 0, false);
	uint32_t xored_sectors_count, xored_sector_size;
	// The two last dwords of the IV is the sectors count and sector size, right now it is xored with our fake sector size and sector count, and with the hash
	auto& data = block->GetData();
	auto first_4_dwords = reinterpret_cast<boost::endian::big_uint32_buf_t*>(&data[0]);
	xored_sectors_count = first_4_dwords[2].value();
	xored_sector_size = first_4_dwords[3].value();
	// Lets calculate the hash of the block
	enc_device->CalculateHash(data, data.begin() + offsetof(MetadataBlockHeader, hash), true);
	// Now xor it with the real hash
	xored_sectors_count ^= first_4_dwords[2].value();
	xored_sector_size ^= first_4_dwords[3].value();
	// And xor it with our fake sectors count and block size
	xored_sectors_count ^= 0x10;
	xored_sector_size ^= 1 << 9;
	uint32_t sector_size = 0;
	while (!(xored_sector_size & 1)) {
		xored_sector_size >>= 1;
		sector_size++;
	}
	if (xored_sector_size >> 1) {
		// Not pow of 2
		throw std::runtime_error("Wfs: Failed to detect sector size and sectors count");
	}
	device->SetLog2SectorSize(sector_size);
	device->SetSectorsCount(xored_sectors_count);
	// Now try to fetch block again, this time check the hash, it will raise exception
	try {
		block = MetadataBlock::LoadBlock(enc_device, 0, block_size, 0, true);
	}
	catch (const Block::BadHash &) {
		throw std::runtime_error("Wfs: Failed to detect sector size and sectors count");
	}
}
