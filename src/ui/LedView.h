/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <View.h>

#include "PinEntry.h"


namespace airpins {

/*!	A pin's level as an LED, as in pigg: green when high, red when low,
	grey before the level is known. An output's LED is also pigg's
	"clicker": holding the mouse button on it inverts the output.
*/
class LedView : public BView {
public:
								LedView(const PinEntry* entry,
									BHandler* target);

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();

			void				Update();

private:
			void				_SendHold(bool pressed);

			const PinEntry*		fEntry;
			BHandler*			fTarget;
			bool				fPressed;
			int					fShownLevel;	// -2: nothing shown
};

}	// namespace airpins
