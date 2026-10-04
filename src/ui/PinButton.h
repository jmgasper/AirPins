/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <View.h>

#include "PinEntry.h"
#include "PinTable.h"


namespace airpins {

/*!	A pin of the header as pigg draws it: a disc in the pin's colour with
	its board number. Clicking a GPIO's disc opens its function menu.
*/
class PinButton : public BView {
public:
								PinButton(const HeaderPin& pin,
									const PinEntry* entry, BHandler* target,
									bool small = false);

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);
	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();

			void				Update();

	static	BString				Description(const HeaderPin& pin,
									const PinEntry* entry);

private:
			float				_Diameter() const;

			const HeaderPin&	fPin;
			const PinEntry*		fEntry;
			BHandler*			fTarget;
			bool				fSmall;
			bool				fInside;
};

}	// namespace airpins
