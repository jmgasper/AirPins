/*
 * Copyright 2026, air/OS contributors.
 * Distributed under the terms of the MIT License.
 */

#include "Json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace airpins {

class JsonParser {
public:
	JsonParser(const std::string& text)
		:
		fText(text),
		fPosition(0),
		fDepth(0)
	{
	}

	bool Parse(JsonValue& value, std::string* error)
	{
		bool ok = _Value(value);
		if (ok) {
			_SkipSpace();
			if (fPosition != fText.size())
				ok = _Fail("unexpected text after the value");
		}
		if (!ok && error != NULL)
			*error = fError;
		return ok;
	}

private:
	bool _Fail(const char* what)
	{
		if (fError.empty()) {
			char buffer[160];
			snprintf(buffer, sizeof(buffer), "%s at offset %zu", what,
				fPosition);
			fError = buffer;
		}
		return false;
	}

	void _SkipSpace()
	{
		while (fPosition < fText.size()
			&& strchr(" \t\r\n", fText[fPosition]) != NULL) {
			fPosition++;
		}
	}

	bool _Literal(const char* literal)
	{
		size_t length = strlen(literal);
		if (fText.compare(fPosition, length, literal) != 0)
			return false;
		fPosition += length;
		return true;
	}

	bool _Value(JsonValue& value)
	{
		if (++fDepth > 64)
			return _Fail("nested too deeply");
		_SkipSpace();
		if (fPosition >= fText.size())
			return _Fail("unexpected end");

		bool ok;
		char c = fText[fPosition];
		if (c == '{')
			ok = _Object(value);
		else if (c == '[')
			ok = _Array(value);
		else if (c == '"') {
			value.fType = JsonValue::Type::String;
			ok = _String(value.fString);
		} else if (_Literal("true")) {
			value.fType = JsonValue::Type::Bool;
			value.fBool = true;
			ok = true;
		} else if (_Literal("false")) {
			value.fType = JsonValue::Type::Bool;
			value.fBool = false;
			ok = true;
		} else if (_Literal("null")) {
			value.fType = JsonValue::Type::Null;
			ok = true;
		} else if (c == '-' || (c >= '0' && c <= '9'))
			ok = _Number(value);
		else
			ok = _Fail("unexpected character");
		fDepth--;
		return ok;
	}

	bool _Number(JsonValue& value)
	{
		const char* start = fText.c_str() + fPosition;
		char* end;
		double number = strtod(start, &end);
		if (end == start)
			return _Fail("bad number");
		fPosition += end - start;
		value.fType = JsonValue::Type::Number;
		value.fNumber = number;
		return true;
	}

	static void _AppendUtf8(std::string& out, unsigned long code)
	{
		if (code < 0x80)
			out += (char)code;
		else if (code < 0x800) {
			out += (char)(0xc0 | (code >> 6));
			out += (char)(0x80 | (code & 0x3f));
		} else if (code < 0x10000) {
			out += (char)(0xe0 | (code >> 12));
			out += (char)(0x80 | ((code >> 6) & 0x3f));
			out += (char)(0x80 | (code & 0x3f));
		} else {
			out += (char)(0xf0 | (code >> 18));
			out += (char)(0x80 | ((code >> 12) & 0x3f));
			out += (char)(0x80 | ((code >> 6) & 0x3f));
			out += (char)(0x80 | (code & 0x3f));
		}
	}

	bool _Hex4(unsigned long& code)
	{
		if (fPosition + 4 > fText.size())
			return _Fail("short \\u escape");
		char digits[5] = {};
		memcpy(digits, fText.c_str() + fPosition, 4);
		char* end;
		code = strtoul(digits, &end, 16);
		if (end != digits + 4)
			return _Fail("bad \\u escape");
		fPosition += 4;
		return true;
	}

	bool _String(std::string& out)
	{
		fPosition++;	// the quote
		out.clear();
		while (fPosition < fText.size()) {
			char c = fText[fPosition++];
			if (c == '"')
				return true;
			if ((unsigned char)c < 0x20)
				return _Fail("control character in a string");
			if (c != '\\') {
				out += c;
				continue;
			}
			if (fPosition >= fText.size())
				break;
			c = fText[fPosition++];
			switch (c) {
				case '"': case '\\': case '/': out += c; break;
				case 'b': out += '\b'; break;
				case 'f': out += '\f'; break;
				case 'n': out += '\n'; break;
				case 'r': out += '\r'; break;
				case 't': out += '\t'; break;
				case 'u':
				{
					unsigned long code;
					if (!_Hex4(code))
						return false;
					if (code >= 0xd800 && code < 0xdc00
						&& _Literal("\\u")) {
						unsigned long low;
						if (!_Hex4(low))
							return false;
						if (low >= 0xdc00 && low < 0xe000)
							code = 0x10000 + ((code - 0xd800) << 10)
								+ (low - 0xdc00);
					}
					_AppendUtf8(out, code);
					break;
				}
				default:
					return _Fail("bad escape");
			}
		}
		return _Fail("unterminated string");
	}

	bool _Array(JsonValue& value)
	{
		fPosition++;
		value.fType = JsonValue::Type::Array;
		_SkipSpace();
		if (_Literal("]"))
			return true;
		while (true) {
			value.fArray.emplace_back();
			if (!_Value(value.fArray.back()))
				return false;
			_SkipSpace();
			if (_Literal("]"))
				return true;
			if (!_Literal(","))
				return _Fail("expected , or ]");
		}
	}

	bool _Object(JsonValue& value)
	{
		fPosition++;
		value.fType = JsonValue::Type::Object;
		_SkipSpace();
		if (_Literal("}"))
			return true;
		while (true) {
			_SkipSpace();
			if (fPosition >= fText.size() || fText[fPosition] != '"')
				return _Fail("expected a member name");
			std::string name;
			if (!_String(name))
				return false;
			_SkipSpace();
			if (!_Literal(":"))
				return _Fail("expected :");
			value.fMembers.emplace_back(name, JsonValue());
			if (!_Value(value.fMembers.back().second))
				return false;
			_SkipSpace();
			if (_Literal("}"))
				return true;
			if (!_Literal(","))
				return _Fail("expected , or }");
		}
	}

	const std::string&	fText;
	size_t				fPosition;
	int					fDepth;
	std::string			fError;
};


const JsonValue*
JsonValue::Find(const char* name) const
{
	for (const auto& member : fMembers) {
		if (member.first == name)
			return &member.second;
	}
	return NULL;
}


bool
JsonValue::Parse(const std::string& text, JsonValue& value, std::string* error)
{
	value = JsonValue();
	JsonParser parser(text);
	return parser.Parse(value, error);
}


std::string
JsonValue::Quote(const std::string& text)
{
	std::string out = "\"";
	for (unsigned char c : text) {
		switch (c) {
			case '"': out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default:
				if (c < 0x20) {
					char buffer[8];
					snprintf(buffer, sizeof(buffer), "\\u%04x", c);
					out += buffer;
				} else
					out += (char)c;
		}
	}
	return out + "\"";
}

}	// namespace airpins
