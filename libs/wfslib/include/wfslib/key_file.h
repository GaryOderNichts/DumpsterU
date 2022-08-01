/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <stdexcept>
#include <string>
#include <vector>

class KeyFile {
 public:
  KeyFile(const std::vector<std::byte>& data, size_t expected_size) : data_(std::move(data)) {
    if (data_.size() != expected_size)
      throw std::runtime_error("Unexpected key file size");
  }

 protected:
  template <class T>
  static T* LoadFromFile(const std::string& path, size_t size);

  std::vector<std::byte> data_;

  std::vector<std::byte> GetKey(size_t offset, size_t size) const;
};

class OTP : KeyFile {
 public:
  static const size_t OTP_SIZE = 0x400;

  OTP(const std::vector<std::byte>& data) : KeyFile(data, OTP_SIZE) {}

  static OTP* LoadFromFile(const std::string& path);

  std::vector<std::byte> GetMLCKey() const;
  std::vector<std::byte> GetUSBSeedEncryptionKey() const;
};

class SEEPROM : KeyFile {
 public:
  static const size_t SEEPROM_SIZE = 0x200;

  SEEPROM(const std::vector<std::byte>& data) : KeyFile(data, SEEPROM_SIZE) {}

  static SEEPROM* LoadFromFile(const std::string& path);

  std::vector<std::byte> GetUSBKeySeed() const;
  std::vector<std::byte> GetUSBKey(const OTP& otp) const;
};
