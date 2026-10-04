/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "DeviceWindow.h"

#include <Button.h>
#include <Clipboard.h>
#include <GridLayout.h>
#include <GridView.h>
#include <LayoutBuilder.h>
#include <StringView.h>

#include "BoardInfo.h"
#include "Messages.h"
#include "Version.h"


namespace airpins {

static const uint32 kMsgCopy = 'copy';


DeviceWindow::DeviceWindow(BWindow* parent, const HardwareDetails& details)
	:
	BWindow(BRect(0, 0, 100, 100), "Device details", B_FLOATING_WINDOW_LOOK,
		B_FLOATING_SUBSET_WINDOW_FEEL, B_NOT_RESIZABLE | B_NOT_ZOOMABLE
			| B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE)
{
	AddToSubset(parent);

	BoardInfo board = DecodeRevision(details.revision);
	BString serial;
	if (details.serial != 0)
		serial.SetToFormat("%016" B_PRIx64, details.serial);
	else
		serial = "not known";
	BString events = details.simulated ? "Simulated"
		: details.edgeInterrupts
			? "Edge interrupts, timestamped by the driver"
			: "Sampled every millisecond";
	BString driver;
	if (details.simulated)
		driver = "None (no GPIO hardware found)";
	else
		driver.SetToFormat("/dev/misc/rpi_gpio, interface version %" B_PRIu32,
			details.apiVersion);
	BString application;
	application.SetToFormat("%s %s", kAppName, kAppVersion);

	const struct { const char* label; BString value; } kRows[] = {
		{ "Hardware:", details.name },
		{ "Model:", board.known ? BString(board.model.c_str())
			: BString("not known") },
		{ "Revision code:", details.revision != 0
			? BString(board.revisionCode.c_str()) : BString("not known") },
		{ "Processor:", board.known ? BString(board.processor.c_str()) : "" },
		{ "Memory:", board.known ? BString(board.memory.c_str()) : "" },
		{ "Manufacturer:", board.known ? BString(board.manufacturer.c_str())
			: "" },
		{ "Serial number:", serial },
		{ "Driver:", driver },
		{ "Input changes:", events },
		{ "Configurable pins:", "26 (GPIO 2 to 27 on the 40-pin header)" },
		{ "Application:", application },
	};

	// a grid view: a layout takes items only once it belongs to a view
	BGridView* gridView = new BGridView(B_USE_DEFAULT_SPACING,
		B_USE_SMALL_SPACING);
	BGridLayout* grid = gridView->GridLayout();
	int row = 0;
	for (const auto& entry : kRows) {
		if (entry.value.IsEmpty())
			continue;
		BStringView* label = new BStringView(NULL, entry.label);
		label->SetAlignment(B_ALIGN_RIGHT);
		label->SetHighUIColor(B_PANEL_TEXT_COLOR, B_DISABLED_LABEL_TINT);
		grid->AddView(label, 0, row);
		grid->AddView(new BStringView(NULL, entry.value), 1, row);
		fText << entry.label << " " << entry.value << "\n";
		row++;
	}

	BButton* close = new BButton("Close", new BMessage(B_QUIT_REQUESTED));
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(gridView)
		.AddGroup(B_HORIZONTAL)
			.Add(new BButton("Copy", new BMessage(kMsgCopy)))
			.AddGlue()
			.Add(close)
		.End();
	SetDefaultButton(close);
	CenterIn(parent->Frame());
}


void
DeviceWindow::MessageReceived(BMessage* message)
{
	if (message->what != kMsgCopy) {
		BWindow::MessageReceived(message);
		return;
	}
	if (be_clipboard->Lock()) {
		be_clipboard->Clear();
		BMessage* clip = be_clipboard->Data();
		clip->AddData("text/plain", B_MIME_TYPE, fText.String(),
			fText.Length());
		be_clipboard->Commit();
		be_clipboard->Unlock();
	}
}

}	// namespace airpins
