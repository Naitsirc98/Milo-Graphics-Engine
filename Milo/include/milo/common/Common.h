#pragma once

#include "Collections.h"
#include "Strings.h"
#include "Memory.h"
#include "Exceptions.h"
#include "Concurrency.h"
#include "milo/math/Math.h"
#include <typeinfo>

#include <functional>

namespace milo {

	template<typename R, typename ...Args>
	using Function = std::function<R(Args...)>;

	using TypeInfo = std::type_info;

	using TypeHash = size_t;

	template<typename T>
	inline constexpr TypeHash typeHash(const T& value) noexcept {
		return static_cast<TypeHash>(typeid(value).hash_code());
	}
}
