/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include "Config.h"
#include "Gpio.h"
#include "PinHistory.h"


namespace airpins {

//! Everything the window knows about one GPIO.
struct PinEntry {
	int					bcm = -1;
	PinSetting			setting;		// what the user configured
	PinHistory			history;		// levels while configured
	PinHardwareState	hardware;		// as last read from the hardware
	bool				holding = false;	// output inverted while pressed
	bool				holdLevel = false;	// the level it returns to

	bool Configured() const { return setting.mode != PinMode::Unused; }
	//! The level an output stands at when not held.
	bool StableLevel() const
	{
		if (setting.level >= 0)
			return setting.level != 0;
		return history.HasLevel() ? history.Level() : hardware.level;
	}
};

}	// namespace airpins
