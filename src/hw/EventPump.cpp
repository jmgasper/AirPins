/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "EventPump.h"

#include <Message.h>

#include <vector>

#include "Gpio.h"
#include "Messages.h"


namespace airpins {

static const bigtime_t kIdleWait = 100000;
static const bigtime_t kBatchTime = 30000;
static const size_t kMaxBatch = 16384;


EventPump::EventPump(GpioHardware* hardware, const BMessenger& target)
	:
	fHardware(hardware),
	fTarget(target),
	fThread(-1),
	fQuit(0)
{
}


EventPump::~EventPump()
{
	Stop();
}


status_t
EventPump::Start()
{
	fQuit = 0;
	fThread = spawn_thread(&_Thread, "gpio events", B_DISPLAY_PRIORITY, this);
	if (fThread < 0)
		return fThread;
	return resume_thread(fThread);
}


void
EventPump::Stop()
{
	if (fThread < 0)
		return;
	atomic_set(&fQuit, 1);
	status_t result;
	wait_for_thread(fThread, &result);
	fThread = -1;
}


status_t
EventPump::_Thread(void* data)
{
	((EventPump*)data)->_Run();
	return B_OK;
}


void
EventPump::_Run()
{
	std::vector<GpioEvent> batch;
	GpioEvent events[64];
	uint32 lost = 0;

	while (atomic_get(&fQuit) == 0) {
		int count = 0;
		uint32 lostNow = 0;
		status_t status = fHardware->WaitEvents(events, 64,
			batch.empty() ? kIdleWait : 5000, count, lostNow);
		if (status != B_OK && status != B_INTERRUPTED) {
			snooze(kIdleWait);
			continue;
		}
		lost += lostNow;
		batch.insert(batch.end(), events, events + count);

		// collect for a while: one message for many events
		if (batch.empty() || (system_time() - batch.front().time < kBatchTime
				&& batch.size() < kMaxBatch && count > 0)) {
			continue;
		}

		BMessage message(kMsgPinEvents);
		message.AddData("events", B_RAW_TYPE, batch.data(),
			batch.size() * sizeof(GpioEvent));
		message.AddInt32("lost", lost);
		if (fTarget.SendMessage(&message, (BHandler*)NULL, 1000000) == B_OK)
			lost = 0;
		else
			lost += batch.size();
		batch.clear();
	}
}

}	// namespace airpins
