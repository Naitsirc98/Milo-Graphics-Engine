#pragma once

#include <string>

namespace milo {

	using String = std::string;

	template<typename T>
	inline String str(const T& value) {
		return std::to_string(value);
	}
}