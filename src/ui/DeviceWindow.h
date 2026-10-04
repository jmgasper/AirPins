/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <Window.h>

#include "Gpio.h"


namespace airpins {

//! What is known about the board and its GPIO, like pigg's device details.
class DeviceWindow : public BWindow {
public:
								DeviceWindow(BWindow* parent,
									const HardwareDetails& details);

	virtual	void				MessageReceived(BMessage* message);

private:
			BString				fText;
};

}	// namespace airpins
