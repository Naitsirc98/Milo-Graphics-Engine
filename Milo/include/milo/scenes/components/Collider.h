#pragma once

#include "milo/scenes/EntityComponentSystem.h"
#include "milo/assets/meshes/Mesh.h"

namespace milo {

	struct SphereCollider {
		BoundingSphere sphere;
	};

	struct BoxCollider {
		OrientedBoundingBox box;
	};

}