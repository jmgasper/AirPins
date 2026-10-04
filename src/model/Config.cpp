/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "Config.h"

#include <stdio.h>
#include <stdlib.h>

#include "Json.h"
#include "PinTable.h"


namespace airpins {

namespace {

bool
Fail(std::string* error, const std::string& what)
{
	if (error != NULL)
		*error = what;
	return false;
}


/*!	pigg (serde) writes Input(Option<InputPull>) as {"Input":"PullUp"},
	{"Input":"PullDown"}, {"Input":"None"} or {"Input":null}, and
	Output(Option<bool>) as {"Output":true}, {"Output":false} or
	{"Output":null}.
*/
bool
ParseFunction(const JsonValue& value, PinSetting& setting)
{
	if (value.IsString() && value.String() == "Input") {
		setting.mode = PinMode::Input;
		return true;
	}
	if (value.IsString() && value.String() == "Output") {
		setting.mode = PinMode::Output;
		return true;
	}
	if (!value.IsObject() || value.Members().size() != 1)
		return false;

	const std::string& kind = value.Members()[0].first;
	const JsonValue& argument = value.Members()[0].second;
	if (kind == "Input") {
		setting.mode = PinMode::Input;
		if (argument.IsNull())
			setting.pull = Pull::None;
		else if (argument.IsString() && argument.String() == "PullUp")
			setting.pull = Pull::Up;
		else if (argument.IsString() && argument.String() == "PullDown")
			setting.pull = Pull::Down;
		else if (argument.IsString() && argument.String() == "None")
			setting.pull = Pull::None;
		else
			return false;
		return true;
	}
	if (kind == "Output") {
		setting.mode = PinMode::Output;
		if (argument.IsNull())
			setting.level = -1;
		else if (argument.IsBool())
			setting.level = argument.Bool() ? 1 : 0;
		else
			return false;
		return true;
	}
	return false;
}

}	// namespace


bool
ParseConfiguration(const std::string& text, Configuration& config,
	std::string* error, std::vector<int>* skipped)
{
	JsonValue root;
	std::string parseError;
	if (!JsonValue::Parse(text, root, &parseError))
		return Fail(error, "Not a configuration file: " + parseError);
	if (!root.IsObject())
		return Fail(error, "Not a configuration file: no JSON object");

	// "configured_pins" is what early pigg versions wrote
	const JsonValue* functions = root.Find("pin_functions");
	if (functions == NULL)
		functions = root.Find("configured_pins");
	if (functions == NULL || !functions->IsObject())
		return Fail(error, "Not a configuration file: no pin functions");

	Configuration result;
	for (const auto& member : functions->Members()) {
		char* end;
		long bcm = strtol(member.first.c_str(), &end, 10);
		if (member.first.empty() || *end != '\0')
			return Fail(error, "Bad pin number \"" + member.first + "\"");

		PinSetting setting;
		if (!ParseFunction(member.second, setting)) {
			return Fail(error, "Pin " + member.first
				+ " has a function AirPins does not know");
		}
		if (!IsConfigurable((int)bcm)) {
			if (skipped != NULL)
				skipped->push_back((int)bcm);
			continue;
		}
		result.pins[(int)bcm] = setting;
	}

	config = result;
	return true;
}


std::string
FormatConfiguration(const Configuration& config)
{
	std::string text = "{\"pin_functions\":{";
	bool first = true;
	for (const auto& entry : config.pins) {
		const PinSetting& setting = entry.second;
		const char* function;
		switch (setting.mode) {
			case PinMode::Input:
				switch (setting.pull) {
					case Pull::Up: function = "{\"Input\":\"PullUp\"}"; break;
					case Pull::Down: function = "{\"Input\":\"PullDown\"}"; break;
					default: function = "{\"Input\":null}"; break;
				}
				break;
			case PinMode::Output:
				function = setting.level < 0 ? "{\"Output\":null}"
					: setting.level != 0 ? "{\"Output\":true}"
					: "{\"Output\":false}";
				break;
			default:
				continue;
		}
		if (!first)
			text += ",";
		first = false;
		char key[16];
		snprintf(key, sizeof(key), "\"%d\":", entry.first);
		text += key;
		text += function;
	}
	text += "}}";
	return text;
}


const char*
PullName(Pull pull)
{
	switch (pull) {
		case Pull::Up: return "Pull up";
		case Pull::Down: return "Pull down";
		default: return "No pull";
	}
}

}	// namespace airpins
