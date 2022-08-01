/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <memory>
#include <string>

struct Attributes;
struct AttributesBlock;
class MetadataBlock;

struct AttributesBlock {
  std::shared_ptr<MetadataBlock> block;
  size_t attributes_offset;
  ::Attributes* Attributes() const;
};

class WfsItem {
 public:
  WfsItem(const std::string& name, const AttributesBlock& block);
  virtual ~WfsItem() {}
  const std::string& GetName() { return name_; }
  std::string GetRealName();
  virtual bool IsDirectory();
  virtual bool IsFile();
  virtual bool IsLink();

 protected:
  const AttributesBlock& attributes_data() const { return attributes_; }

  std::string name_;
  AttributesBlock attributes_;
};
