/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <vector>
#include <memory>

class Device;
class FileDevice;
class DeviceEncryption;
class Area;
class WfsItem;
class File;
class Directory;

class Wfs {
public:
	Wfs(const std::shared_ptr<Device>& device, std::vector<uint8_t>& key);

	const std::shared_ptr<DeviceEncryption>& GetDevice() {
		return device;
	}

	std::shared_ptr<WfsItem> GetObject(const std::string& filename);
	std::shared_ptr<File> GetFile(const std::string& filename);
	std::shared_ptr<Directory> GetDirectory(const std::string& filename);

	static void DetectDeviceSectorSizeAndCount(const std::shared_ptr<FileDevice>& device, const std::vector<uint8_t>& key);

private:
	std::shared_ptr<DeviceEncryption> device;

	friend class Area;
	std::shared_ptr<Area> root_area;

	static bool VerifyDeviceAndKey(const std::shared_ptr<FileDevice>& device, const std::vector<uint8_t>& key);
};