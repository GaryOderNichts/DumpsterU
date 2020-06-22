/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "KeyFile.h"

#include <fstream>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>

template<class T>
T * KeyFile::LoadFromFile(const std::string& path, size_t size) {
	static_assert(std::is_base_of<KeyFile, T>::value, "T must be a descendant of KeyFile");

	std::fstream file(path, std::ios::binary | std::ios::in);
	if (file.fail()) {
		throw std::runtime_error("KeyFile: Failed to open file");
	}

	std::vector<uint8_t> data(size);
	file.read(reinterpret_cast<char*>(&*data.begin()), size);
	if (file.gcount() != static_cast<std::streamsize>(size))
		throw std::runtime_error("KeyFile: KeyFile too small");
	return new T(data);
}

OTP * OTP::LoadFromFile(const std::string& path) {
	return KeyFile::LoadFromFile<OTP>(path, OTP_SIZE);
}

SEEPROM * SEEPROM::LoadFromFile(const std::string& path) {
	return KeyFile::LoadFromFile<SEEPROM>(path, SEEPROM_SIZE);
}

std::vector<uint8_t> KeyFile::GetKey(size_t offset, size_t size) const {
	std::vector<uint8_t> key(size);
	std::copy(data.begin() + offset, data.begin() + offset + size, key.begin());
	return key;
}

std::vector<uint8_t> OTP::GetMLCKey() const {
	return GetKey(0x180, 0x10);
}

std::vector<uint8_t> OTP::GetUSBSeedEncryptionKey() const {
	return GetKey(0x130, 0x10);
}

std::vector<uint8_t> SEEPROM::GetUSBKeySeed() const {
	return GetKey(0xB0, 0x10);
}

std::vector<uint8_t> SEEPROM::GetUSBKey(const OTP& otp) const {
	std::vector<uint8_t> key(GetUSBKeySeed());
	std::vector<uint8_t> enc_key(otp.GetUSBSeedEncryptionKey());
	CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption encryptor(&*enc_key.begin(), enc_key.size());
	encryptor.ProcessData(&*key.begin(), &*key.begin(), key.size());
	return key;
}