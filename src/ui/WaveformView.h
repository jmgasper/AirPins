/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <View.h>

#include "PinEntry.h"


namespace airpins {

/*!	The recent history of a pin's level, newest on the right, like pigg's
	waveform charts. Changes closer together than a pixel show as a solid
	band, so nothing short is lost from view.
*/
class WaveformView : public BView {
public:
								WaveformView(const PinEntry* entry);

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);
	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();

			void				SetSpan(bigtime_t span);
			void				Tick(bigtime_t now);

private:
			const PinEntry*		fEntry;
			bigtime_t			fSpan;
			bigtime_t			fNow;
			BPoint				fMouse;
			bool				fMouseInside;
};

}	// namespace airpins
