/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <bit>
#include <cassert>

inline size_t align_to_power_of_2(size_t size) {
  assert(size != 0);
  return static_cast<size_t>(1ULL << std::bit_width(size - 1));
}

inline size_t div_ceil(size_t n, size_t div) {
  return (n + div - 1) / div;
}
