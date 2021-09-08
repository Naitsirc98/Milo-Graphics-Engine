#pragma once

#include "milo/common/Strings.h"

namespace milo {

	const uint32_t TAG_MAX_SIZE = 128 - sizeof(size_t);

	class Tag {
	private:
		char m_Str[TAG_MAX_SIZE]{0};
		uint32_t m_Length = 0;
		size_t m_Hash = 0;
	public:
		Tag() = default;
		Tag(const char* value);
		Tag(const String& value);

		bool operator==(const Tag& rhs) const;
		bool operator!=(const Tag& rhs) const;

		const char* value() const;
		const uint32_t size() const;
		String str() const;
		size_t hash() const;

		void setValue(const char* value);
		void setValue(const String& value);
	};

	template<>
	inline String str(const Tag& value) {
		return value.value();
	}
}

namespace std {
	template<> struct hash<milo::Tag> {
		std::size_t operator()(const milo::Tag& tag) const {
			return tag.hash();
		}
	};
}