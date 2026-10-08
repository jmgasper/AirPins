/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "Icons.h"

#include <Font.h>
#include <IconUtils.h>

#include <math.h>
#include <string.h>


namespace airpins {

BBitmap*
CreateIcon(IconId icon, float size, rgb_color color)
{
	if (icon < 0 || icon >= kIconCount || !(size >= 1 && size <= 1024))
		return NULL;
	int32 pixels = (int32)ceilf(size);
	// Vector rasterization and tinting only access the pixels. SetIcon()
	// creates the server-backed copies used to draw the button afterwards.
	BBitmap* bitmap = new BBitmap(BRect(0, 0, pixels - 1, pixels - 1),
		B_BITMAP_NO_SERVER_LINK, B_RGBA32);
	if (bitmap->InitCheck() != B_OK) {
		delete bitmap;
		return NULL;
	}
	memset(bitmap->Bits(), 0, bitmap->BitsLength());
	if (BIconUtils::GetVectorIcon(kIconData[icon].data, kIconData[icon].size,
			bitmap) != B_OK) {
		delete bitmap;
		return NULL;
	}

	// The artwork is a black mask: keep its coverage, take the colour.
	for (int32 y = 0; y < pixels; y++) {
		uint8* pixel = (uint8*)bitmap->Bits() + y * bitmap->BytesPerRow();
		for (int32 x = 0; x < pixels; x++, pixel += 4) {
			pixel[0] = color.blue;
			pixel[1] = color.green;
			pixel[2] = color.red;
		}
	}
	return bitmap;
}


float
IconSize()
{
	return floorf(be_plain_font->Size() * 16 / 12);
}

}	// namespace airpins
