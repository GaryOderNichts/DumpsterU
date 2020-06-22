/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "DataBlock.h"
#include "MetadataBlock.h"
#include "DeviceEncryption.h"

#include "Structs.h"

DataBlock::DataBlock(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t data_size, uint32_t iv, const DataBlockHash& data_hash, bool encrypted) :
	Block(device, block_number, size_category, iv, encrypted, std::vector<uint8_t>(data_size, 0)), data_hash(data_hash) {
}

void DataBlock::Fetch(bool check_hash) {
	this->Block::Fetch();
	if (check_hash && !this->device->CheckHash(data, data_hash.block->GetData().begin() + data_hash.hash_offset, false))
		throw Block::BadHash(block_number);
}

void DataBlock::Flush() {
	this->device->CalculateHash(data, data_hash.block->GetData().begin() + data_hash.hash_offset, false);
	this->Block::Flush();
	// TODO: Write now we write two blocks for each block written, we need some caching with option to commit changes
	data_hash.block->Flush();
}

std::shared_ptr<DataBlock> DataBlock::LoadBlock(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t data_size, uint32_t iv, const DataBlockHash& data_hash, bool encrypted) {
	auto block = std::make_shared<DataBlock>(device, block_number, size_category, data_size, iv, data_hash, encrypted);
	if (data_size) {
		// Fetch block only if have data
		block->Fetch();
	}
	return block;
}
