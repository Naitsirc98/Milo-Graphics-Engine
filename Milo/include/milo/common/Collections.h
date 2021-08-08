#pragma once

#include <array>
#include <vector>
#include <list>
#include <deque>
#include <unordered_map>
#include <map>

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
}