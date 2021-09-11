#pragma once

#include "milo/assets/skybox/Skybox.h"

namespace milo {

	enum class SkyType {
		Static, Dynamic
	};

	struct SkyboxView {
		Skybox* skybox{nullptr};
		SkyType type{SkyType::Static};
		bool enabled{true};
	};

}