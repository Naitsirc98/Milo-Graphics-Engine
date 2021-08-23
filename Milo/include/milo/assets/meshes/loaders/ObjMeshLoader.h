#pragma once

#include "milo/assets/meshes/MeshLoader.h"

namespace milo {

	class ObjMeshLoader : public MeshLoader {
	public:
		Mesh* load(const String& filename) override;
	};

}