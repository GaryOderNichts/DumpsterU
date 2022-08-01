/*
 * Copyright (C) 2022 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "directory_items_iterator.h"

#include <stdexcept>
#include "area.h"
#include "directory.h"
#include "metadata_block.h"
#include "structs.h"
#include "sub_block_allocator.h"

DirectoryItemsIterator::DirectoryItemsIterator(const std::shared_ptr<Directory>& directory,
                                               const std::shared_ptr<NodeState>& node_state)
    : directory_(directory), node_state_(std::move(node_state)) {}

DirectoryItemsIterator::DirectoryItemsIterator(const DirectoryItemsIterator& mit)
    : directory_(mit.directory_), node_state_(mit.node_state_) {}

DirectoryItemsIterator& DirectoryItemsIterator::operator++() {
  // Go back until we are not at the end of a node
  while (++(node_state_->current_index) == node_state_->node->choices_count.value()) {
    node_state_ = std::move(node_state_->parent);
    if (!node_state_)
      return *this;
  }
  // Enter nodes until we hit a node that has value
  while (node_state_->node->choices()[node_state_->current_index] != std::byte{0}) {
    auto block = node_state_->block;
    uint16_t node_offset = 0;
    if (node_state_->block->Header()->block_flags.value() &
        node_state_->block->Header()->Flags::EXTERNAL_DIRECTORY_TREE) {
      node_offset =
          static_cast<ExternalDirectoryTreeNode*>(node_state_->node)->get_item(node_state_->current_index).value();
    } else {
      node_offset =
          static_cast<InternalDirectoryTreeNode*>(node_state_->node)->get_item(node_state_->current_index).value();
    }
    auto current_node = SubBlockAllocator(block).GetNode<DirectoryTreeNode>(node_offset);
    std::string path = node_state_->path +
                       std::string(1, std::to_integer<char>(node_state_->node->choices()[node_state_->current_index])) +
                       current_node->prefix();
    node_state_ = std::make_shared<NodeState>(NodeState{block, current_node, std::move(node_state_), 0, path});
  }
  if (!(node_state_->block->Header()->block_flags.value() &
        node_state_->block->Header()->Flags::EXTERNAL_DIRECTORY_TREE)) {
    // This is just internal node (in the directories trees tree), it just point to another tree
    auto block = directory_->area()->GetMetadataBlock(
        static_cast<InternalDirectoryTreeNode*>(node_state_->node)->get_next_allocator_block_number().value());
    auto current_node = SubBlockAllocator(block).GetRootNode<DirectoryTreeNode>();
    node_state_ =
        std::make_shared<NodeState>(NodeState{block, current_node, std::move(node_state_), 0, current_node->prefix()});
    // -- because it will be advanced immedialty to 0 when we do ++
    --node_state_->current_index;
    // Go to the first node in this directory block
    return operator++();
  }
  return *this;
}

DirectoryItemsIterator DirectoryItemsIterator::operator++(int) {
  DirectoryItemsIterator tmp(*this);
  operator++();
  return tmp;
}

bool DirectoryItemsIterator::operator==(const DirectoryItemsIterator& rhs) const {
  if (!rhs.node_state_ && !node_state_)
    return true;
  if (!rhs.node_state_ != !node_state_)
    return false;
  return rhs.node_state_->block == node_state_->block && rhs.node_state_->node == node_state_->node &&
         rhs.node_state_->current_index == node_state_->current_index;
}

bool DirectoryItemsIterator::operator!=(const DirectoryItemsIterator& rhs) const {
  return !operator==(rhs);
}

std::shared_ptr<WfsItem> DirectoryItemsIterator::operator*() {
  if (!node_state_)
    return nullptr;
  if (node_state_->block->Header()->block_flags.value() &
      node_state_->block->Header()->Flags::EXTERNAL_DIRECTORY_TREE) {
    auto block = node_state_->block;
    auto external_node = static_cast<ExternalDirectoryTreeNode*>(node_state_->node);
    return directory_->Create(node_state_->path,
                              AttributesBlock{block, external_node->get_item(node_state_->current_index).value()});
  } else {
    // Should not happen (can't happen, the iterator should stop only at external trees)
    throw std::logic_error("Should not happen!");
  }
}
