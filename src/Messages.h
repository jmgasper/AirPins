/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <SupportDefs.h>


namespace airpins {

static const char* const kAppSignature = "application/x-vnd.airOS-AirPins";
static const char* const kConfigMimeType = "application/x-vnd.pigg-config";
static const char* const kAppName = "AirPins";
static const char* const kPiggUrl = "https://github.com/andrewdavidmackenzie/pigg";

enum {
	// from the event pump: "events" (GpioEvent[]), "lost" (int32)
	kMsgPinEvents			= 'pevt',

	// pin controls: "pin" (int32) and
	kMsgSetPinMode			= 'pmod',	// "mode" (int32 PinMode)
	kMsgSetPinPull			= 'ppul',	// "pull" (int32 Pull)
	kMsgSetPinFunction		= 'pfun',	// "mode", and "pull" for inputs
	kMsgToggleOutput		= 'ptgl',
	kMsgHoldOutput			= 'phld',	// "pressed" (bool)

	kMsgOpen				= 'open',
	kMsgSave				= 'save',
	kMsgSaveAs				= 'svas',
	kMsgResetPins			= 'rset',
	kMsgSetLayout			= 'layt',	// "layout" (int32 LayoutMode)
	kMsgSetSpan				= 'span',	// "seconds" (int32)
	kMsgDeviceDetails		= 'devd',
	kMsgUseSimulated		= 'simu',
	kMsgVisitPigg			= 'pigg',

	kMsgTick				= 'tick',
	kMsgRefreshState		= 'rfsh',
	kMsgClearMessage		= 'clrm'
};

enum class LayoutMode : int32 { Board = 0, Bcm = 1, Compact = 2 };

}	// namespace airpins
