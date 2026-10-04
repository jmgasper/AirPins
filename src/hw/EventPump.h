/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <Messenger.h>
#include <OS.h>


namespace airpins {

class GpioHardware;

/*!	A thread that waits for the pins' level changes and sends them to a
	window in batches (kMsgPinEvents), at most about 30 a second.
*/
class EventPump {
public:
								EventPump(GpioHardware* hardware,
									const BMessenger& target);
								~EventPump();

			status_t			Start();
			void				Stop();

private:
	static	status_t			_Thread(void* data);
			void				_Run();

			GpioHardware*		fHardware;
			BMessenger			fTarget;
			thread_id			fThread;
			int32				fQuit;
};

}	// namespace airpins
