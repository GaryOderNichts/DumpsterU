/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <cstdint>
#include <span>
#include <vector>

class Device {
 public:
  virtual ~Device() {}
  virtual void ReadSectors(const std::span<std::byte>& data, uint32_t sector_address, uint32_t sectors_count) = 0;
  virtual void WriteSectors(const std::span<std::byte>& data, uint32_t sector_address, uint32_t sectors_count) = 0;
  virtual uint32_t SectorsCount() = 0;
  virtual uint32_t Log2SectorSize() = 0;
  uint32_t SectorSize() { return 1 << Log2SectorSize(); }
};
