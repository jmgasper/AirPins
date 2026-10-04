/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <View.h>

#include <vector>

#include "Messages.h"
#include "PinEntry.h"
#include "PinTable.h"

class BCardLayout;
class BGridLayout;
class BGroupLayout;
class BMenuField;
class BStringView;


namespace airpins {

class IconButton;
class LedView;
class PinButton;
class WaveformView;

/*!	The pins in one of pigg's three layouts: as on the board's header (two
	columns, mirrored), in GPIO number order, or only the configured ones
	with the rest in a dock. It is the target of a scroll view; the rows
	live in a child view that is laid out at its preferred size.
*/
class PinsView : public BView {
public:
								PinsView(PinEntry* pins, BHandler* target);

	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float width, float height);
	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();

			void				SetLayoutMode(LayoutMode mode);
			LayoutMode			Mode() const { return fMode; }
			void				Rebuild();

			void				UpdatePin(int bcm);
			void				UpdateAll();
			void				UpdateLevel(int bcm);
			void				Tick(bigtime_t now);
			void				SetSpan(bigtime_t span);

			BSize				ContentSize();

private:
	struct Widgets {
		int				bcm;
		PinButton*		button;
		BStringView*	name;
		BStringView*	hint;
		BMenuField*		function;
		BCardLayout*	options;
		BMenuField*		pull;
		IconButton*		toggle;
		LedView*		led;
		WaveformView*	wave;
	};

			void				_AddPin(BGridLayout* grid, int row,
									int firstColumn, const HeaderPin& pin,
									bool mirrored);
			void				_Update(Widgets& widgets);
			void				_UpdateContentSize();

			PinEntry*			fPins;
			BHandler*			fTarget;
			LayoutMode			fMode;
			bigtime_t			fSpan;
			BView*				fContent;
			std::vector<Widgets> fWidgets;
			std::vector<PinButton*> fDock;
};

}	// namespace airpins
