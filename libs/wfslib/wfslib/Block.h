/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <memory>
#include <vector>
#include <exception>
#include <string>

#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

class DeviceEncryption;

class Block {
public:
	enum BlockSize {
		Basic = 12,
		Regular = 13,
		MegaBasic = 15,
		MegaRegular = 16,
	};

	virtual void Fetch(bool check_hash = true);
	virtual void Flush();

	std::vector<uint8_t>& GetData() { return data; }
	uint32_t GetBlockNumber() { return block_number;  }

	class BadHash : public std::exception {
	public:
		BadHash(uint32_t block_number);
		virtual char const* what() const NOEXCEPT;
	private:
		uint32_t block_number;
		std::string msg;
	};
private:
	uint32_t ToDeviceSector(uint32_t block_number);

protected:
	Block(const std::shared_ptr<DeviceEncryption>& device, uint32_t block_number, Block::BlockSize size_category, uint32_t iv, bool encrypted, std::vector<uint8_t>&& data) :
		device(device), block_number(block_number), size_category(size_category), iv(iv), encrypted(encrypted), data(data) {
	}
	virtual ~Block() {}

	std::shared_ptr<DeviceEncryption> device;

	uint32_t block_number;
	Block::BlockSize size_category;
	uint32_t iv;
	bool encrypted;

	// this vector will be rounded to sector after read
	std::vector<uint8_t> data;
};
