#pragma once

#include "Model.h"

namespace milo {

	class ModelLoader {
	public:
		virtual ~ModelLoader() = default;
		virtual Model* load(const String& filename) = 0;
	};

}