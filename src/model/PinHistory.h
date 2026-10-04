/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <deque>


namespace airpins {

struct LevelChange {
	int64_t	time;		// microseconds, the system_time() clock
	bool	level;
};

//! The recent levels of one pin, for its LED and its waveform.
class PinHistory {
public:
						PinHistory(size_t maxChanges = 20000);

	//! A level at a time; a level equal to the current one is no change.
	//! Times never go back: an earlier one is taken as the last one.
	void				Add(int64_t time, bool level);
	void				Clear();

	bool				HasLevel() const { return !fChanges.empty(); }
	bool				Level() const;
	int64_t				LastChange() const;

	//! The level at \a time: 0, 1, or -1 when nothing was known yet.
	int					LevelAt(int64_t time) const;
	//! Level changes after \a start up to \a end.
	size_t				ChangesIn(int64_t start, int64_t end) const;
	//! Forgets changes before \a time, keeping the level at that time.
	void				Trim(int64_t time);

	const std::deque<LevelChange>& Changes() const { return fChanges; }

private:
	std::deque<LevelChange>	fChanges;
	size_t					fMaxChanges;
};

}	// namespace airpins
