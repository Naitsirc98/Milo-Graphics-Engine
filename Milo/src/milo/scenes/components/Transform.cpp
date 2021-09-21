#include "milo/scenes/components/Transform.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"

namespace milo {

	void Transform::updateChildrenPosition(const Vector3& position) {

		Vector3 delta = position - m_Translation;
		Entity entity = SceneManager::activeScene()->find(m_EntityId);

		for(EntityId childId : entity.children()) {
			Entity child = {childId, entity.scene()};
			child.getComponent<Transform>().translation(delta);
		}
	}

	void Transform::updateChildrenScale(const Vector3& scale) {

		Vector3 delta = scale - m_Scale;
		Entity entity = SceneManager::activeScene()->find(m_EntityId);

		for(EntityId childId : entity.children()) {
			Entity child = {childId, entity.scene()};
			Transform& transform = child.getComponent<Transform>();
			transform.scale(transform.m_Scale + delta);
		}
	}

	void Transform::updateChildrenRotation(const Quaternion& rotation) {

		Entity entity = SceneManager::activeScene()->find(m_EntityId);

		for(EntityId childId : entity.children()) {
			Entity child = {childId, entity.scene()};
			child.getComponent<Transform>().rotation(rotation);
		}
	}
}