/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <stdint.h>
#include <string>


namespace airpins {

//! What a Raspberry Pi revision code says about the board.
struct BoardInfo {
	bool		known = false;
	std::string	model;			// "Raspberry Pi 4 Model B Rev 1.5"
	std::string	processor;		// "BCM2711"
	std::string	memory;			// "4 GB"
	std::string	manufacturer;	// "Sony UK"
	std::string	revisionCode;	// "c03115"
};

BoardInfo DecodeRevision(uint32_t revision);

}	// namespace airpins
