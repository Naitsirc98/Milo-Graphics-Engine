#pragma once

#include "BasicDefinitions.h"
#include "Collections.h"
#include "Strings.h"
#include "Memory.h"
#include "Exceptions.h"
#include "milo/math/Math.h"

#include <functional>

namespace milo {

	template<typename R, typename ...Args>
	using Function = std::function<R(Args...)>;

	using TypeInfo = std::type_info;
}
