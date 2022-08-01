/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <memory>
#include <string>
#include "directory_items_iterator.h"
#include "wfs_item.h"

class Area;
class MetadataBlock;
class File;

struct DirectoryTreeNode;

class Directory : public WfsItem, public std::enable_shared_from_this<Directory> {
 public:
  Directory(const std::string& name,
            AttributesBlock attributes,
            const std::shared_ptr<Area>& area,
            const std::shared_ptr<MetadataBlock>& block)
      : WfsItem(name, attributes), area_(area), block_(block) {}

  std::shared_ptr<WfsItem> GetObject(const std::string& name);
  std::shared_ptr<Directory> GetDirectory(const std::string& name);
  std::shared_ptr<File> GetFile(const std::string& name);

  size_t Size();
  DirectoryItemsIterator begin();
  DirectoryItemsIterator end();

 private:
  friend DirectoryItemsIterator;
  const std::shared_ptr<Area>& area() const { return area_; }

  // TODO: We may have cyclic reference here if we do cache in area.
  std::shared_ptr<Area> area_;

  std::shared_ptr<MetadataBlock> block_;

  std::shared_ptr<WfsItem> Create(const std::string& name, const AttributesBlock& attributes);
  AttributesBlock GetObjectAttributes(const std::shared_ptr<MetadataBlock>& block, const std::string& name);
};
