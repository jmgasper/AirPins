/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <Bitmap.h>
#include <InterfaceDefs.h>

#include "IconData.h"


namespace airpins {

//! A Font Awesome icon \a size pixels square in \a color, or NULL.
BBitmap* CreateIcon(IconId icon, float size, rgb_color color);
//! The icon size that goes with the plain font (16 at 12 points).
float IconSize();

}	// namespace airpins
