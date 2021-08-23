#pragma once

#include "Mesh.h"

namespace milo {

	class MeshLoader {
	public:
		virtual Mesh* load(const String& filename) = 0;
	};

	enum class MeshFormat {
		Unknown, Obj
	};

	class MeshFormats {
	public:
		static MeshFormat formatOf(const String& filename);
	};
}