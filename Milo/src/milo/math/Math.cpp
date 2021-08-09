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

	int32 milo::Random::nextInt(int32 min, int32 max)
	{
		return rand() % (max - min + 1) + min;
	}

	float milo::Random::nextFloat(float min, float max)
	{
		return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max - min)));
	}
}