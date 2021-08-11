#pragma once

#include "milo/common/Strings.h"

namespace milo {

	enum class GraphicsAPI {
		Vulkan,
		Default = Vulkan
	};

	template<>
	inline String str(const GraphicsAPI& value) {
		if(value == GraphicsAPI::Vulkan) return "Vulkan";
		return "Unknown GraphicsAPI";
	}
}