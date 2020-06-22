/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "MetadataBlock.h"
#include "DeviceEncryption.h"

#include "Structs.h"

MetadataBlock::MetadataBlock(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t iv) :
	Block(device, block_number, size_category, iv, true, std::vector<uint8_t>(1LL << size_category, 0)) {
}

void MetadataBlock::Fetch(bool check_hash) {
	this->Block::Fetch();
	if (check_hash && !this->device->CheckHash(data, data.begin() + offsetof(MetadataBlockHeader, hash), true))
		throw Block::BadHash(block_number);
}

void MetadataBlock::Flush() {
	this->device->CalculateHash(data, data.begin() + offsetof(MetadataBlockHeader, hash), true);
	this->Block::Flush();
}

std::shared_ptr<MetadataBlock> MetadataBlock::LoadBlock(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t iv, bool check_hash) {
	auto block = std::make_shared<MetadataBlock>(device, block_number, size_category, iv);
	block->Fetch(check_hash);
	return block;
}

MetadataBlockHeader * MetadataBlock::Header() {
	return reinterpret_cast<MetadataBlockHeader *>(&data[0]);
}