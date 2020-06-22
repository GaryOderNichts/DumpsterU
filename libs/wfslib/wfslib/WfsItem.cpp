/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "WfsItem.h"
#include "Structs.h"
#include "MetadataBlock.h"

#include <cctype>

WfsItem::WfsItem(const std::string& name, const AttributesBlock& attributes) : name(name), attributes(attributes) {}

Attributes * AttributesBlock::Attributes() const {
	if (!block) return NULL;
	return reinterpret_cast<::Attributes *>(&block->GetData()[attributes_offset]);
}

bool WfsItem::IsDirectory() {
	auto attributes = this->attributes.Attributes();
	return !attributes->IsLink() && attributes->IsDirectory();
}

bool WfsItem::IsFile() {
	auto attributes = this->attributes.Attributes();
	return !attributes->IsLink() && !attributes->IsDirectory();
}

bool WfsItem::IsLink() {
	auto attributes = this->attributes.Attributes();
	return attributes->IsLink();
}

std::string WfsItem::GetRealName() {
	auto attributes = this->attributes.Attributes();
	auto& filename = name;
	std::string real_filename = "";
	if (attributes->filename_length.value() != filename.size()) {
		throw std::runtime_error("Unexepected filename length");
	}
	uint8_t * bitmap_pos = reinterpret_cast<uint8_t*>(&attributes->case_bitmap);
	uint8_t cur = 0, i = 0;
	for (char c : name) {
		if (i++ % 8 == 0) {
			cur = *bitmap_pos++;
		}
		if (cur & 1) c = std::toupper(c);
		cur >>= 1;
		real_filename += c;
	}
	return real_filename;
}
