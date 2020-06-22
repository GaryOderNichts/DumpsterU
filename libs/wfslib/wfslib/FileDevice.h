/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Device.h"
#include <memory>
#include <iostream>
#include <mutex>

class FileDevice : public Device {
public:
	FileDevice(const std::string& path, uint32_t log2_sector_size = 9 /* 512 */, bool read_only = true);
	std::vector<uint8_t> ReadSectors(uint32_t sector_address, uint32_t sectors_count);
	void WriteSectors(const std::vector<uint8_t>& data, uint32_t sector_address, uint32_t sectors_count);
	uint32_t GetSectorsCount() {
		return sectors_count;
	}
	uint32_t GetLog2SectorSize() {
		return log2_sector_size;
	}

	void SetSectorsCount(uint32_t sectors_count) { this->sectors_count = sectors_count; }
	void SetLog2SectorSize(uint32_t log2_sector_size) { this->log2_sector_size = log2_sector_size; }
private:
	std::unique_ptr<std::iostream> file;
	std::mutex io_lock;

	uint32_t log2_sector_size;
	uint32_t sectors_count;
	bool read_only;
};