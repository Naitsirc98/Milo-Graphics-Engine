#pragma once

#include "milo/common/Common.h"

namespace milo {

	// TMP
	struct Mesh {

	};

	struct Material {
		Color color;
	};

	struct MeshView {

		Mesh* mesh = nullptr;
		Material* material = nullptr;
	};

}