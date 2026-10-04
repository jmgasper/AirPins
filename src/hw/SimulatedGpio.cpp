/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 *
 * Simulated pins for machines without GPIO, like pigg's "fake hardware":
 * an input sees a signal that changes at random intervals, an output is
 * whatever it is set to.
 */

#include "Gpio.h"

#include <Autolock.h>
#include <Locker.h>

#include <deque>


namespace airpins {

namespace {

class SimulatedGpio : public GpioHardware {
public:
	SimulatedGpio()
		:
		fLock("simulated gpio"),
		fWakeUp(create_sem(0, "simulated gpio events")),
		fRandom(0x41495250)		// "AIRP"
	{
		// as a Raspberry Pi 4 starts: GPIO 0 to 8 pulled up, the others
		// pulled down, GPIO 14 and 15 belong to the serial port
		for (int pin = 0; pin < kBcmPinCount; pin++) {
			Pin& state = fPins[pin];
			state.function = kFunctionInput;
			state.pull = DefaultPullUp(pin) ? 1 : 2;
			state.level = DefaultPullUp(pin);
			state.claimed = false;
			state.nextChange = 0;
		}
		fPins[14].function = fPins[15].function = kFunctionAlt0;
		fPins[14].level = fPins[15].level = true;
		fPins[14].pull = 0;
		fPins[15].pull = 1;
	}

	~SimulatedGpio() override
	{
		delete_sem(fWakeUp);
	}

	HardwareDetails Details() const override
	{
		HardwareDetails details;
		details.name = "Simulated GPIO";
		details.simulated = true;
		details.revision = 0xc03115;
		details.edgeInterrupts = true;
		return details;
	}

	status_t GetState(HardwareState& state) override
	{
		BAutolock locker(fLock);
		_Run(system_time());
		state.time = system_time();
		for (int pin = 0; pin < kBcmPinCount; pin++) {
			state.pins[pin].function = fPins[pin].function;
			state.pins[pin].pull = fPins[pin].pull;
			state.pins[pin].level = fPins[pin].level;
			state.pins[pin].claimedHere = fPins[pin].claimed;
			state.pins[pin].claimedElsewhere = false;
		}
		return B_OK;
	}

	status_t Configure(int pin, const PinSetting& setting) override
	{
		if (pin < 0 || pin >= kBcmPinCount)
			return B_BAD_VALUE;
		BAutolock locker(fLock);
		Pin& state = fPins[pin];
		bigtime_t now = system_time();

		if (setting.mode == PinMode::Unused) {
			if (state.claimed) {
				state.function = state.saved.function;
				state.pull = state.saved.pull;
				state.level = state.saved.level;
				state.claimed = false;
			}
			return B_OK;
		}
		if (!IsConfigurable(pin))
			return B_BAD_VALUE;

		if (!state.claimed) {
			state.saved.function = state.function;
			state.saved.pull = state.pull;
			state.saved.level = state.level;
		}
		bool wasInput = state.claimed && state.function == kFunctionInput;
		state.claimed = true;
		if (setting.mode == PinMode::Input) {
			state.function = kFunctionInput;
			state.pull = setting.pull == Pull::Up ? 1
				: setting.pull == Pull::Down ? 2 : 0;
			if (!wasInput)
				state.nextChange = now + _Interval();
			_Record(pin, state.level, kEventClaimed, now);
		} else {
			state.function = kFunctionOutput;
			state.pull = 0;
			if (setting.level >= 0)
				state.level = setting.level != 0;
			_Record(pin, state.level, kEventClaimed, now);
		}
		release_sem(fWakeUp);
		return B_OK;
	}

	status_t Write(int pin, bool level) override
	{
		if (pin < 0 || pin >= kBcmPinCount)
			return B_BAD_VALUE;
		BAutolock locker(fLock);
		Pin& state = fPins[pin];
		if (!state.claimed || state.function != kFunctionOutput)
			return B_NOT_ALLOWED;
		if (state.level != level) {
			state.level = level;
			_Record(pin, level, kEventWritten, system_time());
			release_sem(fWakeUp);
		}
		return B_OK;
	}

	status_t WaitEvents(GpioEvent* events, int capacity, bigtime_t timeout,
		int& count, uint32& lost) override
	{
		count = 0;
		lost = 0;
		bigtime_t deadline = system_time() + timeout;
		while (true) {
			bigtime_t next;
			{
				BAutolock locker(fLock);
				bigtime_t now = system_time();
				_Run(now);
				while (!fEvents.empty() && count < capacity) {
					events[count++] = fEvents.front();
					fEvents.pop_front();
				}
				if (count > 0 || now >= deadline)
					return B_OK;
				next = deadline;
				for (const Pin& pin : fPins) {
					if (pin.claimed && pin.function == kFunctionInput
						&& pin.nextChange < next) {
						next = pin.nextChange;
					}
				}
			}
			acquire_sem_etc(fWakeUp, 1, B_ABSOLUTE_TIMEOUT, next);
		}
	}

private:
	struct Pin {
		uint8		function;
		uint8		pull;
		bool		level;
		bool		claimed;
		bigtime_t	nextChange;
		struct {
			uint8	function;
			uint8	pull;
			bool	level;
		}			saved;		// what Unused gives back
	};

	uint32 _Next()
	{
		// xorshift32
		fRandom ^= fRandom << 13;
		fRandom ^= fRandom >> 17;
		fRandom ^= fRandom << 5;
		return fRandom;
	}

	bigtime_t _Interval()
	{
		// mostly slow changes, now and then a burst of short pulses
		uint32 dice = _Next() % 100;
		if (dice < 15)
			return 20000 + _Next() % 60000;
		return 150000 + _Next() % 1200000;
	}

	void _Record(int pin, bool level, uint8 flags, bigtime_t time)
	{
		fEvents.push_back({ time, (uint8)pin, level, flags });
		while (fEvents.size() > 8192)
			fEvents.pop_front();
	}

	//! Changes the simulated inputs whose time has come.
	void _Run(bigtime_t now)
	{
		for (int pin = 0; pin < kBcmPinCount; pin++) {
			Pin& state = fPins[pin];
			if (!state.claimed || state.function != kFunctionInput)
				continue;
			while (state.nextChange <= now) {
				state.level = !state.level;
				_Record(pin, state.level, 0, state.nextChange);
				state.nextChange += _Interval();
			}
		}
	}

	BLocker				fLock;
	sem_id				fWakeUp;
	uint32				fRandom;
	Pin					fPins[kBcmPinCount];
	std::deque<GpioEvent> fEvents;
};

}	// namespace


GpioHardware*
CreateSimulatedGpio()
{
	return new SimulatedGpio;
}

}	// namespace airpins
