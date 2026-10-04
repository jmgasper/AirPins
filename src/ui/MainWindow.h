/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <Entry.h>
#include <Window.h>

#include "Config.h"
#include "Messages.h"
#include "PinEntry.h"

class BFilePanel;
class BMenuItem;
class BMessageRunner;
class BStringView;


namespace airpins {

class EventPump;
class GpioHardware;
class IconButton;
class PinsView;

class MainWindow : public BWindow {
public:
								MainWindow(const BMessage& settings,
									bool simulate);
	virtual						~MainWindow();

	virtual	bool				QuitRequested();
	virtual	void				MessageReceived(BMessage* message);
	virtual	void				MenusBeginning();

			void				OpenFile(const entry_ref& ref);
			void				StoreSettings(BMessage& settings) const;

private:
			void				_BuildMenu(BMenuBar* menuBar);
			void				_UseHardware(bool simulated);
			void				_ReleaseHardware();

			bool				_SetPin(int bcm, const PinSetting& setting,
									bool ask);
			void				_ApplyConfiguration(const Configuration& config,
									bool ask);
			Configuration		_CurrentConfiguration() const;
			bool				_Confirm(const BString& text,
									const char* proceed);
			bool				_ConfirmTakeOver(const std::vector<int>& pins);

			void				_HandleEvents(BMessage* message);
			void				_ToggleOutput(int bcm);
			void				_HoldOutput(int bcm, bool pressed);
			void				_RefreshState();
			void				_Tick();

			void				_SetLayoutMode(LayoutMode mode);
			void				_SetSpan(int seconds);

			void				_Open();
			bool				_Save(bool askName);
			status_t			_WriteFile(const entry_ref& directory,
									const char* name);
			bool				_AskToSave(const char* action);
			void				_ResetPins();

			void				_UpdateTitle();
			void				_UpdateStatus();
			void				_ShowMessage(const BString& message);
			void				_ShowDeviceDetails();
			void				_FitToContent();

			PinEntry			fPins[kBcmPinCount];
			GpioHardware*		fHardware;
			EventPump*			fPump;
			BString				fHardwareName;
			bool				fSimulated;

			Configuration		fSavedConfiguration;
			entry_ref			fFileRef;
			bool				fHasFile;
			bool				fQuitAfterSave;
			bool				fApplying;

			PinsView*			fPinsView;
			BStringView*		fStatusView;
			BStringView*		fMessageView;
			IconButton*			fLayoutButtons[3];
			BMenuItem*			fLayoutItems[3];
			BMenuItem*			fSpanItems[4];
			BMenuItem*			fSimulatedItem;
			BMenuItem*			fSaveItem;

			BFilePanel*			fOpenPanel;
			BFilePanel*			fSavePanel;
			BMessageRunner*		fTicker;
			BMessageRunner*		fRefresher;
			BMessageRunner*		fMessageClearer;
			int					fSpanSeconds;
};

}	// namespace airpins
