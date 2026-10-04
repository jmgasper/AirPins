/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "LedView.h"

#include <Messenger.h>
#include <Window.h>

#include <math.h>

#include "Messages.h"


namespace airpins {

LedView::LedView(const PinEntry* entry, BHandler* target)
	:
	BView(NULL, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fEntry(entry),
	fTarget(target),
	fPressed(false),
	fShownLevel(-2)
{
}


void
LedView::AttachedToWindow()
{
	BView::AttachedToWindow();
	AdoptParentColors();
	Update();
}


BSize
LedView::MinSize()
{
	float size = ceilf(be_plain_font->Size() * 1.5f);
	return BSize(size, size);
}


BSize
LedView::MaxSize()
{
	return MinSize();
}


BSize
LedView::PreferredSize()
{
	return MinSize();
}


void
LedView::Draw(BRect updateRect)
{
	if (fShownLevel == -2)
		return;

	rgb_color color;
	if (fShownLevel == 1)
		color = (rgb_color){ 40, 200, 70, 255 };
	else if (fShownLevel == 0)
		color = (rgb_color){ 200, 40, 40, 255 };
	else
		color = (rgb_color){ 110, 110, 110, 255 };
	if (fPressed)
		color = tint_color(color, B_DARKEN_1_TINT);

	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	BRect disc = Bounds().InsetByCopy(2, 2);
	SetHighColor(color);
	FillEllipse(disc);
	SetHighColor(tint_color(color, B_DARKEN_3_TINT));
	StrokeEllipse(disc);

	// the lens' reflection
	BRect shine(disc.left + disc.Width() * 0.22f, disc.top + disc.Height() * 0.14f,
		disc.left + disc.Width() * 0.58f, disc.top + disc.Height() * 0.42f);
	SetHighColor(255, 255, 255, fShownLevel == 1 ? 150 : 90);
	FillEllipse(shine);
}


void
LedView::MouseDown(BPoint where)
{
	if (fEntry->setting.mode != PinMode::Output)
		return;
	SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
	fPressed = true;
	_SendHold(true);
	Invalidate();
}


void
LedView::MouseUp(BPoint where)
{
	if (!fPressed)
		return;
	fPressed = false;
	_SendHold(false);
	Invalidate();
}


void
LedView::Update()
{
	int level = -2;
	if (fEntry->Configured()) {
		level = fEntry->history.HasLevel()
			? (fEntry->history.Level() ? 1 : 0) : -1;
	}
	if (level != fShownLevel) {
		fShownLevel = level;
		Invalidate();
	}

	const char* tip = NULL;
	if (fEntry->setting.mode == PinMode::Output)
		tip = "Hold down to invert the level";
	else if (fEntry->setting.mode == PinMode::Input)
		tip = "The input's level";
	SetToolTip(tip);
}


void
LedView::_SendHold(bool pressed)
{
	BMessage message(kMsgHoldOutput);
	message.AddInt32("pin", fEntry->bcm);
	message.AddBool("pressed", pressed);
	BMessenger(fTarget).SendMessage(&message);
}

}	// namespace airpins
