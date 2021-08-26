#pragma once

#include <string>
#include <sstream>
#include <regex>

namespace milo {

	using String = std::string;
	using StringStream = std::stringstream;
	using Regex = std::regex;

	template<typename T>
	inline String str(const T& value) {
		return std::to_string(value);
	}

	inline String str(const char* value) {
		return value;
	}

	template<>
	inline String str(const String& value) {
		return value;
	}

	inline String replace(const String& str, const Regex& regex, const String& replacement) {
		return std::regex_replace(str, regex, replacement);
	}
}