/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 *
 * The revision code layout is documented by Raspberry Pi Ltd at
 * https://www.raspberrypi.com/documentation/computers/raspberry-pi.html
 * ("New-style revision codes").
 */

#include "BoardInfo.h"

#include <stdio.h>


namespace airpins {

namespace {

const char*
TypeName(uint32_t type)
{
	switch (type) {
		case 0x00: return "Model A";
		case 0x01: return "Model B";
		case 0x02: return "Model A+";
		case 0x03: return "Model B+";
		case 0x04: return "2 Model B";
		case 0x06: return "Compute Module 1";
		case 0x08: return "3 Model B";
		case 0x09: return "Zero";
		case 0x0a: return "Compute Module 3";
		case 0x0c: return "Zero W";
		case 0x0d: return "3 Model B+";
		case 0x0e: return "3 Model A+";
		case 0x10: return "Compute Module 3+";
		case 0x11: return "4 Model B";
		case 0x12: return "Zero 2 W";
		case 0x13: return "400";
		case 0x14: return "Compute Module 4";
		case 0x15: return "Compute Module 4S";
		case 0x17: return "5";
		case 0x18: return "Compute Module 5";
		case 0x19: return "500";
		case 0x1a: return "Compute Module 5 Lite";
		default: return NULL;
	}
}

}	// namespace


BoardInfo
DecodeRevision(uint32_t revision)
{
	BoardInfo info;
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%x", (unsigned)revision);
	info.revisionCode = buffer;

	// only new-style codes (bit 23) are decoded
	if ((revision & (1u << 23)) == 0)
		return info;

	const char* type = TypeName((revision >> 4) & 0xff);
	if (type == NULL)
		return info;

	snprintf(buffer, sizeof(buffer), "Raspberry Pi %s Rev 1.%u", type,
		(unsigned)(revision & 0xf));
	info.model = buffer;

	static const char* const kProcessors[] = {
		"BCM2835", "BCM2836", "BCM2837", "BCM2711", "BCM2712"
	};
	uint32_t processor = (revision >> 12) & 0xf;
	info.processor = processor < 5 ? kProcessors[processor] : "unknown";

	static const char* const kMemory[] = {
		"256 MB", "512 MB", "1 GB", "2 GB", "4 GB", "8 GB", "16 GB"
	};
	uint32_t memory = (revision >> 20) & 7;
	info.memory = memory < 7 ? kMemory[memory] : "unknown";

	static const char* const kManufacturers[] = {
		"Sony UK", "Egoman", "Embest", "Sony Japan", "Embest", "Stadium"
	};
	uint32_t manufacturer = (revision >> 16) & 0xf;
	info.manufacturer = manufacturer < 6 ? kManufacturers[manufacturer]
		: "unknown";

	info.known = true;
	return info;
}

}	// namespace airpins
