/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "App.h"

#include <AboutWindow.h>
#include <Entry.h>
#include <Mime.h>
#include <String.h>

#include <stdio.h>
#include <string.h>

#include "MainWindow.h"
#include "Messages.h"
#include "Settings.h"
#include "Version.h"


namespace airpins {

App::App()
	:
	BApplication(kAppSignature),
	fWindow(NULL),
	fSimulate(false),
	fPendingRefs(B_REFS_RECEIVED)
{
}


void
App::ArgvReceived(int32 argc, char** argv)
{
	BMessage refs(B_REFS_RECEIVED);
	for (int32 i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--simulate") == 0 || strcmp(argv[i], "-s") == 0) {
			fSimulate = true;
			continue;
		}
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			printf("usage: AirPins [--simulate] [configuration.pigg]\n"
				"  --simulate  use simulated pins, not the board's\n");
			continue;
		}
		entry_ref ref;
		if (get_ref_for_path(argv[i], &ref) == B_OK && BEntry(&ref).Exists())
			refs.AddRef("refs", &ref);
		else
			fprintf(stderr, "AirPins: %s: no such file\n", argv[i]);
	}
	if (refs.HasRef("refs"))
		RefsReceived(&refs);
}


void
App::RefsReceived(BMessage* message)
{
	entry_ref ref;
	if (message->FindRef("refs", &ref) != B_OK)
		return;

	// one configuration at a time: the first one
	if (fWindow == NULL) {
		fPendingRefs.MakeEmpty();
		fPendingRefs.AddRef("refs", &ref);
		return;
	}
	BMessage open(B_REFS_RECEIVED);
	open.AddRef("refs", &ref);
	open.AddBool("ask", true);
	fWindow->PostMessage(&open);
}


void
App::ReadyToRun()
{
	_InstallFileType();

	BMessage settings;
	LoadSettings(settings);
	fWindow = new MainWindow(settings, fSimulate);
	fWindow->Show();

	if (fPendingRefs.HasRef("refs"))
		fWindow->PostMessage(&fPendingRefs);
}


void
App::AboutRequested()
{
	BAboutWindow* about = new BAboutWindow(kAppName, kAppSignature);
	about->SetVersion(kAppVersion);
	about->AddDescription("Configure the Raspberry Pi's GPIO pins as inputs"
		" or outputs, set outputs and watch the levels of inputs and outputs"
		" change over time.");
	about->AddCopyright(2026, "air/OS contributors");
	const char* pigg[] = {
		"AirPins is a native Haiku version of pigg, the Raspberry Pi GPIO"
		" GUI by Andrew Mackenzie and the pigg contributors"
		" (https://github.com/andrewdavidmackenzie/pigg, Apache License 2.0)."
		" Its layouts, pin colours, LED and waveform views and its"
		" configuration files follow pigg's; AirPins and pigg open each"
		" other's .pigg files.",
		NULL
	};
	about->AddText("Based on pigg", pigg);
	const char* icons[] = {
		"Font Awesome Free 6.7.2 by Fonticons, Inc. (CC BY 4.0).",
		NULL
	};
	about->AddText("Toolbar icons", icons);
	about->AddExtraInfo("AirPins is distributed under the terms of the MIT"
		" License.");
	// over AirPins' window: a desktop of two screens is one wide BScreen,
	// and its middle is the edge between them
	if (fWindow != NULL && about->Lock()) {
		BRect frame;
		if (fWindow->LockLooper()) {
			frame = fWindow->Frame();
			fWindow->UnlockLooper();
		}
		if (frame.IsValid())
			about->CenterIn(frame);
		about->Unlock();
	}
	about->Show();
}


void
App::_InstallFileType()
{
	BMimeType type(kConfigMimeType);
	if (!type.IsInstalled() && type.Install() != B_OK)
		return;

	// pigg's files are JSON: without a rule of their own they sniff as
	// plain text and open in a text editor
	static const char* const kRule = "0.70 [0:32] (\"\\\"pin_functions\\\"\""
		" | \"\\\"configured_pins\\\"\")";
	BString rule;
	if (type.GetSnifferRule(&rule) != B_OK || rule != kRule)
		type.SetSnifferRule(kRule);

	char preferred[B_MIME_TYPE_LENGTH];
	if (type.GetPreferredApp(preferred) == B_OK)
		return;
	type.SetShortDescription("pigg configuration");
	type.SetLongDescription("GPIO pin configuration of pigg and AirPins");
	BMessage extensions;
	extensions.AddString("extensions", "pigg");
	type.SetFileExtensions(&extensions);
	type.SetPreferredApp(kAppSignature);
}

}	// namespace airpins


int
main()
{
	airpins::App app;
	app.Run();
	return 0;
}
