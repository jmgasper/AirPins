/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace airpins {

//! Just enough JSON for configuration files.
class JsonValue {
public:
	enum class Type { Null, Bool, Number, String, Array, Object };

						JsonValue() : fType(Type::Null), fBool(false),
							fNumber(0) {}

	Type				GetType() const { return fType; }
	bool				IsNull() const { return fType == Type::Null; }
	bool				IsBool() const { return fType == Type::Bool; }
	bool				IsNumber() const { return fType == Type::Number; }
	bool				IsString() const { return fType == Type::String; }
	bool				IsObject() const { return fType == Type::Object; }
	bool				IsArray() const { return fType == Type::Array; }

	bool				Bool() const { return fBool; }
	double				Number() const { return fNumber; }
	const std::string&	String() const { return fString; }
	const std::vector<JsonValue>& Array() const { return fArray; }
	//! Members in file order.
	const std::vector<std::pair<std::string, JsonValue>>& Members() const
							{ return fMembers; }
	const JsonValue*	Find(const char* name) const;

	static bool			Parse(const std::string& text, JsonValue& value,
							std::string* error = NULL);
	static std::string	Quote(const std::string& text);

private:
	friend class JsonParser;

	Type				fType;
	bool				fBool;
	double				fNumber;
	std::string			fString;
	std::vector<JsonValue> fArray;
	std::vector<std::pair<std::string, JsonValue>> fMembers;
};

}	// namespace airpins
