#pragma once

#include <array>
#include <vector>
#include <list>
#include <deque>
#include <unordered_map>
#include <map>
#include "Strings.h"

namespace milo {

	template<typename T, size_t LENGTH>
	using Array = std::array<T, LENGTH>;

	template<typename T>
	using ArrayList = std::vector<T>;

	template<typename T>
	using LinkedList = std::list<T>;

	template<typename T>
	using Deque = std::deque<T>;

	template<typename T>
	using Queue = Deque<T>;

	template<typename T>
	using Stack = Deque<T>;

	template<typename K, typename V>
	using HashMap = std::unordered_map<K, V>;

	template<typename K, typename V>
	using SortedMap = std::map<K, V>;

	template<typename First, typename Second>
	using Pair = std::pair<First, Second>;

	class Lists {
	public:
		template<typename T>
		inline static String str(const ArrayList<T>& list) {
			StringStream stream;
			stream << '[';
			for(size_t i = 0;i < list.size() - 1;++i) {
				stream << milo::str(list[i]) << ", ";
			}
			if(!list.empty()) stream << milo::str(list[list.size() - 1]);
			stream << ']';
			return stream.str();
		}
	};
}