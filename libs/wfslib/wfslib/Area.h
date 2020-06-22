/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <memory>
#include <vector>

#include "Block.h"
#include "DataBlock.h"
#include "WfsItem.h"

class MetadataBlock;
class DeviceEncryption;
class Directory;

struct WfsArea;
struct WfsHeader;

class Area : public std::enable_shared_from_this<Area> {
public:
	Area(const std::shared_ptr<DeviceEncryption>& device, const std::shared_ptr<Area>& root_area, const std::shared_ptr<MetadataBlock>& block, const std::string& root_directory_name, const AttributesBlock& root_directory_attributes);

	static std::shared_ptr<Area> LoadRootArea(const std::shared_ptr<DeviceEncryption>& device);

	std::shared_ptr<Area> GetArea(uint32_t block_number, const std::string& root_directory_name, const AttributesBlock& root_directory_attributes, Block::BlockSize size);

	std::shared_ptr<Directory> GetRootDirectory();

	std::shared_ptr<Directory> GetDirectory(uint32_t block_number, const std::string& name, const AttributesBlock& attributes);

	std::shared_ptr<MetadataBlock> GetMetadataBlock(uint32_t block_number);
	std::shared_ptr<MetadataBlock> GetMetadataBlock(uint32_t block_number, Block::BlockSize size);

	std::shared_ptr<DataBlock> GetDataBlock(uint32_t block_number, Block::BlockSize size, uint32_t data_size, const DataBlock::DataBlockHash& data_hash, bool encrypted);

	uint32_t GetBlockNumber(const std::shared_ptr<Block>& block);

	size_t GetDataBlockLog2Size();

private:
	WfsArea * Data();
	WfsHeader * WfsData();

	uint32_t ToBasicBlockNumber(uint32_t block_number);
	uint32_t IV(uint32_t block_number);

	std::shared_ptr<DeviceEncryption> device;
	std::shared_ptr<Area> root_area; // Empty pointer for root area

	std::shared_ptr<MetadataBlock> header_block;

	std::string root_directory_name;
	AttributesBlock root_directory_attributes;
};