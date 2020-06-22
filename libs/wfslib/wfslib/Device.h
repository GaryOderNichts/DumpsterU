/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <cstdint>
#include <vector>

class Device {
public:
	virtual ~Device() {}
	virtual std::vector<uint8_t> ReadSectors(uint32_t sector_address, uint32_t sectors_count) = 0;
	virtual void WriteSectors(const std::vector<uint8_t>& data, uint32_t sector_address, uint32_t sectors_count) = 0;
	virtual uint32_t GetSectorsCount() = 0;
	virtual uint32_t GetLog2SectorSize() = 0;
	uint32_t GetSectorSize() { return 1 << GetLog2SectorSize(); }
};
