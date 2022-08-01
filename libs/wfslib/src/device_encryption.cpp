/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "device_encryption.h"

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha.h>
#include <array>
#include <boost/endian/buffers.hpp>
#include <cassert>
#include "device.h"
#include "utils.h"

struct WfsBlockIV {
  boost::endian::big_uint32_buf_t iv[4];
};

DeviceEncryption::DeviceEncryption(const std::shared_ptr<Device>& device, const std::span<std::byte>& key)
    : device_(device), key_(key.begin(), key.end()) {}

void DeviceEncryption::HashData(const std::span<std::byte>& data, const std::span<std::byte>& hash) {
  assert(data.size() % device_->SectorSize() == 0);
  // Pad and hash
  CryptoPP::SHA1().CalculateDigest(reinterpret_cast<uint8_t*>(hash.data()), reinterpret_cast<uint8_t*>(data.data()),
                                   data.size());
}

void DeviceEncryption::CalculateHash(const std::span<std::byte>& data, const std::span<std::byte>& hash) {
  bool hash_in_block = data.size() >= hash.size() && data.data() <= hash.data() &&
                       hash.data() + hash.size() <= data.data() + data.size();
  // Fill hash space with 0xFF
  if (hash_in_block)
    std::ranges::fill(hash, std::byte{0xFF});

  HashData(data, hash);
}

void DeviceEncryption::WriteBlock(uint32_t sector_address,
                                  const std::span<std::byte>& data,
                                  uint32_t iv,
                                  bool encrypt) {
  assert(data.size() % device_->SectorSize() == 0);
  auto const sectors_count = static_cast<uint32_t>(data.size() / device_->SectorSize());

  std::vector<std::byte> enc_data(data.begin(), data.end());
  if (encrypt) {
    // Encrypt
    auto _iv = GetIV(sectors_count, iv);
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor(reinterpret_cast<const uint8_t*>(key_.data()), key_.size(),
                                                            reinterpret_cast<uint8_t*>(&_iv));
    encryptor.ProcessData(reinterpret_cast<uint8_t*>(enc_data.data()), reinterpret_cast<uint8_t*>(enc_data.data()),
                          enc_data.size());
  }

  // Write
  device_->WriteSectors(enc_data, sector_address, sectors_count);
}

void DeviceEncryption::ReadBlock(uint32_t sector_address, const std::span<std::byte>& data, uint32_t iv, bool encrypt) {
  assert(data.size() % device_->SectorSize() == 0);
  auto const sectors_count = static_cast<uint32_t>(data.size() / device_->SectorSize());

  device_->ReadSectors(data, sector_address, sectors_count);

  if (encrypt) {
    auto _iv = GetIV(sectors_count, iv);
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor(reinterpret_cast<const uint8_t*>(key_.data()), key_.size(),
                                                            reinterpret_cast<uint8_t*>(&_iv));
    decryptor.ProcessData(reinterpret_cast<uint8_t*>(data.data()), reinterpret_cast<uint8_t*>(data.data()),
                          data.size());
  }
}

bool DeviceEncryption::CheckHash(const std::span<std::byte>& data, const std::span<std::byte>& hash) {
  std::array<std::byte, DIGEST_SIZE> placeholder_hash;

  bool hash_in_block = data.size() >= hash.size() && data.data() <= hash.data() &&
                       hash.data() + hash.size() <= data.data() + data.size();

  if (hash_in_block) {
    placeholder_hash.fill(std::byte{0xFF});
    std::ranges::swap_ranges(placeholder_hash, hash);
  }

  std::array<std::byte, DIGEST_SIZE> calculated_hash;
  HashData(data, calculated_hash);

  if (hash_in_block)
    std::ranges::swap_ranges(placeholder_hash, hash);

  return std::ranges::equal(calculated_hash, hash);
}

WfsBlockIV DeviceEncryption::GetIV(uint32_t sectors_count, uint32_t iv) {
  WfsBlockIV aes_iv;
  aes_iv.iv[0] = sectors_count * device_->SectorSize();
  aes_iv.iv[1] = iv;
  aes_iv.iv[2] = device_->SectorsCount();
  aes_iv.iv[3] = device_->SectorSize();
  return aes_iv;
}
