/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <vector>
#include <memory>
#include <cryptopp/sha.h>

class Device;
struct WfsBlockIV;

class DeviceEncryption {
public:
	DeviceEncryption(const std::shared_ptr<Device>& device, const std::vector<uint8_t>& key);

	void WriteBlock(uint32_t sector_address, const std::vector<uint8_t>& data, uint32_t iv, bool encrypt);
	std::vector<uint8_t> ReadBlock(uint32_t sector_address, uint32_t length, uint32_t iv, bool encrypt);

	void CalculateHash(const std::vector<uint8_t>& data, const std::vector<uint8_t>::iterator& hash, bool hash_in_block);
	bool CheckHash(const std::vector<uint8_t>& data, const std::vector<uint8_t>::iterator& hash, bool hash_in_block);

	const std::shared_ptr<Device>& GetDevice() { return device; }

	static constexpr size_t DIGEST_SIZE = CryptoPP::SHA1::DIGESTSIZE;

private:
	void HashData(const std::vector<uint8_t>& data, const std::vector<uint8_t>::iterator& hash);
	WfsBlockIV GetIV(uint32_t sectors_count, uint32_t iv);
	size_t ToSectorSize(size_t size);

	std::shared_ptr<Device> device;

	const std::vector<uint8_t> key;
};