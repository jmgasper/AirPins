/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 *
 * Configuration files are pigg's ".pigg" files, so the two programs can
 * open each other's: {"pin_functions":{"26":{"Input":"PullUp"},
 * "17":{"Output":true}}}, keyed by GPIO (BCM) number.
 */
#pragma once

#include <map>
#include <string>
#include <vector>


namespace airpins {

enum class PinMode { Unused, Input, Output };
enum class Pull { None, Up, Down };

struct PinSetting {
	PinMode	mode = PinMode::Unused;
	Pull	pull = Pull::None;	// inputs
	int		level = -1;			// outputs: 0, 1 or -1 (as it is)

	bool operator==(const PinSetting& other) const
	{
		return mode == other.mode
			&& (mode != PinMode::Input || pull == other.pull)
			&& (mode != PinMode::Output || level == other.level);
	}
	bool operator!=(const PinSetting& other) const
		{ return !(*this == other); }
};

struct Configuration {
	std::map<int, PinSetting>	pins;	// GPIO number -> setting; no Unused

	bool operator==(const Configuration& other) const
		{ return pins == other.pins; }
	bool operator!=(const Configuration& other) const
		{ return !(*this == other); }
};

/*!	Reads a configuration. Pins that are not configurable on the header
	are left out and named in \a skipped.
*/
bool ParseConfiguration(const std::string& text, Configuration& config,
	std::string* error = NULL, std::vector<int>* skipped = NULL);
std::string FormatConfiguration(const Configuration& config);

const char* PullName(Pull pull);		// "Pull up", "Pull down", "No pull"

}	// namespace airpins
