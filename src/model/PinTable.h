/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 *
 * The header layout, pin colours and the choice of configurable pins follow
 * pigg (https://github.com/andrewdavidmackenzie/pigg, Apache-2.0).
 */
#pragma once

#include <stdint.h>


namespace airpins {

enum class PinKind {
	Power3V3,
	Power5V,
	Ground,
	Gpio,		// configurable
	IdEeprom	// GPIO 0/1: the HAT ID EEPROM's I2C bus, not configurable
};

struct HeaderPin {
	uint8_t		board;		// 1..40, the physical pin number
	int8_t		bcm;		// GPIO number, or -1
	PinKind		kind;
	const char*	name;		// "GPIO17", "3V3", "Ground"...
};

struct PinColor {
	uint8_t red, green, blue;
	uint8_t textRed, textGreen, textBlue;
};

static const int kHeaderPinCount = 40;
static const int kBcmPinCount = 28;			// GPIO 0..27 reach the header

// GPIO function select values of the BCM2711 (the same as the driver's)
enum {
	kFunctionInput = 0,
	kFunctionOutput = 1,
	kFunctionAlt5 = 2,
	kFunctionAlt4 = 3,
	kFunctionAlt0 = 4,
	kFunctionAlt1 = 5,
	kFunctionAlt2 = 6,
	kFunctionAlt3 = 7
};

const HeaderPin& HeaderPinAt(int index);	// 0..39, in board order
const HeaderPin* HeaderPinForBcm(int bcm);	// NULL if not on the header
bool IsConfigurable(int bcm);

PinColor ColorFor(const HeaderPin& pin);

//! ALT0..ALT5 signal name of a GPIO ("TXD0"), NULL when reserved.
const char* AltSignal(int bcm, int alt);
//! The signal of a function select value, "ALT0 TXD0" style, or NULL
//! for input and output.
const char* FunctionSelectName(int function);
int AltIndex(int function);					// -1 for input/output
//! A short description of what a signal belongs to ("UART0", "I2C1").
const char* SignalGroup(const char* signal);

//! The pull resistor of a GPIO after reset (BCM2711 datasheet).
bool DefaultPullUp(int bcm);

}	// namespace airpins
