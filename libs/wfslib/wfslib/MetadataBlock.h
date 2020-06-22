/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Block.h"
#include <memory>

struct MetadataBlockHeader;

class MetadataBlock : public Block {
public:
	MetadataBlock(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t iv);

	virtual void Fetch(bool check_hash = true);
	virtual void Flush();

	static std::shared_ptr<MetadataBlock> LoadBlock(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t iv, bool check_hash = true);

	MetadataBlockHeader * Header();
};