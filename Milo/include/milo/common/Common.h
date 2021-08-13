#pragma once

#include "Collections.h"
#include "Strings.h"
#include "Memory.h"
#include "Exceptions.h"
#include "Concurrency.h"
#include "milo/math/Math.h"
#include <typeinfo>
#include <optional>

#include <functional>

namespace milo {

	using ResourceHandle = size_t;

	template<typename R, typename ...Args>
	using Function = std::function<R(Args...)>;

	using TypeInfo = std::type_info;

	using TypeHash = size_t;

	template<typename T>
	inline constexpr TypeHash typeHash(const T& value) noexcept {
		return static_cast<TypeHash>(typeid(value).hash_code());
	}

	template<typename T>
	using Optional = std::optional<T>;
}
