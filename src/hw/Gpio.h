/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <OS.h>
#include <String.h>

#include "Config.h"
#include "PinTable.h"


namespace airpins {

struct PinHardwareState {
	uint8	function = kFunctionInput;	// kFunction*
	uint8	pull = 0;					// 0 none, 1 up, 2 down
	bool	level = false;
	bool	claimedHere = false;
	bool	claimedElsewhere = false;
};

struct HardwareState {
	bigtime_t			time = 0;
	PinHardwareState	pins[kBcmPinCount];
};

// GpioEvent.flags, as the driver's
enum {
	kEventClaimed = 0x01,
	kEventWritten = 0x02,
	kEventSampled = 0x04
};

struct GpioEvent {
	bigtime_t	time;		// system_time()
	uint8		pin;
	bool		level;
	uint8		flags;
};

struct HardwareDetails {
	BString		name;			// "Local GPIO", "Simulated GPIO"
	bool		simulated = false;
	uint32		revision = 0;	// the board's revision code
	uint64		serial = 0;
	bool		edgeInterrupts = false;
	uint32		apiVersion = 0;
};


/*!	The GPIO pins: the board's own through /dev/misc/rpi_gpio, or simulated
	ones. Pins configured through an object go back to what they were when
	it is deleted. WaitEvents() may run in a thread of its own while the
	other methods are called.
*/
class GpioHardware {
public:
	virtual						~GpioHardware() {}

	virtual	HardwareDetails		Details() const = 0;
	virtual	status_t			GetState(HardwareState& state) = 0;
	//! Input or output claims the pin; Unused gives it back.
	virtual	status_t			Configure(int pin,
									const PinSetting& setting) = 0;
	virtual	status_t			Write(int pin, bool level) = 0;
	virtual	status_t			WaitEvents(GpioEvent* events, int capacity,
									bigtime_t timeout, int& count,
									uint32& lost) = 0;
};

//! The board's pins, or NULL (and why in \a error) on another machine.
GpioHardware* CreateLocalGpio(BString* error = NULL);
GpioHardware* CreateSimulatedGpio();

}	// namespace airpins
