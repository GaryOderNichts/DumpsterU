/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <memory>
#include <string>

#include "WfsItem.h"

class Area;
class MetadataBlock;
class File;

struct DirectoryTreeNode;

class Directory : public WfsItem, public std::enable_shared_from_this<Directory> {
public:
	Directory(const std::string& name, AttributesBlock attributes, const std::shared_ptr<Area>& area, const std::shared_ptr<MetadataBlock>& block) : WfsItem(name, attributes), area(area), block(block) {
	}

	std::shared_ptr<WfsItem> GetObject(const std::string& name);
	std::shared_ptr<Directory> GetDirectory(const std::string& name);
	std::shared_ptr<File> GetFile(const std::string& name);

	size_t GetItemsCount();

private:
	// TODO: We may have cyclic reference here if we do cache in area.
	std::shared_ptr<Area> area;

	std::shared_ptr<MetadataBlock> block;

	std::shared_ptr<WfsItem> Create(const std::string& name, const AttributesBlock& attributes);
	AttributesBlock GetObjectAttributes(const std::shared_ptr<MetadataBlock>& block, const std::string& name);

	struct NodeState {
		std::shared_ptr<MetadataBlock> block;
		DirectoryTreeNode * node;
		std::shared_ptr<NodeState> parent;
		size_t current_index;
		std::string path;
	};

public:
	class FilesIterator : public std::iterator<std::input_iterator_tag, std::shared_ptr<WfsItem>> {
	private:
		std::shared_ptr<Directory> directory;
		std::shared_ptr<NodeState> node_state;
	public:
		FilesIterator(const std::shared_ptr<Directory>& directory, const std::shared_ptr<NodeState>& node_state) : directory(directory), node_state(std::move(node_state)) {}
		FilesIterator(const FilesIterator& mit) : directory(mit.directory), node_state(mit.node_state) {}
		FilesIterator& operator++();
		FilesIterator operator++(int) { FilesIterator tmp(*this); operator++(); return tmp; }
		bool operator==(const FilesIterator& rhs) const {
			if (!rhs.node_state && !node_state) return true;
			if (!rhs.node_state != !node_state) return false;
			return rhs.node_state->block == node_state->block &&
				rhs.node_state->node == node_state->node &&
				rhs.node_state->current_index == node_state->current_index;
		}
		bool operator!=(const FilesIterator& rhs) const { return !operator==(rhs); }
		std::shared_ptr<WfsItem> operator*();
	};

	FilesIterator begin();
	FilesIterator end();
};