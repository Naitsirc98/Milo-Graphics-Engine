#include "milo/scenes/components/Tag.h"
#include <utility>

namespace milo {

	Tag::Tag(const String& value) {
		setValue(value);
	}

	Tag::Tag(const char* const value) {
		setValue(value);
	}

	void Tag::setValue(const char* value) {
		m_Length = std::min((uint32_t)strlen(value), TAG_MAX_SIZE - 1);
		memcpy(m_Str, value, m_Length * sizeof(char));
		m_Hash = std::hash<String>{}(m_Str);
	}

	void Tag::setValue(const String& value) {
		memcpy(m_Str, value.c_str(), std::min((uint32_t)value.size(), TAG_MAX_SIZE - 1) * sizeof(char));
		m_Length = value.size();
		m_Hash = std::hash<String>{}(m_Str);
	}

	const char* Tag::value() const {
		return m_Str;
	}

	const uint32_t Tag::size() const {
		return m_Length;
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