#pragma once

#include "milo/assets/meshes/Mesh.h"
#include "milo/assets/materials/Material.h"
#include "milo/assets/AssetManager.h"

namespace milo {

	struct MeshView {

		Mesh* mesh{Assets::meshes().getCube()};
		Material* material{Assets::materials().getDefault()};
		bool opaque = true;
	};

}