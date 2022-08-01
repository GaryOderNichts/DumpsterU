/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "structs.h"
#include "utils.h"

size_t Attributes::DataOffset() {
  return offsetof(Attributes, case_bitmap) + div_ceil(filename_length.value(), 8);
}

size_t ExternalDirectoryTreeNode::size() {
  size_t total_size = sizeof(DirectoryTreeNode) + prefix_length.value() + choices_count.value() +
                      choices_count.value() * sizeof(boost::endian::big_uint16_buf_t);
  return align_to_power_of_2(total_size);
}

size_t InternalDirectoryTreeNode::size() {
  size_t total_size = sizeof(DirectoryTreeNode) + prefix_length.value() + choices_count.value() +
                      choices_count.value() * sizeof(boost::endian::big_uint16_buf_t);
  if (choices_count.value() > 0 && choices()[0] == std::byte{0})
    total_size += 2;
  return align_to_power_of_2(total_size);
}
