/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "File.h"
#include "Structs.h"
#include "MetadataBlock.h"
#include "DataBlock.h"
#include "Area.h"

#include <cassert>

struct FileDataChunkInfo {
	std::shared_ptr<Block> data_block;
	size_t offset_in_block;
	size_t size;
};

uint32_t File::GetSize() {
	return attributes.Attributes()->size.value();
}

uint32_t File::GetSizeOnDisk() {
	return attributes.Attributes()->size_on_disk.value();
}

static size_t GetOffsetInMetadataBlock(const std::shared_ptr<MetadataBlock>& block, uint8_t* data) {
	return data - &block->GetData()[0];
}

class File::DataCategoryReader {
public:
	DataCategoryReader(const std::shared_ptr<File>& file) : file(file) {}
	virtual ~DataCategoryReader() {}
	virtual size_t GetAttributesMetadataSize() = 0;
	virtual FileDataChunkInfo GetFileDataChunkInfo(size_t offset, size_t size) = 0;
	size_t Read(uint8_t* data, size_t offset, size_t size) {
		auto chunk_info = GetFileDataChunkInfo(offset, size);
		auto data_begin = chunk_info.data_block->GetData().begin();
		std::copy(data_begin + chunk_info.offset_in_block, data_begin + chunk_info.offset_in_block + chunk_info.size, data);
		return chunk_info.size;
	}
	size_t Write(const uint8_t* data, size_t offset, size_t size) {
		auto chunk_info = GetFileDataChunkInfo(offset, size);
		auto data_begin = chunk_info.data_block->GetData().begin();
		std::copy(data, data + chunk_info.size, data_begin + chunk_info.offset_in_block);
		chunk_info.data_block->Flush();
		return chunk_info.size;
	}

	virtual void Resize(size_t new_size) = 0;

protected:
	std::shared_ptr<File> file;

	size_t GetAttributesMetadataOffset() {
		return file->attributes.attributes_offset + file->attributes.Attributes()->DataOffset();
	}
	size_t GetAttributesMetadataEndOffset() {
		return file->attributes.attributes_offset + round_pow2(file->attributes.Attributes()->DataOffset() + GetAttributesMetadataSize());
	}
	uint8_t* GetAttributesMetadataEnd() {
		// We can't do [GetAttributesMetadataEndOffset()] because it might point to data.end(), so in debug it will cause an error
		return &file->attributes.block->GetData()[0] + GetAttributesMetadataEndOffset();
	}
};

// Category 0 - File data is in the attribute metadata (limited to 512 bytes minus attribute size) (no minumum)
class File::DataCategory0Reader : public File::DataCategoryReader {
public:
	DataCategory0Reader(const std::shared_ptr<File>& file) : DataCategoryReader(file) {}

	virtual size_t GetAttributesMetadataSize() {
		return file->attributes.Attributes()->size_on_disk.value();
	}

	virtual FileDataChunkInfo GetFileDataChunkInfo(size_t offset, size_t size) {
		return FileDataChunkInfo { file->attributes.block, GetAttributesMetadataOffset() + offset, size };
	}

	virtual void Resize(size_t new_size) {
		// Just update the attribute, the data in the metadata block
		file->attributes.Attributes()->size = static_cast<uint32_t>(new_size);
		file->attributes.block->Flush();
	}
};

class File::RegularDataCategoryReader : public File::DataCategoryReader {
public:
	RegularDataCategoryReader(const std::shared_ptr<File>& file) : DataCategoryReader(file) {}

	virtual size_t GetAttributesMetadataSize() {
		// round up dividation
		size_t data_blocks_count = ((file->attributes.Attributes()->size_on_disk.value() - 1) >> GetDataBlockSize()) + 1;
		return sizeof(DataBlockMetadata) * data_blocks_count;
	}

	virtual FileDataChunkInfo GetFileDataChunkInfo(size_t offset, size_t size) {
		auto blocks_list = reinterpret_cast<DataBlockMetadata *>(GetAttributesMetadataEnd());
		int64_t block_index = offset >> GetDataBlockSize();
		size_t offset_in_block = offset & ((1 << GetDataBlockSize()) - 1);
		auto hash_block = file->attributes.block;
		uint32_t block_offset = static_cast<uint32_t>((offset >> GetDataBlockSize()) << GetDataBlockSize());

		LoadDataBlock(blocks_list[-block_index - 1].block_number.value(),
			static_cast<uint32_t>(std::min(1U << GetDataBlockSize(), file->attributes.Attributes()->size.value() - block_offset)),
			DataBlock::DataBlockHash{ hash_block, GetOffsetInMetadataBlock(hash_block, reinterpret_cast<uint8_t*>(&blocks_list[-block_index - 1].hash)) });
		size = std::min(size, current_data_block->GetData().size() - offset_in_block);
		return FileDataChunkInfo{ current_data_block, offset_in_block, size };
	}

	virtual void Resize(size_t new_size) {
		size_t old_size = file->attributes.Attributes()->size.value();
		while (old_size != new_size) {
			std::shared_ptr<Block> current_block;
			size_t new_block_size = 0;
			if (new_size < old_size) {
				// Just update last block
				if (new_size > 0) {
					// Minus 1 because if it is right at the end of the block, we will get the next block
					auto chunk_info = GetFileDataChunkInfo(new_size - 1, 1);
					current_block = chunk_info.data_block;
					new_block_size = std::min(chunk_info.offset_in_block + 1, static_cast<size_t>(1U) << GetDataBlockSize());
				}
				old_size = new_size;
			}
			else {
				if (old_size & ((1 << GetDataBlockSize()) - 1)) {
					// We need to incrase the size of the last block
					// Minus 1 because if it is right at the end of the block, we will get the next block
					auto chunk_info = GetFileDataChunkInfo(old_size - 1, 1);
					current_block = chunk_info.data_block;
					new_block_size = std::min(chunk_info.offset_in_block + 1 + (new_size - old_size), static_cast<size_t>(1U) << GetDataBlockSize());
					old_size += new_block_size - (chunk_info.offset_in_block + 1);
				}
				else {
					// Open new block, the size of the loaded block will be 0
					auto chunk_info = GetFileDataChunkInfo(old_size, 0);
					current_block = chunk_info.data_block;
					assert(chunk_info.offset_in_block == 0);
					new_block_size = std::min(new_size - old_size, static_cast<size_t>(1U) << GetDataBlockSize());
					old_size += new_block_size;
				}
			}
			file->attributes.Attributes()->size = static_cast<uint32_t>(old_size);
			if (current_block) {
				assert(std::dynamic_pointer_cast<DataBlock>(current_block));
				auto& data = current_block->GetData();
				if (new_block_size != data.size()) {
					data.resize(new_block_size);
					current_block->Flush();
				}
			}
		}
		file->attributes.block->Flush();
	}

protected:
	virtual size_t GetBlocksLog2CountInDataBlock() = 0;
	virtual Block::BlockSize GetDataBlockSize() {
		return static_cast<Block::BlockSize>(file->area->GetDataBlockLog2Size() + GetBlocksLog2CountInDataBlock());
	}
	std::shared_ptr<DataBlock> current_data_block;

	void LoadDataBlock(uint32_t block_number, uint32_t data_size, const DataBlock::DataBlockHash& data_hash) {
		if (current_data_block && file->area->GetBlockNumber(current_data_block) == block_number) return;
		current_data_block = file->area->GetDataBlock(block_number, GetDataBlockSize(), data_size, data_hash, !(file->attributes.Attributes()->flags.value() & Attributes::UNENCRYPTED_FILE));
	}
};

// Category 1 - File data in regluar blocks, in the attribute metadata there is a reversed list of block numbers and hashes. Limited to 5 blocks. (no minumum)
class File::DataCategory1Reader : public File::RegularDataCategoryReader {
public:
	DataCategory1Reader(const std::shared_ptr<File>& file) : RegularDataCategoryReader(file) {}
protected:
	virtual size_t GetBlocksLog2CountInDataBlock() {
		return 0;
	}
};

// Category 2 - File data in mega block (8 regular blocks), in the attribute metadata there is a reversed list of block numbers and hashes. Limited to 5 mega blocks. (minimum size of more than 1 regular block)
class File::DataCategory2Reader : public File::RegularDataCategoryReader {
public:
	DataCategory2Reader(const std::shared_ptr<File>& file) : RegularDataCategoryReader(file) {}
protected:
	virtual size_t GetBlocksLog2CountInDataBlock() {
		return 3;
	}
};

// Category 3 - File data in clusters of mega block (8 mega blocksblocks), in the attribute metadata there is a reversed list of block number and 8 hashes for each cluster. Limited to 4 clusters. (minimum size of more than 1 mega block)
class File::DataCategory3Reader : public File::DataCategory2Reader {
public:
	DataCategory3Reader(const std::shared_ptr<File>& file) : DataCategory2Reader(file) {}

	virtual size_t GetAttributesMetadataSize() {
		size_t data_blocks_clusters_count = ((file->attributes.Attributes()->size_on_disk.value() - 1) >> ClusterDataLog2Size()) + 1;
		return sizeof(DataBlocksClusterMetadata) * data_blocks_clusters_count;
	}

	virtual FileDataChunkInfo GetFileDataChunkInfo(size_t offset, size_t size) {
		return GetFileDataChunkInfoFromClustersList(offset, offset, size, file->attributes.block, reinterpret_cast<DataBlocksClusterMetadata *>(GetAttributesMetadataEnd()), true);
	}
protected:
	FileDataChunkInfo GetFileDataChunkInfoFromClustersList(size_t offset, size_t original_offset, size_t size, const std::shared_ptr<MetadataBlock>& metadata_block, DataBlocksClusterMetadata * clusters_list, bool reverse) {
		int64_t cluster_index = offset >> ClusterDataLog2Size();
		size_t offset_in_cluster = offset & ((1 << ClusterDataLog2Size()) - 1);
		size_t block_index = offset_in_cluster >> GetDataBlockSize();
		size_t offset_in_block = offset_in_cluster & ((1 << GetDataBlockSize()) - 1);
		uint32_t block_offset = static_cast<uint32_t>((original_offset >> GetDataBlockSize()) << GetDataBlockSize());

		DataBlocksClusterMetadata * cluster = NULL;
		if (reverse)
			cluster = &clusters_list[-cluster_index - 1];
		else
			cluster = &clusters_list[cluster_index];
		LoadDataBlock(cluster->block_number.value() + static_cast<uint32_t>(block_index << GetBlocksLog2CountInDataBlock()),
			static_cast<uint32_t>(std::min(1U << GetDataBlockSize(), file->attributes.Attributes()->size.value() - block_offset)),
			DataBlock::DataBlockHash{ metadata_block, GetOffsetInMetadataBlock(metadata_block, reinterpret_cast<uint8_t*>(&cluster->hash[block_index])) });
		size = std::min(size, current_data_block->GetData().size() - offset_in_block);
		return FileDataChunkInfo{ current_data_block, offset_in_block, size };
	}

	size_t ClusterDataLog2Size() {
		return GetDataBlockSize() + 3;
	}
};

// Category 4 - File data in clusters of mega block (8 mega blocksblocks), in the attribute metadata there is list of block numbers of metadata block with lists of block number and 8 hashes for each cluster. Limited to 237 metadata blocks of lists. (max file size) (minumum size of more/equal than 1 cluster)
class File::DataCategory4Reader : public File::DataCategory3Reader {
public:
	DataCategory4Reader(const std::shared_ptr<File>& file) : DataCategory3Reader(file) {}

	virtual size_t GetAttributesMetadataSize() {
		size_t data_blocks_clusters_count = ((file->attributes.Attributes()->size_on_disk.value() - 1) >> ClusterDataLog2Size()) + 1;
		size_t blocks_count = ((data_blocks_clusters_count - 1) / ClustersInBlock()) + 1;
		return sizeof(boost::endian::big_uint32_buf_t) * blocks_count;
	}

	virtual FileDataChunkInfo GetFileDataChunkInfo(size_t offset, size_t size) {
		auto blocks_list = reinterpret_cast<boost::endian::big_uint32_buf_t *>(GetAttributesMetadataEnd());
		int64_t block_index = offset / (ClustersInBlock() << ClusterDataLog2Size());
		size_t offset_in_block = offset % (ClustersInBlock() << ClusterDataLog2Size());
		LoadMetadataBlock(blocks_list[-block_index - 1].value());
		return GetFileDataChunkInfoFromClustersList(offset_in_block, offset, size, current_metadata_block,
			reinterpret_cast<DataBlocksClusterMetadata *>(&*(current_metadata_block->GetData().begin() + sizeof(MetadataBlockHeader))), false);
	}

protected:
	std::shared_ptr<MetadataBlock> current_metadata_block;

	void LoadMetadataBlock(uint32_t block_number) {
		if (current_metadata_block && file->area->GetBlockNumber(current_metadata_block) == block_number) return;
		current_metadata_block = file->area->GetMetadataBlock(block_number);
	}

	size_t ClustersInBlock() {
		size_t clusters_in_block = (file->area->GetDataBlockLog2Size() - sizeof(MetadataBlockHeader)) / sizeof(DataBlocksClusterMetadata);
		clusters_in_block = std::min(clusters_in_block, static_cast<size_t>(48));
		return clusters_in_block;
	}
};

std::shared_ptr<File::DataCategoryReader> File::CreateReader(const std::shared_ptr<File>& file) {
	switch (file->attributes.Attributes()->size_category.value()) {
	case 0: return std::make_shared<DataCategory0Reader>(file);
	case 1: return std::make_shared<DataCategory1Reader>(file);
	case 2: return std::make_shared<DataCategory2Reader>(file);
	case 3: return std::make_shared<DataCategory3Reader>(file);
	case 4: return std::make_shared<DataCategory4Reader>(file);
	default: throw std::runtime_error("Unexpected file category");
	}
}

void File::Resize(size_t new_size) {
	// TODO: implment it, write now change up to size_on_disk without ever chaning size_on_disk
	auto attributes = this->attributes.Attributes();
	new_size = std::min(new_size, static_cast<size_t>(attributes->size_on_disk.value()));
	size_t old_size = attributes->size.value();
	if (new_size != old_size) {
		CreateReader(shared_from_this())->Resize(new_size);
	}
}

File::file_device::file_device(const std::shared_ptr<File>& file) : file(file), reader(CreateReader(file)), pos(0) {
}

size_t File::file_device::size() {
	return file->attributes.Attributes()->size.value();
}

std::streamsize File::file_device::read(char_type* s, std::streamsize n)
{
	std::streamsize amt = static_cast<std::streamsize>(size() - pos);
	std::streamsize result = std::min(n, amt);

	if (result <= 0) return -1; // EOF

	std::streamsize to_read = result;
	while (to_read > 0) {
		size_t read = reader->Read(reinterpret_cast<uint8_t*>(s), static_cast<size_t>(pos), static_cast<size_t>(to_read));
		s += read;
		pos += read;
		to_read -= read;
	}
	return result;
}
std::streamsize File::file_device::write(const char_type* s, std::streamsize n)
{
	std::streamsize amt = static_cast<std::streamsize>(size() - pos);
	if (n > amt) {
		// Try to resize file
		// TODO: This call can't stay like that when we will need to allocate new pages and even change the category
		reader->Resize(std::min(static_cast<size_t>(file->GetSizeOnDisk()), static_cast<size_t>(pos + n)));
		amt = static_cast<std::streamsize>(size() - pos);
	}
	std::streamsize result = std::min(n, amt);

	if (result <= 0) return -1; // Failed to resize file

	std::streamsize to_write = result;
	while (to_write > 0) {
		size_t wrote = reader->Write(reinterpret_cast<const uint8_t*>(s), static_cast<size_t>(pos), static_cast<size_t>(to_write));
		s += wrote;
		pos += wrote;
		to_write -= wrote;
	}
	return result;
}
boost::iostreams::stream_offset File::file_device::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
{
	// Determine new value of pos_
	boost::iostreams::stream_offset next;
	if (way == std::ios_base::beg) {
		next = off;
	}
	else if (way == std::ios_base::cur) {
		next = pos + off;
	}
	else if (way == std::ios_base::end) {
		next = size() + off - 1;
	}
	else {
		throw std::ios_base::failure("bad seek direction");
	}

	// Check for errors
	if (next < 0 || next >= static_cast<boost::iostreams::stream_offset>(size()))
		throw std::ios_base::failure("bad seek offset");

	pos = next;
	return pos;
}
