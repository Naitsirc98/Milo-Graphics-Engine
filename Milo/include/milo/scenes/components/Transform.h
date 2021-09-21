#pragma once

#include "milo/scenes/EntityComponentSystem.h"
#include "milo/math/Math.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace milo {

	class Transform {
		friend class Scene;
		friend class Entity;
	private:
		EntityId m_EntityId{};
		Vector3 m_Translation{0.0f, 0.0f, 0.0f};
		Vector3 m_TranslationDelta{0.0f, 0.0f, 0.0f};
		Vector3 m_Scale{1.0f, 1.0f, 1.0f};
		Vector3 m_ScaleDelta{0.0f, 0.0f, 0.0f};
		Quaternion m_Rotation{0.0f, 0.0f, 0.0f, 1.0f};
		Quaternion m_RotationDelta{0.0f, 0.0f, 0.0f, 0.0f};

	public:
		Transform() = default;

		inline const Vector3& translation() const {
			return m_Translation;
		}

		inline void translation(const Vector3& translation) {
			if(m_Translation == translation) return;
			m_TranslationDelta = translation - m_Translation;
			//updateChildrenPosition(translation);
			m_Translation = translation;
		}

		inline const Vector3& scale() const {
			return m_Scale;
		}

		inline void scale(const Vector3& scale) {
			if(m_Scale == scale) return;
			m_ScaleDelta = scale - m_Scale;
			//updateChildrenScale(scale);
			m_Scale = scale;
		}

		inline const Quaternion& rotation() const {
			return m_Rotation;
		}

		inline void rotation(const Quaternion& rotation) {
			if(m_Rotation == rotation) return;
			m_RotationDelta = rotation - m_Rotation;
			//updateChildrenRotation(rotation);
			m_Rotation = rotation;
		}

		inline void rotate(float radians, const Vector3 axis) {
			rotation(angleAxis(radians, axis));
		}

		Matrix4 modelMatrix() const noexcept;

		inline Matrix4 localModelMatrix() const noexcept {
			return glm::translate(m_Translation) * glm::scale(m_Scale) * glm::toMat4(m_Rotation);
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

			//if(m_TranslationDelta != Vector3()) updateChildrenPosition(m_TranslationDelta);
			//if(m_ScaleDelta != Vector3()) updateChildrenScale(m_ScaleDelta);
			//if(m_RotationDelta != Quaternion()) updateChildrenRotation(m_RotationDelta);

			m_TranslationDelta = Vector3();
			m_ScaleDelta = Vector3();
			m_RotationDelta = Quaternion();
		}

		void updateChildrenPosition(const Vector3& position);
		void updateChildrenScale(const Vector3& scale);
		void updateChildrenRotation(const Quaternion& rotation);
	};
}