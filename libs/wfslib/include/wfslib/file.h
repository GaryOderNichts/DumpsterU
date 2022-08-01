/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/stream.hpp>
#include <memory>
#include <vector>
#include "wfs_item.h"

class Area;

class File : public WfsItem, public std::enable_shared_from_this<File> {
 public:
  class DataCategoryReader;
  class DataCategory0Reader;
  class RegularDataCategoryReader;
  class DataCategory1Reader;
  class DataCategory2Reader;
  class DataCategory3Reader;
  class DataCategory4Reader;

  File(const std::string& name, const AttributesBlock& attributes, const std::shared_ptr<Area>& area)
      : WfsItem(name, attributes), area_(area) {}

  uint32_t Size();
  uint32_t SizeOnDisk();
  void Resize(size_t new_size);

  class file_device {
   public:
    typedef char char_type;
    struct category : public boost::iostreams::seekable_device_tag, public boost::iostreams::optimally_buffered_tag {};
    file_device(const std::shared_ptr<File>& file);

    std::streamsize read(char_type* s, std::streamsize n);
    std::streamsize write(const char_type* s, std::streamsize n);
    boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);
    std::streamsize optimal_buffer_size() const;

   private:
    size_t size();
    std::shared_ptr<File> file_;
    std::shared_ptr<DataCategoryReader> reader_;
    boost::iostreams::stream_offset pos_;
  };

  typedef boost::iostreams::stream<file_device> stream;

 private:
  const std::shared_ptr<Area>& area() const { return area_; }

  // TODO: We may have cyclic reference here if we do cache in area.
  std::shared_ptr<Area> area_;

  static std::shared_ptr<DataCategoryReader> CreateReader(const std::shared_ptr<File>& file);
};
