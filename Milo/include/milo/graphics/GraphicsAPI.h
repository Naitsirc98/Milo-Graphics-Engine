#pragma once

#include "milo/common/Strings.h"

namespace milo {

	enum class GraphicsAPI {
		Vulkan,
		Default = Vulkan
	};

	template<>
	inline String str(const GraphicsAPI& graphicsApi) {
		if(graphicsApi == GraphicsAPI::Vulkan) return "Vulkan";
		return "Unknown GraphicsAPI";
	}
}