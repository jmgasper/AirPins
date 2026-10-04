/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <Application.h>


namespace airpins {

class MainWindow;

class App : public BApplication {
public:
								App();

	virtual	void				ArgvReceived(int32 argc, char** argv);
	virtual	void				RefsReceived(BMessage* message);
	virtual	void				ReadyToRun();
	virtual	void				AboutRequested();

private:
			void				_InstallFileType();

			MainWindow*			fWindow;
			bool				fSimulate;
			BMessage			fPendingRefs;
};

}	// namespace airpins
