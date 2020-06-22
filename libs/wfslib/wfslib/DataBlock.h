/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Block.h"
#include <memory>

class MetadataBlock;

class DataBlock : public Block {
public:
	struct DataBlockHash {
		std::shared_ptr<MetadataBlock> block;
		size_t hash_offset;
	};

	DataBlock(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t data_size, uint32_t iv, const DataBlockHash& data_hash, bool encrypted);

	virtual void Fetch(bool check_hash = true);
	virtual void Flush();

	static std::shared_ptr<DataBlock> LoadBlock(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t data_size, uint32_t iv, const DataBlockHash& data_hash, bool encrypted);

private:
	DataBlockHash data_hash;
};