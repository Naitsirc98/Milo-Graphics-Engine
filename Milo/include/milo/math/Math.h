#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "milo/common/Strings.h"

#define MILO_PI 3.14159265359f

namespace milo {

	using namespace glm;

	using byte_t = int8_t;

	using Vector2 = glm::vec2;
	using Vector2i = glm::ivec2;

	using Vector3 = glm::vec3;
	using Vector3i = glm::ivec3;

	using Vector4 = glm::vec4;
	using Vector4i = glm::ivec4;

	using Matrix3 = glm::mat3;
	using Matrix4 = glm::mat4;

	using Quaternion = glm::quat;

	template<typename MathStruct>
	inline const typename MathStruct::value_type* ptr(const MathStruct& mathStruct) noexcept
	{
		return glm::value_ptr(mathStruct);
	}

	struct Size {

		int32_t width;
		int32_t height;
		Size() : Size(0, 0) {}
		Size(int32_t width, int32_t height) : width(width), height(height) {}

		inline float aspect() const noexcept {return height == 0 ? 0.0f : (float)width / (float)height;}

		inline bool isZero() const noexcept {return width == 0 || height == 0;}

		inline bool operator==(const Size& rhs) const noexcept {
			return width == rhs.width && height == rhs.height;
		}

		inline bool operator!=(const Size& rhs) const noexcept {
			return !(rhs == *this);
		}
	};

	template<typename T>
	struct Range
	{
		T min;
		T max;
	};

	struct Viewport
	{
		float x;
		float y;
		float width;
		float height;

		constexpr float aspect() const {return height == 0.0f ? 0.0f : width / height;}
	};

	String str(const Vector2& vec);
	String str(const Vector3& vec);
	String str(const Vector4& vec);
	String str(const Matrix4& m);

	class Random
	{
	private:
		Random() = default;
	public:
		static int32 nextInt(int32 min = 0, int32 max = RAND_MAX);
		static float nextFloat(float min = 0.0f, float max = 1.0f);
	};

	template<typename Integer>
	inline Integer roundUp2(Integer numToRound, Integer multiple) {
		return (numToRound + multiple - 1) & -multiple;
	}

	struct Plane {

		union {
			struct {Vector3 xyz;};
			struct {float x, y, z;};
		};

		float w;

		explicit Plane(float x = 0, float y = 0, float z = 0, float w = 0) : xyz(x, y, z), w(w) {}
		explicit Plane(const Vector3& normal, float w = 0) : xyz(normal), w(w) {}

		inline Plane& operator*(const Matrix4& m) {
			Vector4 v(xyz, w);
			v = m * v;
			xyz = {v.x, v.y, v.z};
			w = v.w;
			return *this;
		}
	};

	class Mesh;

	struct BoundingVolume {

		enum class Type {
			Sphere, AABB, OBB
		};

		virtual size_t byteSize() const = 0;
		virtual Type type() const = 0;
		virtual bool isVisible(const Matrix4& transform, const Plane* planes, uint32_t planeCount) const = 0;
	};

	struct BoundingSphere : public BoundingVolume {

		Vector3 center{0, 0, 0};
		float radius{0};

		inline size_t byteSize() const override {return sizeof(center) + sizeof(radius);}
		Type type() const override {return Type::Sphere;};
		bool isVisible(const Matrix4& transform, const Plane* planes, uint32_t planeCount) const override;

		static BoundingSphere of(Mesh* mesh);
	};

	struct AxisAlignedBoundingBox : public BoundingVolume {

		Vector3 center;
		Vector3 size;

		inline size_t byteSize() const override {return sizeof(center) + sizeof(size);}
		Type type() const override {return Type::AABB;};
		bool isVisible(const Matrix4& transform, const Plane* planes, uint32_t planeCount) const override;

		static AxisAlignedBoundingBox of(Mesh* mesh);
	};
	using AABB = AxisAlignedBoundingBox;

	struct OrientedBoundingBox : public BoundingVolume {

		Vector3 center;
		Vector3 size;
		Vector3 xAxis;
		Vector3 yAxis;
		Vector3 zAxis;

		inline size_t byteSize() const override {return sizeof(center) + sizeof(size) + sizeof(xAxis) + sizeof(yAxis) + sizeof(zAxis);}
		Type type() const override {return Type::OBB;};
		bool isVisible(const Matrix4& transform, const Plane* planes, uint32_t planeCount) const override;

		static OrientedBoundingBox of(Mesh* mesh);
	};
	using OBB = OrientedBoundingBox;

	static constexpr int32_t MAX_POLYHEDRON_VERTEX_COUNT = 28;
	static constexpr int32_t MAX_POLYHEDRON_FACE_COUNT = 16;
	static constexpr int32_t MAX_POLYHEDRON_EDGE_COUNT = (MAX_POLYHEDRON_FACE_COUNT - 2) * 3;
	static constexpr int32_t MAX_POLYHEDRON_FACE_EDGE_COUNT = MAX_POLYHEDRON_FACE_COUNT - 1;

	struct Edge {
		uint8_t vertexIndex[2];
		uint8_t faceIndex[2];
	};

	struct Face {
		uint8_t edgeCount;
		uint8_t edgeIndex[MAX_POLYHEDRON_FACE_EDGE_COUNT];
	};

	struct Polyhedron {

		uint8_t vertexCount;
		uint8_t edgeCount;
		uint8_t faceCount;

		Vector3 vertex[MAX_POLYHEDRON_VERTEX_COUNT];
		Edge edge[MAX_POLYHEDRON_EDGE_COUNT];
		Face face[MAX_POLYHEDRON_FACE_COUNT];
		Plane plane[MAX_POLYHEDRON_FACE_COUNT];
	};

	inline float distancePointLine(const Vector3& q, const Vector3& p, const Vector3& v) {
		Vector3 a = cross(q - p, v);
		return sqrt(dot(a, a) / dot(v, v));
	}

	inline const Vector3& minv(const Vector3& v1, const Vector3& v2) {
		return length2(v1) >= length2(v2) ? v1 : v2;
	}

	inline void decomposeModelMatrix(const Matrix4& m, Vector3* scale = nullptr, Quaternion* orientation = nullptr,
									 Vector3* translation = nullptr, Vector3* skew = nullptr, Vector4* perspective = nullptr) {

		Vector3 s;
		Quaternion o;
		Vector3 t;
		Vector3 sk;
		Vector4 p;

		glm::decompose(m, s, o, t, sk, p);

		if(scale != nullptr) *scale = s;
		if(orientation != nullptr) *orientation = o;
		if(translation != nullptr) *translation = t;
		if(skew != nullptr) *skew = sk;
		if(perspective != nullptr) *perspective = p;
	}

	inline Vector3 getScale(const Matrix4& m) {
		Vector3 scale;
		decomposeModelMatrix(m, &scale);
		return scale;
	}

	inline void decomposeProjectionMatrix(const Matrix4& proj, float& fov, float& znear, float& zfar) {
		const float* m = value_ptr(proj);
		fov = 2 * atan(1.0f / m[5]) * 180.0f / MILO_PI;
		znear = m[14] / (m[10] - 1.0f);
		zfar = m[14] / (m[10] + 1.0f);
	}

	Polyhedron buildFrustumPolyhedron(const Matrix4& cameraMatrix, float g, float s, float n, float f);
}