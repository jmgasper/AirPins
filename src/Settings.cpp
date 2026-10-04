/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "Settings.h"

#include <File.h>
#include <FindDirectory.h>
#include <Path.h>


namespace airpins {

static status_t
SettingsPath(BPath& path)
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path, true);
	if (status != B_OK)
		return status;
	return path.Append("AirPins settings");
}


status_t
LoadSettings(BMessage& settings)
{
	BPath path;
	status_t status = SettingsPath(path);
	if (status != B_OK)
		return status;
	BFile file(path.Path(), B_READ_ONLY);
	status = file.InitCheck();
	if (status != B_OK)
		return status;
	return settings.Unflatten(&file);
}


status_t
SaveSettings(const BMessage& settings)
{
	BPath path;
	status_t status = SettingsPath(path);
	if (status != B_OK)
		return status;
	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status = file.InitCheck();
	if (status != B_OK)
		return status;
	return settings.Flatten(&file);
}

}	// namespace airpins
