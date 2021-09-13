#include "milo/math/Math.h"

namespace milo {
	
	String str(const Vector2& vec) {
		StringStream ss;
		ss << "(" << milo::str(vec[0]) << ", " << milo::str(vec[1]) << ")";
		return ss.str();
	}

	String str(const Vector3& vec)
	{
		StringStream ss;
		ss << "(" << milo::str(vec[0]) << ", " << milo::str(vec[1]) << ", " << milo::str(vec[2]) << ")";
		return ss.str();
	}

	String str(const Vector4& vec)
	{
		StringStream ss;
		ss << "(" << milo::str(vec[0]) << ", " << milo::str(vec[1]) << ", " << milo::str(vec[2]) << ", " << milo::str(vec[3]) << ")";
		return ss.str();
	}

	String str(const Matrix4& m)
	{
		StringStream ss;
		ss << str(m[0]) << "\n";
		ss << str(m[1]) << "\n";
		ss << str(m[2]) << "\n";
		ss << str(m[3]) << "\n";
		return ss.str();
	}

	inline static int genSeed() {
		srand(time(nullptr));
		return rand();
	}

	static int _ = genSeed();

	int32 milo::Random::nextInt(int32 min, int32 max) {
		return rand() % (max - min + 1) + min;
	}

	float milo::Random::nextFloat(float min, float max) {
		return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max - min)));
	}

	Polyhedron buildFrustumPolyhedron(const Matrix4& cameraMatrix, float g, float s, float n, float f) {

		Polyhedron poly;

		poly.vertexCount = 8;
		poly.edgeCount = 12;
		poly.faceCount = 6;

		// Generate vertices for the near side
		float y = n / g;
		float x = y * s;

		poly.vertex[0] = cameraMatrix * Vector4(x, y, n, 1);
		poly.vertex[1] = cameraMatrix * Vector4(x, -y, n, 1);
		poly.vertex[2] = cameraMatrix * Vector4(-x, -y, n, 1);
		poly.vertex[3] = cameraMatrix * Vector4(-x, y, n, 1);

		// Generate vertices for the far side
		y = f / g;
		x = y * s;
		poly.vertex[4] = cameraMatrix * Vector4(x, y, f, 1);
		poly.vertex[5] = cameraMatrix * Vector4(x, -y, f, 1);
		poly.vertex[6] = cameraMatrix * Vector4(-x, -y, f, 1);
		poly.vertex[7] = cameraMatrix * Vector4(-x, y, f, 1);

		// Generate lateral planes
		Matrix4 inverse = glm::inverse(cameraMatrix);
		float mx = 1.0f / sqrtf(g * g * s);
		float my = 1.0f / sqrtf(g * g + 1.0f);
		poly.plane[0] = Plane(-g * mx, 0, s * mx, 0) * inverse;
		poly.plane[1] = Plane(0, g * my, my, 0) * inverse;
		poly.plane[2] = Plane(g * mx, 0, s * mx, 0) * inverse;
		poly.plane[3] = Plane(0, -g * my, my, 0) * inverse;

		// Generate near and far planes
		float d = dot(cameraMatrix[2], cameraMatrix[3]);
		poly.plane[4] = Plane(cameraMatrix[2], -(d + n));
		poly.plane[5] = Plane(-cameraMatrix[2], d + f);

		// Generate all edges and lateral faces
		Edge* edge = poly.edge;
		Face* face = poly.face;
		for(int32_t i = 0;i < 4;++i, ++edge, ++face) {

			edge[0].vertexIndex[0] = i;
			edge[0].vertexIndex[1] = i + 4;
			edge[0].faceIndex[0] = i;
			edge[0].faceIndex[1] = (i - 1) & 3;

			edge[4].vertexIndex[0] = i;
			edge[4].vertexIndex[1] = (i + 1) & 3;
			edge[4].faceIndex[0] = 4;
			edge[4].faceIndex[1] = i;

			edge[8].vertexIndex[0] = ((i + 1) & 3) + 4;
			edge[8].vertexIndex[1] = i + 4;
			edge[8].faceIndex[0] = 5;
			edge[8].faceIndex[1] = i;

			face->edgeCount = 4;
			face->edgeIndex[0] = i;
			face->edgeIndex[1] = (i + 1) & 3;
			face->edgeIndex[2] = i + 4;
			face->edgeIndex[3] = i + 8;
		}

		// Generate near and far faces
		face[0].edgeCount = face[1].edgeCount = 4;
		face[0].edgeIndex[0] = 4;
		face[0].edgeIndex[1] = 5;
		face[0].edgeIndex[2] = 6;
		face[0].edgeIndex[3] = 7;
		face[1].edgeIndex[0] = 8;
		face[1].edgeIndex[1] = 9;
		face[1].edgeIndex[2] = 10;
		face[1].edgeIndex[3] = 11;

		return poly;
	}
}