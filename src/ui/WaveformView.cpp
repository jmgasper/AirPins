/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "WaveformView.h"

#include <String.h>

#include <math.h>

#include <algorithm>


namespace airpins {

namespace {

const rgb_color kBackground = { 24, 28, 32, 255 };
const rgb_color kGrid = { 48, 54, 60, 255 };
const rgb_color kInputTrace = { 60, 220, 110, 255 };
const rgb_color kOutputTrace = { 255, 176, 32, 255 };

}	// namespace


WaveformView::WaveformView(const PinEntry* entry)
	:
	BView(NULL, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fEntry(entry),
	fSpan(16000000),
	fNow(system_time()),
	fMouseInside(false)
{
}


void
WaveformView::AttachedToWindow()
{
	BView::AttachedToWindow();
	AdoptParentColors();
}


BSize
WaveformView::MinSize()
{
	// pigg's charts are 256 by 28 at its usual font size
	float scale = be_plain_font->Size() / 12.0f;
	return BSize(ceilf(256 * scale), ceilf(28 * scale));
}


BSize
WaveformView::MaxSize()
{
	return BSize(B_SIZE_UNLIMITED, MinSize().height);
}


BSize
WaveformView::PreferredSize()
{
	return MinSize();
}


void
WaveformView::SetSpan(bigtime_t span)
{
	fSpan = span;
	Invalidate();
}


void
WaveformView::Tick(bigtime_t now)
{
	fNow = now;
	if (fEntry->Configured())
		Invalidate();
}


void
WaveformView::Draw(BRect updateRect)
{
	if (!fEntry->Configured())
		return;

	BRect bounds = Bounds();
	SetHighColor(kBackground);
	FillRoundRect(bounds, 3, 3);

	float width = bounds.Width();
	float top = bounds.top + 4;
	float bottom = bounds.bottom - 4;
	bigtime_t start = fNow - fSpan;

	// a mark every second (every ten for long spans)
	bigtime_t step = fSpan > 20000000 ? 10000000 : 1000000;
	BeginLineArray(64);
	for (bigtime_t mark = fNow - fNow % step; mark > start && mark > 0;
			mark -= step) {
		float x = bounds.right - (fNow - mark) * width / fSpan;
		AddLine(BPoint(x, bounds.top + 1), BPoint(x, bounds.bottom - 1), kGrid);
	}
	EndLineArray();

	const PinHistory& history = fEntry->history;
	if (!history.HasLevel())
		return;

	rgb_color trace = fEntry->setting.mode == PinMode::Output
		? kOutputTrace : kInputTrace;
	SetHighColor(trace);

	auto xOf = [&](bigtime_t time) {
		return floorf(bounds.right - (fNow - time) * width / fSpan);
	};
	auto yOf = [&](bool level) { return level ? top : bottom; };

	const std::deque<LevelChange>& changes = history.Changes();
	// the first change inside the window, and the level before it
	auto first = std::upper_bound(changes.begin(), changes.end(), start,
		[](bigtime_t time, const LevelChange& change) {
			return time < change.time;
		});
	bool level;
	float x;
	if (first == changes.begin()) {
		// nothing known before the first change
		if (first == changes.end())
			return;
		level = first->level;
		x = xOf(first->time);
		++first;
	} else {
		level = (first - 1)->level;
		x = bounds.left;
	}

	BeginLineArray(512);
	int32 lines = 0;
	auto line = [&](BPoint from, BPoint to) {
		if (lines == 512) {
			EndLineArray();
			BeginLineArray(512);
			lines = 0;
		}
		AddLine(from, to, trace);
		lines++;
	};

	float lastVertical = -1;
	for (auto change = first; change != changes.end(); ++change) {
		float changeX = xOf(change->time);
		if (changeX > x)
			line(BPoint(x, yOf(level)), BPoint(changeX, yOf(level)));
		// many changes in one column: one vertical line for them all
		if (changeX != lastVertical) {
			line(BPoint(changeX, top), BPoint(changeX, bottom));
			lastVertical = changeX;
		}
		level = change->level;
		x = changeX;
	}
	if (bounds.right > x)
		line(BPoint(x, yOf(level)), BPoint(bounds.right, yOf(level)));
	EndLineArray();
}


void
WaveformView::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	if (!fEntry->Configured()) {
		SetToolTip((const char*)NULL);
		return;
	}
	if (transit == B_EXITED_VIEW || transit == B_OUTSIDE_VIEW)
		return;

	BRect bounds = Bounds();
	bigtime_t now = system_time();
	bigtime_t at = now - (bigtime_t)((bounds.right - where.x) * fSpan
		/ bounds.Width());
	int level = fEntry->history.LevelAt(at);
	size_t changes = fEntry->history.ChangesIn(now - fSpan, now);

	BString tip;
	tip.SetToFormat("%.1f s ago: %s\n%zu change%s in the last %d s",
		(now - at) / 1000000.0, level < 0 ? "not known" : level ? "high" : "low",
		changes, changes == 1 ? "" : "s", (int)(fSpan / 1000000));
	SetToolTip(tip);
}

}	// namespace airpins
