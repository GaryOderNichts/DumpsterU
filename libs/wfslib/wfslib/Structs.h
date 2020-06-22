/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <boost/endian/buffers.hpp> 
#include <vector>
#include <string>

size_t round_pow2(size_t size);

// sizeof 0x18
struct MetadataBlockHeader {
	// Flags:
	// 0x00400000 - Area header
	// 0x00800000 - Root area
	// 0x20000000 - leaf directory tree
	// 0x40000000 - ?
	// 0x80000000 - Directory?
	enum Flags {
		AREA					= 0x00400000,
		ROOT_AREA				= 0x00800000,
		EXTERNAL_DIRECTORY_TREE	= 0x20000000,
		DIRECTORY				= 0x80000000,
	};
	boost::endian::big_uint32_buf_t block_flags; // 20 least bits ignored
	boost::endian::big_uint8_buf_t hash[20];
};
static_assert(sizeof(MetadataBlockHeader) == 0x18, "Incorrect sizeof MetadataBlockHeader");

// sizeof 0x18
struct DataBlockMetadata {
	boost::endian::big_uint32_buf_t block_number;
	boost::endian::big_uint8_buf_t hash[20];
};
static_assert(sizeof(DataBlockMetadata) == 0x18, "Incorrect sizeof DataBlockMetadata");

// sizeof 0xa4
struct DataBlocksClusterMetadata {
	boost::endian::big_uint32_buf_t block_number;
	boost::endian::big_uint8_buf_t hash[8][20];
};
static_assert(sizeof(DataBlocksClusterMetadata) == 0xa4, "Incorrect sizeof DataBlocksClusterMetadata");


// sizeof 0xc
struct Permissions {
	boost::endian::big_uint32_buf_t owner;
	boost::endian::big_uint32_buf_t group;
	boost::endian::big_uint32_buf_t mode;
};
static_assert(sizeof(Permissions) == 0xc, "Incorrect sizeof Permissions");

// sizeof 0x2c
struct Attributes {
	enum Flags {
		UNENCRYPTED_FILE = 0x2000000,
		LINK = 0x4000000,
		AREA_SIZE_BASIC = 0x10000000,
		AREA_SIZE_REGULAR = 0x20000000,
		QUOTA = 0x40000000,
		DIRECTORY = 0x80000000,
	};
	boost::endian::big_uint32_buf_t flags;
	boost::endian::big_uint32_buf_t size_on_disk;
	boost::endian::big_uint32_buf_t ctime;
	boost::endian::big_uint32_buf_t mtime;
	boost::endian::big_uint32_buf_t unknown; // TODO: ????
	union {
		boost::endian::big_uint32_buf_t size; // for file
		boost::endian::big_uint32_buf_t blocks_count; // for quota
	};
	boost::endian::big_uint32_buf_t directory_block_number; // in case of directory
	Permissions permissions;
	boost::endian::big_uint8_buf_t entry_log2_size; // log2 size of the whole entry, including this attributes
	boost::endian::big_uint8_buf_t size_category; // 0-4, see File.c
	boost::endian::big_uint8_buf_t filename_length;
	boost::endian::big_uint8_buf_t case_bitmap; // This byte in the struct also behave as padding, it isn't really a byte, it is a bitmap of filename_length

	bool IsDirectory() {
		return !!(flags.value() & DIRECTORY);
	}
	bool IsFile() {
		return !IsDirectory();
	}
	bool IsLink() {
		return !!(flags.value() & LINK);
	}

	size_t DataOffset();
};
static_assert(sizeof(Attributes) == 0x2C, "Incorrect sizeof Attributes");

// sizeof 0x48
struct WfsHeader {
	boost::endian::big_uint32_buf_t iv; // most 2 bits are device type. 1-mlc/3-usb
	boost::endian::big_uint32_buf_t version; // should be 0x01010800
	boost::endian::big_uint16_buf_t device_type; // usb - 0x16a2. mlc - 0x136a?
	boost::endian::big_uint16_buf_t _pad;
	Attributes root_area_attributes;
	boost::endian::big_uint32_buf_t root_area_directory_block_number; // should be 12 (6*2, when regular block size used)
	boost::endian::big_uint32_buf_t root_area_blocks_count;
	boost::endian::big_uint32_buf_t unknown[2]; // not used??
};
static_assert(sizeof(WfsHeader) == 0x48, "Incorrect sizeof WfsHeader");

// sizeof 0x44
struct WfsAreaHeaderCacheRelated {
	boost::endian::big_uint32_buf_t unknown_zero; // init to zero. unknown
	boost::endian::big_uint32_buf_t cache_blocks_count; // some cache blocks count, not sure
	// TODO...
	boost::endian::big_uint8_buf_t unknown[0x38];
	boost::endian::big_uint32_buf_t cache_log2_block_size; // some cache block size, not sure
};
static_assert(sizeof(WfsAreaHeaderCacheRelated) == 0x44, "Incorrect sizeof WfsAreaHeaderCacheRelated");

/// sizeof 0x60
struct WfsAreaHeader {
	boost::endian::big_uint32_buf_t iv; // used for blocks  encryption
	boost::endian::big_uint32_buf_t blocks_count;
	boost::endian::big_uint32_buf_t root_directory_block_number; // is 6
	boost::endian::big_uint32_buf_t unknown_allocator_block_number_4; // is 4
	boost::endian::big_uint32_buf_t unknown_allocator_block_number_5; // is 5
	boost::endian::big_uint8_buf_t depth; // how many total parents area this area has
	boost::endian::big_uint8_buf_t log2_block_size; // 12/13
	boost::endian::big_uint8_buf_t log2_mega_block_size; // mega block - 8 blocks. so 15/16
	boost::endian::big_uint8_buf_t log2_mega_blocks_cluster_size; // mega blocks cluster - 8 mega blocks. so 18/19
	boost::endian::big_uint8_buf_t data_type; // not sure. bool? (0 or 1)
	boost::endian::big_uint8_buf_t unknown_zero; // init to zero. does it used/changed by something?
	boost::endian::big_uint16_buf_t spare_blocks_count; // in case of quota, how many spare blocks this area allocated beyond the quota. (parent area blocks, so in parent area block size)

	WfsAreaHeaderCacheRelated cache_related;
};
static_assert(sizeof(WfsAreaHeader) == 0x60, "Incorrect sizeof WfsAreaHeader");

struct WfsArea {
	WfsAreaHeader header;

	// TODO: beyond this it is mostly unknown, and depends on header.data_type. So I don't even bother right now...
};
//static_assert(sizeof(WfsArea) == ,,, "Incorrect sizeof WfsArea");

struct SubBlockAllocatorFreeListEntry {
	boost::endian::big_uint16_buf_t unused;
	boost::endian::big_uint16_buf_t next;
	boost::endian::big_uint16_buf_t prev;
	boost::endian::big_uint16_buf_t log2_block_size;
};
static_assert(sizeof(SubBlockAllocatorFreeListEntry) == 0x8, "Incorrect sizeof SubBlockAllocatorFreeListEntry");

struct SubBlockAllocatorFreeList {
	boost::endian::big_uint16_buf_t free_blocks_count;
	boost::endian::big_uint16_buf_t head; // head free block
};
static_assert(sizeof(SubBlockAllocatorFreeList) == 0x4, "Incorrect sizeof SubBlockAllocatorFreeList");

struct SubBlockAllocatorStruct {
	SubBlockAllocatorFreeList free_list[8]; // for sizes 1<<3-10
	boost::endian::big_uint16_buf_t root;
	boost::endian::big_uint16_buf_t records_count;
};
static_assert(sizeof(SubBlockAllocatorStruct) == 0x24, "Incorrect sizeof SubBlockAllocatorStruct");

struct DirectoryTreeNode {
	boost::endian::big_uint8_buf_t value_length;
	boost::endian::big_uint8_buf_t choices_count;

public:
	std::string value() {
		return std::string(reinterpret_cast<char *>(this) + sizeof(*this), value_length.value());
	}
	std::vector<uint8_t> choices() {
		return std::vector<uint8_t>(reinterpret_cast<uint8_t *>(this) + sizeof(*this) + value_length.value(), reinterpret_cast<uint8_t *>(this) + sizeof(*this) + value_length.value() + choices_count.value());
	}
};
static_assert(sizeof(DirectoryTreeNode) == 0x2, "Incorrect sizeof DirectoryTreeNode");

struct ExternalDirectoryTreeNode : DirectoryTreeNode {
private:
	size_t size() {
		size_t total_size = sizeof(DirectoryTreeNode) + value_length.value() + choices_count.value() + choices_count.value() * sizeof(boost::endian::big_uint16_buf_t);
		size_t size = 1;
		while (size < total_size) size <<= 1;
		return size;
	}
public:
	boost::endian::big_uint16_buf_t get_item(size_t index) {
		return reinterpret_cast<boost::endian::big_uint16_buf_t *>(reinterpret_cast<uint8_t *>(this) + size())[-static_cast<int>(index) - 1];
	}
};

struct InternalDirectoryTreeNode : DirectoryTreeNode {
private:
	size_t size();
public:
	boost::endian::big_uint16_buf_t get_item(size_t index) {
		size_t adjust_offset = 0;
		if (choices()[0] == 0) adjust_offset = 2;
		return reinterpret_cast<boost::endian::big_uint16_buf_t *>(reinterpret_cast<uint8_t *>(this) + size() - adjust_offset)[-static_cast<int>(index) - 1];
	}
	boost::endian::big_uint32_buf_t get_next_allocator_block_number() {
		return reinterpret_cast<boost::endian::big_uint32_buf_t *>(reinterpret_cast<uint8_t *>(this) + size())[-1];
	}
};
