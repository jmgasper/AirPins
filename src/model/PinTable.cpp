/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 *
 * The header layout, pin colours and the choice of configurable pins follow
 * pigg (https://github.com/andrewdavidmackenzie/pigg, Apache-2.0). The
 * alternate functions are those of the BCM2711 ARM Peripherals datasheet,
 * section 5.3.
 */

#include "PinTable.h"

#include <string.h>


namespace airpins {

namespace {

const HeaderPin kHeader[kHeaderPinCount] = {
	{ 1, -1, PinKind::Power3V3, "3V3" },
	{ 2, -1, PinKind::Power5V, "5V" },
	{ 3, 2, PinKind::Gpio, "GPIO2" },
	{ 4, -1, PinKind::Power5V, "5V" },
	{ 5, 3, PinKind::Gpio, "GPIO3" },
	{ 6, -1, PinKind::Ground, "Ground" },
	{ 7, 4, PinKind::Gpio, "GPIO4" },
	{ 8, 14, PinKind::Gpio, "GPIO14" },
	{ 9, -1, PinKind::Ground, "Ground" },
	{ 10, 15, PinKind::Gpio, "GPIO15" },
	{ 11, 17, PinKind::Gpio, "GPIO17" },
	{ 12, 18, PinKind::Gpio, "GPIO18" },
	{ 13, 27, PinKind::Gpio, "GPIO27" },
	{ 14, -1, PinKind::Ground, "Ground" },
	{ 15, 22, PinKind::Gpio, "GPIO22" },
	{ 16, 23, PinKind::Gpio, "GPIO23" },
	{ 17, -1, PinKind::Power3V3, "3V3" },
	{ 18, 24, PinKind::Gpio, "GPIO24" },
	{ 19, 10, PinKind::Gpio, "GPIO10" },
	{ 20, -1, PinKind::Ground, "Ground" },
	{ 21, 9, PinKind::Gpio, "GPIO9" },
	{ 22, 25, PinKind::Gpio, "GPIO25" },
	{ 23, 11, PinKind::Gpio, "GPIO11" },
	{ 24, 8, PinKind::Gpio, "GPIO8" },
	{ 25, -1, PinKind::Ground, "Ground" },
	{ 26, 7, PinKind::Gpio, "GPIO7" },
	{ 27, 0, PinKind::IdEeprom, "ID_SD" },
	{ 28, 1, PinKind::IdEeprom, "ID_SC" },
	{ 29, 5, PinKind::Gpio, "GPIO5" },
	{ 30, -1, PinKind::Ground, "Ground" },
	{ 31, 6, PinKind::Gpio, "GPIO6" },
	{ 32, 12, PinKind::Gpio, "GPIO12" },
	{ 33, 13, PinKind::Gpio, "GPIO13" },
	{ 34, -1, PinKind::Ground, "Ground" },
	{ 35, 19, PinKind::Gpio, "GPIO19" },
	{ 36, 16, PinKind::Gpio, "GPIO16" },
	{ 37, 26, PinKind::Gpio, "GPIO26" },
	{ 38, 20, PinKind::Gpio, "GPIO20" },
	{ 39, -1, PinKind::Ground, "Ground" },
	{ 40, 21, PinKind::Gpio, "GPIO21" },
};

// ALT0 .. ALT5 of GPIO 0 to 27; NULL: reserved
const char* const kAlt[kBcmPinCount][6] = {
	{ "SDA0", "SA5", "PCLK", "SPI3_CE0_N", "TXD2", "SDA6" },
	{ "SCL0", "SA4", "DE", "SPI3_MISO", "RXD2", "SCL6" },
	{ "SDA1", "SA3", "LCD_VSYNC", "SPI3_MOSI", "CTS2", "SDA3" },
	{ "SCL1", "SA2", "LCD_HSYNC", "SPI3_SCLK", "RTS2", "SCL3" },
	{ "GPCLK0", "SA1", "DPI_D0", "SPI4_CE0_N", "TXD3", "SDA3" },
	{ "GPCLK1", "SA0", "DPI_D1", "SPI4_MISO", "RXD3", "SCL3" },
	{ "GPCLK2", "SOE_N", "DPI_D2", "SPI4_MOSI", "CTS3", "SDA4" },
	{ "SPI0_CE1_N", "SWE_N", "DPI_D3", "SPI4_SCLK", "RTS3", "SCL4" },
	{ "SPI0_CE0_N", "SD0", "DPI_D4", "BSCSL_CE_N", "TXD4", "SDA4" },
	{ "SPI0_MISO", "SD1", "DPI_D5", "BSCSL_MISO", "RXD4", "SCL4" },
	{ "SPI0_MOSI", "SD2", "DPI_D6", "BSCSL_SDA", "CTS4", "SDA5" },
	{ "SPI0_SCLK", "SD3", "DPI_D7", "BSCSL_SCL", "RTS4", "SCL5" },
	{ "PWM0_0", "SD4", "DPI_D8", "SPI5_CE0_N", "TXD5", "SDA5" },
	{ "PWM0_1", "SD5", "DPI_D9", "SPI5_MISO", "RXD5", "SCL5" },
	{ "TXD0", "SD6", "DPI_D10", "SPI5_MOSI", "CTS5", "TXD1" },
	{ "RXD0", "SD7", "DPI_D11", "SPI5_SCLK", "RTS5", "RXD1" },
	{ NULL, "SD8", "DPI_D12", "CTS0", "SPI1_CE2_N", "CTS1" },
	{ NULL, "SD9", "DPI_D13", "RTS0", "SPI1_CE1_N", "RTS1" },
	{ "PCM_CLK", "SD10", "DPI_D14", "SPI6_CE0_N", "SPI1_CE0_N", "PWM0_0" },
	{ "PCM_FS", "SD11", "DPI_D15", "SPI6_MISO", "SPI1_MISO", "PWM0_1" },
	{ "PCM_DIN", "SD12", "DPI_D16", "SPI6_MOSI", "SPI1_MOSI", "GPCLK0" },
	{ "PCM_DOUT", "SD13", "DPI_D17", "SPI6_SCLK", "SPI1_SCLK", "GPCLK1" },
	{ "SD0_CLK", "SD14", "DPI_D18", "SD1_CLK", "ARM_TRST", "SDA6" },
	{ "SD0_CMD", "SD15", "DPI_D19", "SD1_CMD", "ARM_RTCK", "SCL6" },
	{ "SD0_DAT0", "SD16", "DPI_D20", "SD1_DAT0", "ARM_TDO", "SPI3_CE1_N" },
	{ "SD0_DAT1", "SD17", "DPI_D21", "SD1_DAT1", "ARM_TCK", "SPI4_CE1_N" },
	{ "SD0_DAT2", NULL, "DPI_D22", "SD1_DAT2", "ARM_TDI", "SPI5_CE1_N" },
	{ "SD0_DAT3", NULL, "DPI_D23", "SD1_DAT3", "ARM_TMS", "SPI6_CE1_N" },
};

struct Group {
	const char* prefix;
	const char* name;
};

// longest prefixes first where they overlap
const Group kGroups[] = {
	{ "SPI0_", "SPI0" }, { "SPI1_", "SPI1" }, { "SPI3_", "SPI3" },
	{ "SPI4_", "SPI4" }, { "SPI5_", "SPI5" }, { "SPI6_", "SPI6" },
	{ "BSCSL", "I2C/SPI slave" }, { "PCM_", "PCM audio" },
	{ "PWM0_", "PWM0" }, { "GPCLK", "general purpose clock" },
	{ "DPI_", "parallel display" }, { "LCD_", "parallel display" },
	{ "PCLK", "parallel display" }, { "DE", "parallel display" },
	{ "SD0_", "SD host 0" }, { "SD1_", "SD host 1" }, { "ARM_", "JTAG" },
	{ "SDA0", "I2C0" }, { "SCL0", "I2C0" }, { "SDA1", "I2C1" },
	{ "SCL1", "I2C1" }, { "SDA3", "I2C3" }, { "SCL3", "I2C3" },
	{ "SDA4", "I2C4" }, { "SCL4", "I2C4" }, { "SDA5", "I2C5" },
	{ "SCL5", "I2C5" }, { "SDA6", "I2C6" }, { "SCL6", "I2C6" },
	{ "TXD0", "UART0" }, { "RXD0", "UART0" }, { "CTS0", "UART0" },
	{ "RTS0", "UART0" }, { "TXD1", "mini UART" }, { "RXD1", "mini UART" },
	{ "CTS1", "mini UART" }, { "RTS1", "mini UART" },
	{ "TXD2", "UART2" }, { "RXD2", "UART2" }, { "CTS2", "UART2" },
	{ "RTS2", "UART2" }, { "TXD3", "UART3" }, { "RXD3", "UART3" },
	{ "CTS3", "UART3" }, { "RTS3", "UART3" }, { "TXD4", "UART4" },
	{ "RXD4", "UART4" }, { "CTS4", "UART4" }, { "RTS4", "UART4" },
	{ "TXD5", "UART5" }, { "RXD5", "UART5" }, { "CTS5", "UART5" },
	{ "RTS5", "UART5" },
	{ "SA", "secondary memory" }, { "SD", "secondary memory" },
	{ "SOE_N", "secondary memory" }, { "SWE_N", "secondary memory" },
};

}	// namespace


const HeaderPin&
HeaderPinAt(int index)
{
	if (index < 0 || index >= kHeaderPinCount)
		index = 0;
	return kHeader[index];
}


const HeaderPin*
HeaderPinForBcm(int bcm)
{
	for (const HeaderPin& pin : kHeader) {
		if (pin.bcm == bcm)
			return &pin;
	}
	return NULL;
}


bool
IsConfigurable(int bcm)
{
	const HeaderPin* pin = HeaderPinForBcm(bcm);
	return pin != NULL && pin->kind == PinKind::Gpio;
}


PinColor
ColorFor(const HeaderPin& pin)
{
	switch (pin.kind) {
		case PinKind::Power3V3:
			return { 255, 235, 4, 0, 0, 0 };		// yellow
		case PinKind::Power5V:
			return { 230, 30, 30, 255, 255, 255 };	// red
		case PinKind::Ground:
			return { 20, 20, 20, 255, 255, 255 };	// black
		case PinKind::IdEeprom:
			return { 150, 150, 150, 0, 0, 0 };		// grey: reserved
		case PinKind::Gpio:
			break;
	}

	switch (pin.bcm) {
		case 2: case 3:								// I2C1: light blue
			return { 173, 216, 230, 0, 0, 0 };
		case 7: case 8: case 9: case 10: case 11:	// SPI0: violet
			return { 238, 130, 238, 255, 255, 255 };
		case 14: case 15:							// UART: green
			return { 0, 128, 0, 255, 255, 255 };
		default:									// orange
			return { 255, 165, 0, 255, 255, 255 };
	}
}


const char*
AltSignal(int bcm, int alt)
{
	if (bcm < 0 || bcm >= kBcmPinCount || alt < 0 || alt > 5)
		return NULL;
	return kAlt[bcm][alt];
}


int
AltIndex(int function)
{
	switch (function) {
		case kFunctionAlt0: return 0;
		case kFunctionAlt1: return 1;
		case kFunctionAlt2: return 2;
		case kFunctionAlt3: return 3;
		case kFunctionAlt4: return 4;
		case kFunctionAlt5: return 5;
		default: return -1;
	}
}


const char*
FunctionSelectName(int function)
{
	static const char* const kNames[] = {
		"ALT0", "ALT1", "ALT2", "ALT3", "ALT4", "ALT5"
	};
	int alt = AltIndex(function);
	return alt < 0 ? NULL : kNames[alt];
}


const char*
SignalGroup(const char* signal)
{
	if (signal == NULL)
		return NULL;
	for (const Group& group : kGroups) {
		if (strncmp(signal, group.prefix, strlen(group.prefix)) == 0)
			return group.name;
	}
	return NULL;
}


bool
DefaultPullUp(int bcm)
{
	return bcm >= 0 && bcm <= 8;
}

}	// namespace airpins
