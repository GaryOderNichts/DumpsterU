/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <cryptopp/sha.h>
#include <memory>
#include <span>
#include <vector>

class Device;
struct WfsBlockIV;

class DeviceEncryption {
 public:
  DeviceEncryption(const std::shared_ptr<Device>& device, const std::span<std::byte>& key);

  void WriteBlock(uint32_t sector_address, const std::span<std::byte>& data, uint32_t iv, bool encrypt);
  void ReadBlock(uint32_t sector_address, const std::span<std::byte>& data, uint32_t iv, bool encrypt);

  void CalculateHash(const std::span<std::byte>& data, const std::span<std::byte>& hash);
  bool CheckHash(const std::span<std::byte>& data, const std::span<std::byte>& hash);

  const std::shared_ptr<Device>& GetDevice() { return device_; }

  static constexpr size_t DIGEST_SIZE = CryptoPP::SHA1::DIGESTSIZE;

 private:
  void HashData(const std::span<std::byte>& data, const std::span<std::byte>& hash);
  WfsBlockIV GetIV(uint32_t sectors_count, uint32_t iv);

  std::shared_ptr<Device> device_;

  const std::vector<std::byte> key_;
};
