#pragma once

#include "milo/math/Math.h"

namespace milo {


	struct Vertex {

		using Attribute = uint8_t;

		inline static const Attribute Attrib_Position = 0x1;
		inline static const Attribute Attrib_Normal = 0x2;
		inline static const Attribute Attrib_TexCoords = 0x4;
		inline static const Attribute Attrib_Tangent = 0x8;
		inline static const Attribute Attrib_BiTangent = 0x10;

		Vector3 position = {0, 0, 0};
		Vector3 normal = {0, 0, 0};
		Vector2 texCoords = {0, 0};
		Vector3 tangent = {0, 0, 0};
		Vector3 biTangent = {0, 0, 0};
	};

	const Vertex::Attribute VERTEX_ALL_ATTRIBUTES = Vertex::Attrib_Position
													| Vertex::Attrib_Normal
													| Vertex::Attrib_TexCoords
													| Vertex::Attrib_Tangent
													| Vertex::Attrib_BiTangent;

	const Vertex::Attribute VERTEX_BASIC_ATTRIBUTES = Vertex::Attrib_Position
													  | Vertex::Attrib_Normal
													  | Vertex::Attrib_TexCoords;
}