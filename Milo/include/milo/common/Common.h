#pragma once

#define NOMINMAX

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

	using Handle = uint64_t;

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

	using Color3 = Vector3;
	using Color = Vector4;

	namespace Colors {

		const Color WHITE{1, 1, 1, 1};
		const Color BLACK{0, 0, 0, 1};
		const Color RED{1, 0, 0, 1};
		const Color GREEN{0, 1, 0, 1};
		const Color BLUE{0, 0, 1, 1};
	}

	enum class SimulationState {
		Editor,
		EditorPlay,
		Play
	};

	SimulationState getSimulationState();
	void setSimulationState(SimulationState state);
}
