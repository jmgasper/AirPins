/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 *
 * From airShot: a native button that draws its icon with full alpha.
 */

#include "IconButton.h"

#include <ControlLook.h>
#include <String.h>

#include <math.h>


namespace airpins {

IconButton::IconButton(const char* name, const char* label, IconId icon,
	BMessage* message)
	:
	BButton(name, label, message),
	fIcon(icon),
	fInside(false)
{
	_UpdateIcon();
}


void
IconButton::SetIconId(IconId icon)
{
	if (icon == fIcon)
		return;
	fIcon = icon;
	_UpdateIcon();
	Invalidate();
}


void
IconButton::AttachedToWindow()
{
	BButton::AttachedToWindow();
	_UpdateIcon();
}


void
IconButton::Draw(BRect updateRect)
{
	BRect rect(Bounds());
	rgb_color base = ui_color(B_CONTROL_BACKGROUND_COLOR);
	rgb_color text = ui_color(B_CONTROL_TEXT_COLOR);
	uint32 flags = be_control_look->Flags(this);
	if (IsDefault())
		flags |= BControlLook::B_DEFAULT_BUTTON;
	if (IsFlat() && !IsTracking())
		flags |= BControlLook::B_FLAT;
	if (fInside)
		flags |= BControlLook::B_HOVER;

	PushState();
	be_control_look->DrawButtonFrame(this, rect, updateRect, base,
		ViewColor(), flags);
	be_control_look->DrawButtonBackground(this, rect, updateRect, base, flags);

	const BBitmap* icon = IconBitmap((Value() == B_CONTROL_OFF
		? B_INACTIVE_ICON_BITMAP : B_ACTIVE_ICON_BITMAP)
		| (IsEnabled() ? 0 : B_DISABLED_ICON_BITMAP));
	BString label(Label() != NULL ? Label() : "");
	float iconWidth = icon != NULL ? icon->Bounds().Width() + 1 : 0;
	float spacing = icon != NULL && !label.IsEmpty()
		? be_control_look->DefaultLabelSpacing() : 0;
	BFont font;
	GetFont(&font);
	font.TruncateString(&label, B_TRUNCATE_END,
		fmaxf(0, rect.Width() + 1 - iconWidth - spacing));
	float labelWidth = ceilf(font.StringWidth(label.String()));
	float left = floorf(rect.left
		+ (rect.Width() + 1 - iconWidth - spacing - labelWidth) / 2);
	if (icon != NULL) {
		BPoint position(left, floorf(rect.top
			+ (rect.Height() - icon->Bounds().Height()) / 2));
		// BControlLook::DrawLabel uses B_OP_OVER, which loses the icon's
		// antialiasing and the disabled bitmap's transparency.
		SetDrawingMode(B_OP_ALPHA);
		SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		DrawBitmap(icon, position);
	}
	if (!label.IsEmpty()) {
		BRect labelRect(left + iconWidth + spacing, rect.top,
			left + iconWidth + spacing + labelWidth - 1, rect.bottom);
		SetDrawingMode(B_OP_COPY);
		be_control_look->DrawLabel(this, label.String(), labelRect,
			updateRect, base, flags,
			BAlignment(B_ALIGN_CENTER, B_ALIGN_MIDDLE), &text);
	}
	PopState();
}


void
IconButton::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	bool inside = transit == B_ENTERED_VIEW || transit == B_INSIDE_VIEW;
	if (inside != fInside) {
		fInside = inside;
		Invalidate();
	}
	BButton::MouseMoved(where, transit, dragMessage);
}


void
IconButton::_UpdateIcon()
{
	BBitmap* bitmap = CreateIcon(fIcon, IconSize(),
		ui_color(B_CONTROL_TEXT_COLOR));
	if (bitmap != NULL) {
		SetIcon(bitmap);
		delete bitmap;
	}
}

}	// namespace airpins
