#include "milo/scenes/components/Tag.h"
#include <utility>

namespace milo {

	Tag::Tag(const String& value) {
		memcpy(m_Str, value.c_str(), std::min(value.size(), TAG_MAX_SIZE - 1) * sizeof(char));
		m_Hash = std::hash<String>{}(m_Str);
	}

	Tag::Tag(const char* const value) {
		size_t length = std::min(strlen(value), TAG_MAX_SIZE - 1);
		memcpy(m_Str, value, length * sizeof(char));
		m_Hash = std::hash<String>{}(m_Str);
	}

	const char* Tag::value() const {
		return m_Str;
	}

	String Tag::str() const {
		return m_Str;
	}

	bool Tag::operator==(const Tag& rhs) const {
		return m_Hash == rhs.m_Hash;
	}

	bool Tag::operator!=(const Tag& rhs) const {
		return ! (rhs == *this);
	}

	size_t Tag::hash() const {
		return m_Hash;
	}
}