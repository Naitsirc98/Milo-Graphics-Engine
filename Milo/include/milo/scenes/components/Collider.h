#pragma once

#include "milo/scenes/EntityComponentSystem.h"
#include "milo/assets/meshes/Mesh.h"

namespace milo {

	struct SphereCollider {
		Vector3 center{0, 0, 0};
		float radius{0};

		static SphereCollider of(Mesh* mesh);
	};

	struct BoxCollider {
		Vector3 center;
		Vector3 size;
		Vector3 xAxis;
		Vector3 yAxis;
		Vector3 zAxis;

		static BoxCollider of(Mesh* mesh);
	};

}