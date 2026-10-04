/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "Gpio.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "rpi_gpio.h"


namespace airpins {

namespace {

class LocalGpio : public GpioHardware {
public:
	LocalGpio(int fd, const rpi_gpio_info& info)
		:
		fDevice(fd),
		fInfo(info)
	{
	}

	~LocalGpio() override
	{
		// the driver gives every claimed pin back
		close(fDevice);
	}

	HardwareDetails Details() const override
	{
		HardwareDetails details;
		details.name = "Local GPIO";
		details.revision = fInfo.board_revision;
		details.serial = fInfo.board_serial;
		details.edgeInterrupts
			= (fInfo.flags & RPI_GPIO_INFO_EDGE_INTERRUPTS) != 0;
		details.apiVersion = fInfo.api_version;
		return details;
	}

	status_t GetState(HardwareState& state) override
	{
		rpi_gpio_state raw;
		status_t status = _Control(RPI_GPIO_GET_STATE, &raw);
		if (status != B_OK)
			return status;

		state.time = raw.time;
		for (int pin = 0; pin < kBcmPinCount; pin++) {
			PinHardwareState& out = state.pins[pin];
			out.function = raw.function[pin];
			out.pull = raw.pull[pin];
			out.level = (raw.levels >> pin) & 1;
			out.claimedHere = (raw.claimed >> pin) & 1;
			out.claimedElsewhere = (raw.claimed_elsewhere >> pin) & 1;
		}
		return B_OK;
	}

	status_t Configure(int pin, const PinSetting& setting) override
	{
		if (setting.mode == PinMode::Unused) {
			uint32 number = pin;
			status_t status = _Control(RPI_GPIO_RELEASE, &number);
			// not claimed: nothing to give back
			return status == B_BAD_VALUE ? B_OK : status;
		}

		rpi_gpio_claim request = {};
		request.pin = pin;
		if (setting.mode == PinMode::Input) {
			request.function = RPI_GPIO_INPUT;
			request.pull = setting.pull == Pull::Up ? RPI_GPIO_PULL_UP
				: setting.pull == Pull::Down ? RPI_GPIO_PULL_DOWN
				: RPI_GPIO_PULL_NONE;
			request.level = -1;
		} else {
			request.function = RPI_GPIO_OUTPUT;
			request.pull = RPI_GPIO_PULL_NONE;
			request.level = setting.level < 0 ? -1 : setting.level != 0;
		}
		return _Control(RPI_GPIO_CLAIM, &request);
	}

	status_t Write(int pin, bool level) override
	{
		rpi_gpio_write request;
		request.mask = 1ull << pin;
		request.levels = (uint64)level << pin;
		return _Control(RPI_GPIO_WRITE, &request);
	}

	status_t WaitEvents(GpioEvent* events, int capacity, bigtime_t timeout,
		int& count, uint32& lost) override
	{
		rpi_gpio_event raw[64];
		rpi_gpio_wait request = {};
		request.timeout = timeout;
		request.events = raw;
		request.capacity = capacity < 64 ? capacity : 64;
		count = 0;
		lost = 0;
		status_t status = _Control(RPI_GPIO_WAIT_EVENTS, &request);
		if (status != B_OK)
			return status;

		lost = request.lost;
		for (uint32 i = 0; i < request.count; i++) {
			events[i].time = raw[i].time;
			events[i].pin = raw[i].pin;
			events[i].level = raw[i].level != 0;
			events[i].flags = raw[i].flags;
		}
		count = request.count;
		return B_OK;
	}

private:
	status_t _Control(uint32 op, void* data)
	{
		return ioctl(fDevice, op, data, 0) == 0 ? B_OK : errno;
	}

	int				fDevice;
	rpi_gpio_info	fInfo;
};

}	// namespace


GpioHardware*
CreateLocalGpio(BString* error)
{
	int fd = open(RPI_GPIO_DEVICE_PATH, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		if (error != NULL) {
			error->SetToFormat("%s: %s", RPI_GPIO_DEVICE_PATH,
				strerror(errno));
		}
		return NULL;
	}

	rpi_gpio_info info;
	if (ioctl(fd, RPI_GPIO_GET_INFO, &info, sizeof(info)) != 0
		|| info.api_version != RPI_GPIO_API_VERSION) {
		if (error != NULL)
			error->SetTo("The GPIO driver is of another version.");
		close(fd);
		return NULL;
	}
	return new LocalGpio(fd, info);
}

}	// namespace airpins
