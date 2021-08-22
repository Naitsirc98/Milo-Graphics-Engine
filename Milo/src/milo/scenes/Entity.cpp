#include "milo/scenes/Entity.h"
#include "milo/scenes/Scene.h"

namespace milo {

	Entity::Entity(EntityId id, milo::Scene &scene) : m_Id(id), m_Scene(scene) {
		if(valid() && !hasComponent<Transform>()) {
			createComponent<Transform>();
		}
	}

	EntityId Entity::id() const noexcept {
		return m_Id;
	}

	milo::Scene &Entity::scene() const noexcept {
		return m_Scene;
	}

	bool Entity::valid() const noexcept {
		return m_Scene.registry().valid(m_Id);
	}

	void Entity::destroy() noexcept {
		if(!valid()) return;
		m_Scene.destroyEntity(m_Id);
	}

	bool Entity::operator==(const Entity& rhs) const {
		return m_Id == rhs.m_Id;
	}

	bool Entity::operator!=(const Entity& rhs) const {
		return !(rhs == *this);
	}
}