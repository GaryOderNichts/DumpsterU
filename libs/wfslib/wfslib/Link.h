/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <memory>

#include "WfsItem.h"

class Area;

class Link : public WfsItem, public std::enable_shared_from_this<Link> {
public:
	Link(const std::string& name, const AttributesBlock& attributes, const std::shared_ptr<Area>& area) : WfsItem(name, attributes), area(area) {
	}

private:
	// TODO: We may have cyclic reference here if we do cache in area.
	std::shared_ptr<Area> area;
};
