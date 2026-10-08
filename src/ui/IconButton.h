/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 *
 * From airShot: a native button that draws its icon with full alpha.
 */
#pragma once

#include <Button.h>

#include "Icons.h"


namespace airpins {

class IconButton : public BButton {
public:
								IconButton(const char* name,
									const char* label, IconId icon,
									BMessage* message);

			void				SetIconId(IconId icon);

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);

private:
			void				_UpdateIcon();

			IconId				fIcon;
			bool				fInside;
			IconId				fRenderedIcon;
			float				fRenderedSize;
			rgb_color			fRenderedColor;
};

}	// namespace airpins
