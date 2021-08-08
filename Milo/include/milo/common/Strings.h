#pragma once

#include <string>
#include <sstream>

namespace milo {

	using String = std::string;
	using StringStream = std::stringstream;

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
}