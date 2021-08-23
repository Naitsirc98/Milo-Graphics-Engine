#include "milo/assets/meshes/MeshLoader.h"
#include "milo/io/Files.h"

namespace milo {

	MeshFormat MeshFormats::formatOf(const String& filename) {

		String extension = Files::extension(filename);

		if(extension == ".obj" || extension == ".OBJ") return MeshFormat::Obj;

		return MeshFormat::Unknown;
	}
}