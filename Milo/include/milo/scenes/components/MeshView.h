#pragma once

#include "milo/assets/meshes/Mesh.h"
#include "milo/assets/materials/Material.h"

namespace milo {

	struct MeshView {

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		bool opaque = true;
	};

}