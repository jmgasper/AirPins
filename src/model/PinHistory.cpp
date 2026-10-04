/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "PinHistory.h"

#include <algorithm>


namespace airpins {

namespace {

bool
EarlierThan(const LevelChange& change, int64_t time)
{
	return change.time < time;
}

}	// namespace


PinHistory::PinHistory(size_t maxChanges)
	:
	fMaxChanges(maxChanges < 2 ? 2 : maxChanges)
{
}


void
PinHistory::Add(int64_t time, bool level)
{
	if (!fChanges.empty()) {
		if (fChanges.back().level == level)
			return;
		if (time < fChanges.back().time)
			time = fChanges.back().time;
	}
	fChanges.push_back({ time, level });
	while (fChanges.size() > fMaxChanges)
		fChanges.pop_front();
}


void
PinHistory::Clear()
{
	fChanges.clear();
}


bool
PinHistory::Level() const
{
	return !fChanges.empty() && fChanges.back().level;
}


int64_t
PinHistory::LastChange() const
{
	return fChanges.empty() ? 0 : fChanges.back().time;
}


int
PinHistory::LevelAt(int64_t time) const
{
	// the last change at or before the time
	auto after = std::upper_bound(fChanges.begin(), fChanges.end(), time,
		[](int64_t value, const LevelChange& change) {
			return value < change.time;
		});
	if (after == fChanges.begin())
		return -1;
	return (after - 1)->level ? 1 : 0;
}


size_t
PinHistory::ChangesIn(int64_t start, int64_t end) const
{
	auto first = std::upper_bound(fChanges.begin(), fChanges.end(), start,
		[](int64_t value, const LevelChange& change) {
			return value < change.time;
		});
	auto last = std::upper_bound(fChanges.begin(), fChanges.end(), end,
		[](int64_t value, const LevelChange& change) {
			return value < change.time;
		});
	return last > first ? last - first : 0;
}


void
PinHistory::Trim(int64_t time)
{
	// keep the last change before the time: it is the level at the start
	auto first = std::lower_bound(fChanges.begin(), fChanges.end(), time,
		EarlierThan);
	if (first == fChanges.begin())
		return;
	fChanges.erase(fChanges.begin(), first - 1);
}

}	// namespace airpins
