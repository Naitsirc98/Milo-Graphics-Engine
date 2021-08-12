#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "milo/common/Strings.h"

namespace milo {

		using namespace glm;

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
				size_t width;
				size_t height;
				Size() : Size(0, 0) {}
				Size(size_t width, size_t height) : width(width), height(height) {}

				[[nodiscard]] float aspect() const {return height == 0 ? 0.0f : (float)width / (float)height;}
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
	}