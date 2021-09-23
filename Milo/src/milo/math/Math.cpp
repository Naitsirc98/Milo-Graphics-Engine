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

		Plane* planes = poly.plane;

		bool allowTestSpheres = true;

		const Matrix4& m = cameraMatrix;

		float nxX, nxY, nxZ, nxW;
		float pxX, pxY, pxZ, pxW;
		float nyX, nyY, nyZ, nyW;
		float pyX, pyY, pyZ, pyW;
		float nzX, nzY, nzZ, nzW;
		float pzX, pzY, pzZ, pzW;

		float invl;
		nxX = m[0][3] + m[0][0]; nxY = m[1][3] + m[1][0]; nxZ = m[2][3] + m[2][0]; nxW = m[3][3] + m[3][0];
		if (allowTestSpheres) {
			invl = (float) (1.0 / sqrtf(nxX * nxX + nxY * nxY + nxZ * nxZ));
			nxX *= invl; nxY *= invl; nxZ *= invl; nxW *= invl;
		}
		planes[0] = Plane(nxX, nxY, nxZ, nxW);
		pxX = m[0][3] - m[0][0]; pxY = m[1][3] - m[1][0]; pxZ = m[2][3] - m[2][0]; pxW = m[3][3] - m[3][0];
		if (allowTestSpheres) {
			invl = (float) (1.0 / sqrtf(pxX * pxX + pxY * pxY + pxZ * pxZ));
			pxX *= invl; pxY *= invl; pxZ *= invl; pxW *= invl;
		}
		planes[1] = Plane(pxX, pxY, pxZ, pxW);
		nyX = m[0][3] + m[0][1]; nyY = m[1][3] + m[1][1]; nyZ = m[2][3] + m[2][1]; nyW = m[3][3] + m[3][1];
		if (allowTestSpheres) {
			invl = (float) (1.0 / sqrtf(nyX * nyX + nyY * nyY + nyZ * nyZ));
			nyX *= invl; nyY *= invl; nyZ *= invl; nyW *= invl;
		}
		planes[2] = Plane(nyX, nyY, nyZ, nyW);
		pyX = m[0][3] - m[0][1]; pyY = m[1][3] - m[1][1]; pyZ = m[2][3] - m[2][1]; pyW = m[3][3] - m[3][1];
		if (allowTestSpheres) {
			invl = (float) (1.0 / sqrtf(pyX * pyX + pyY * pyY + pyZ * pyZ));
			pyX *= invl; pyY *= invl; pyZ *= invl; pyW *= invl;
		}
		planes[3] = Plane(pyX, pyY, pyZ, pyW);
		nzX = m[0][3] + m[0][2]; nzY = m[1][3] + m[1][2]; nzZ = m[2][3] + m[2][2]; nzW = m[3][3] + m[3][2];
		if (allowTestSpheres) {
			invl = (float) (1.0 / sqrtf(nzX * nzX + nzY * nzY + nzZ * nzZ));
			nzX *= invl; nzY *= invl; nzZ *= invl; nzW *= invl;
		}
		planes[4] = Plane(nzX, nzY, nzZ, nzW);
		pzX = m[0][3] - m[0][2]; pzY = m[1][3] - m[1][2]; pzZ = m[2][3] - m[2][2]; pzW = m[3][3] - m[3][2];
		if (allowTestSpheres) {
			invl = (float) (1.0 / sqrtf(pzX * pzX + pzY * pzY + pzZ * pzZ));
			pzX *= invl; pzY *= invl; pzZ *= invl; pzW *= invl;
		}
		planes[5] = Plane(pzX, pzY, pzZ, pzW);

		return poly;
	}
}