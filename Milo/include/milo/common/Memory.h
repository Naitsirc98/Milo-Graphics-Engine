#pragma once

#include <memory>

#define DELETE_PTR(ptr) {delete ptr; ptr = nullptr;}
#define DELETE_ARRAY(arr) {delete arr; arr = nullptr;}

namespace milo {

	template<typename T>
	using SharedPtr = std::shared_ptr<T>;
	using std::make_shared;

	template<typename T>
	using UniquePtr = std::unique_ptr<T>;
	using std::make_unique;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

}