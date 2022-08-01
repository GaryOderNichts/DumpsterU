/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "key_file.h"

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <fstream>

template <class T>
T* KeyFile::LoadFromFile(const std::string& path, size_t size) {
  static_assert(std::is_base_of<KeyFile, T>::value, "T must be a descendant of KeyFile");

  std::fstream file(path, std::ios::binary | std::ios::in);
  if (file.fail()) {
    throw std::runtime_error("KeyFile: Failed to open file");
  }

  std::vector<std::byte> data(size);
  file.read(reinterpret_cast<char*>(data.data()), size);
  if (file.gcount() != static_cast<std::streamsize>(size))
    throw std::runtime_error("KeyFile: KeyFile too small");
  return new T(data);
}

OTP* OTP::LoadFromFile(const std::string& path) {
  return KeyFile::LoadFromFile<OTP>(path, OTP_SIZE);
}

SEEPROM* SEEPROM::LoadFromFile(const std::string& path) {
  return KeyFile::LoadFromFile<SEEPROM>(path, SEEPROM_SIZE);
}

std::vector<std::byte> KeyFile::GetKey(size_t offset, size_t size) const {
  std::vector<std::byte> key(size);
  std::copy(data_.begin() + offset, data_.begin() + offset + size, key.begin());
  return key;
}

std::vector<std::byte> OTP::GetMLCKey() const {
  return GetKey(0x180, 0x10);
}

std::vector<std::byte> OTP::GetUSBSeedEncryptionKey() const {
  return GetKey(0x130, 0x10);
}

std::vector<std::byte> SEEPROM::GetUSBKeySeed() const {
  return GetKey(0xB0, 0x10);
}

std::vector<std::byte> SEEPROM::GetUSBKey(const OTP& otp) const {
  std::vector<std::byte> key(GetUSBKeySeed());
  std::vector<std::byte> enc_key(otp.GetUSBSeedEncryptionKey());
  CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption encryptor(reinterpret_cast<const uint8_t*>(enc_key.data()),
                                                          enc_key.size());
  encryptor.ProcessData(reinterpret_cast<uint8_t*>(key.data()), reinterpret_cast<uint8_t*>(key.data()), key.size());
  return key;
}
