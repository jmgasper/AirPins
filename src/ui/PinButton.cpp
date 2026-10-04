/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "PinButton.h"

#include <ControlLook.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>
#include <Window.h>

#include <math.h>
#include <stdio.h>

#include "Messages.h"


namespace airpins {

namespace {

BMessage*
FunctionMessage(int bcm, PinMode mode, Pull pull = Pull::None)
{
	BMessage* message = new BMessage(kMsgSetPinFunction);
	message->AddInt32("pin", bcm);
	message->AddInt32("mode", (int32)mode);
	message->AddInt32("pull", (int32)pull);
	return message;
}

}	// namespace


PinButton::PinButton(const HeaderPin& pin, const PinEntry* entry,
	BHandler* target, bool small)
	:
	BView(NULL, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fPin(pin),
	fEntry(entry),
	fTarget(target),
	fSmall(small),
	fInside(false)
{
}


void
PinButton::AttachedToWindow()
{
	BView::AttachedToWindow();
	AdoptParentColors();
	Update();
}


float
PinButton::_Diameter() const
{
	float size = ceilf(be_plain_font->Size() * (fSmall ? 1.75f : 2.0f));
	return size;
}


BSize
PinButton::MinSize()
{
	return BSize(_Diameter(), _Diameter());
}


BSize
PinButton::MaxSize()
{
	return MinSize();
}


BSize
PinButton::PreferredSize()
{
	return MinSize();
}


void
PinButton::Draw(BRect updateRect)
{
	PinColor color = ColorFor(fPin);
	rgb_color fill = { color.red, color.green, color.blue, 255 };
	rgb_color text = { color.textRed, color.textGreen, color.textBlue, 255 };
	bool configurable = fPin.kind == PinKind::Gpio && fEntry != NULL;
	bool busy = configurable && fEntry->hardware.claimedElsewhere;

	if (fInside && configurable && !busy)
		fill = tint_color(fill, B_LIGHTEN_1_TINT);
	if (busy)
		fill = tint_color(fill, B_DISABLED_MARK_TINT);

	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	BRect disc = Bounds().InsetByCopy(1, 1);
	SetHighColor(fill);
	FillEllipse(disc);

	// a configured pin wears a ring in the panel's text colour
	rgb_color panelText = ui_color(B_PANEL_TEXT_COLOR);
	if (configurable && fEntry->Configured()) {
		SetHighColor(panelText);
		SetPenSize(2);
		StrokeEllipse(disc.InsetByCopy(0.5, 0.5));
		SetPenSize(1);
	} else {
		SetHighColor(tint_color(fill, B_DARKEN_2_TINT));
		StrokeEllipse(disc);
	}

	BFont font(be_bold_font);
	font.SetSize(floorf(be_plain_font->Size() * (fSmall ? 0.8f : 0.9f)));
	SetFont(&font);
	char number[8];
	snprintf(number, sizeof(number), "%u", fPin.board);
	font_height height;
	font.GetHeight(&height);
	float width = font.StringWidth(number);
	BPoint where(floorf(disc.left + (disc.Width() - width) / 2 + 0.5),
		floorf(disc.top + (disc.Height() + height.ascent - height.descent) / 2
			+ 0.5));
	SetHighColor(text);
	SetDrawingMode(B_OP_OVER);
	DrawString(number, where);
}


void
PinButton::MouseDown(BPoint where)
{
	if (fPin.kind != PinKind::Gpio || fEntry == NULL || fTarget == NULL)
		return;

	BPopUpMenu* menu = new BPopUpMenu("pin", false, false);
	menu->SetAsyncAutoDestruct(true);

	BString title;
	title.SetToFormat("Pin %u: %s", fPin.board, fPin.name);
	BMenuItem* titleItem = new BMenuItem(title, NULL);
	titleItem->SetEnabled(false);
	menu->AddItem(titleItem);
	menu->AddSeparatorItem();

	bool busy = fEntry->hardware.claimedElsewhere;
	const PinSetting& setting = fEntry->setting;
	BMenu* input = new BMenu("Input");
	const Pull pulls[] = { Pull::Up, Pull::Down, Pull::None };
	for (Pull pull : pulls) {
		BMenuItem* item = new BMenuItem(PullName(pull),
			FunctionMessage(fPin.bcm, PinMode::Input, pull));
		item->SetMarked(setting.mode == PinMode::Input && setting.pull == pull);
		input->AddItem(item);
	}
	input->SetTargetForItems(fTarget);
	BMenuItem* inputItem = new BMenuItem(input);
	inputItem->SetMarked(setting.mode == PinMode::Input);
	inputItem->SetEnabled(!busy);
	menu->AddItem(inputItem);

	BMenuItem* output = new BMenuItem("Output",
		FunctionMessage(fPin.bcm, PinMode::Output));
	output->SetMarked(setting.mode == PinMode::Output);
	output->SetEnabled(!busy);
	menu->AddItem(output);
	menu->AddSeparatorItem();
	BMenuItem* unused = new BMenuItem("Unused",
		FunctionMessage(fPin.bcm, PinMode::Unused));
	unused->SetMarked(setting.mode == PinMode::Unused);
	unused->SetEnabled(!busy);
	menu->AddItem(unused);
	if (busy) {
		BMenuItem* note = new BMenuItem("In use by another program", NULL);
		note->SetEnabled(false);
		menu->AddItem(note);
	}
	menu->SetTargetForItems(fTarget);

	menu->Go(ConvertToScreen(where), true, false, true);
}


void
PinButton::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	bool inside = transit == B_ENTERED_VIEW || transit == B_INSIDE_VIEW;
	if (inside != fInside) {
		fInside = inside;
		Invalidate();
	}
}


void
PinButton::Update()
{
	SetToolTip(Description(fPin, fEntry));
	Invalidate();
}


BString
PinButton::Description(const HeaderPin& pin, const PinEntry* entry)
{
	BString text;
	text.SetToFormat("Pin %u: %s", pin.board, pin.name);
	switch (pin.kind) {
		case PinKind::Power3V3:
			text << "\n3.3 V supply";
			return text;
		case PinKind::Power5V:
			text << "\n5 V supply";
			return text;
		case PinKind::Ground:
			return text;
		case PinKind::IdEeprom:
			text.SetToFormat("Pin %u: GPIO%d (%s)\nThe HAT ID EEPROM's I2C bus,"
				" reserved", pin.board, pin.bcm, pin.name);
			return text;
		case PinKind::Gpio:
			break;
	}

	if (entry != NULL) {
		const PinHardwareState& hardware = entry->hardware;
		int alt = AltIndex(hardware.function);
		if (alt >= 0) {
			const char* signal = AltSignal(pin.bcm, alt);
			const char* group = SignalGroup(signal);
			text << "\nNow: " << FunctionSelectName(hardware.function);
			if (signal != NULL) {
				text << " " << signal;
				if (group != NULL)
					text << " (" << group << ")";
			}
		} else if (!entry->Configured()) {
			text << "\nNow: "
				<< (hardware.function == kFunctionOutput ? "output" : "input")
				<< ", " << (hardware.pull == 1 ? "pulled up"
					: hardware.pull == 2 ? "pulled down" : "no pull")
				<< ", " << (hardware.level ? "high" : "low");
		}
		if (hardware.claimedElsewhere)
			text << "\nIn use by another program";
	}

	text << "\nAlternate functions:";
	for (int alt = 0; alt < 6; alt++) {
		const char* signal = AltSignal(pin.bcm, alt);
		if (signal != NULL)
			text << "\n  ALT" << alt << "  " << signal;
	}
	if (entry != NULL && !entry->hardware.claimedElsewhere)
		text << "\nClick to set the pin's function.";
	return text;
}

}	// namespace airpins
