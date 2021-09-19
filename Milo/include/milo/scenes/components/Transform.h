#pragma once

#include "milo/math/Math.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace milo {

	class Transform {
		friend class Scene;
	private:
		Vector3 m_Translation{0.0f, 0.0f, 0.0f};
		Vector3 m_TranslationDelta{0.0f, 0.0f, 0.0f};
		Vector3 m_Scale{1.0f, 1.0f, 1.0f};
		Vector3 m_ScaleDelta{0.0f, 0.0f, 0.0f};
		Quaternion m_Rotation{0.0f, 0.0f, 0.0f, 1.0f};
		Quaternion m_RotationDelta{0.0f, 0.0f, 0.0f, 0.0f};
		Matrix4 m_ModelMatrix = Matrix4(1.0f);
		bool m_Dirty{false};

	public:
		Transform() = default;

		inline const Vector3& translation() const {
			return m_Translation;
		}

		inline void translation(const Vector3& translation) {
			if(m_Translation == translation) return;
			m_TranslationDelta = translation - m_Translation;
			m_Translation = translation;
			m_Dirty = true;
		}

		inline const Vector3& scale() const {
			return m_Scale;
		}

		inline void scale(const Vector3& scale) {
			if(m_Scale == scale) return;
			m_ScaleDelta = scale - m_Scale;
			m_Scale = scale;
			m_Dirty = true;
		}

		inline const Quaternion& rotation() const {
			return m_Rotation;
		}

		inline void rotation(const Quaternion& rotation) {
			if(m_Rotation == rotation) return;
			m_RotationDelta = rotation - m_Rotation;
			m_Rotation = rotation;
			m_Dirty = true;
		}

		inline void rotate(float radians, const Vector3 axis) {
			rotation(angleAxis(radians, axis));
		}

		inline const Matrix4& modelMatrix() const noexcept {
			return m_ModelMatrix;
		}

		inline void setMatrix(const Matrix4& matrix) {
			Vector3 scale;
			Quaternion rotation;
			Vector3 translation;
			Vector3 skew;
			Vector4 perspective;
			glm::decompose(matrix, scale, rotation, translation, skew, perspective);
			this->translation(translation);
			this->scale(scale);
			this->rotation(rotation);
		}

	private:

		inline void update() {
			m_ModelMatrix = glm::translate(m_Translation) * glm::scale(m_Scale) * glm::toMat4(m_Rotation);
			m_TranslationDelta = Vector3();
			m_ScaleDelta = Vector3();
			m_RotationDelta = Quaternion();
			m_Dirty = false;
		}
	};
}