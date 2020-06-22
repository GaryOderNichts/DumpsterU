/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <memory>
#include "MetadataBlock.h"
#include "Structs.h"

struct MetadataBlockHeader;
struct SubBlockAllocatorStruct;

class SubBlockAllocator {
public:
	SubBlockAllocator(const std::shared_ptr<MetadataBlock>& block) : block(block) {}

	template<typename T>
	T * GetNode(uint16_t offset) {
		return reinterpret_cast<T *>(&block->GetData()[offset]);
	}

	template<typename T>
	T * GetRootNode() {
		return GetNode<T>(Header()->root.value());
	}

private:
	std::shared_ptr<MetadataBlock> block;

	// TODO uint16_t Alloc(uint16_t size);
	// TODO void Free(uint16_t offset);

	SubBlockAllocatorStruct * Header();
};
