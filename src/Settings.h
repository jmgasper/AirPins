/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <Message.h>


namespace airpins {

//! The window's frame, layout and waveform span, in the user's settings.
status_t LoadSettings(BMessage& settings);
status_t SaveSettings(const BMessage& settings);

}	// namespace airpins
