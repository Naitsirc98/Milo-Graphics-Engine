#include "milo/scenes/Entity.h"
#include "milo/scenes/Scene.h"

namespace milo {

	Entity::Entity(EntityId id, Scene* scene) : m_Id(id), m_Scene(scene) {
		if(valid() && !hasComponent<Transform>()) {
			createComponent<Transform>();
		}
		if(valid() && !hasComponent<EntityBasicInfo>()) {
			createComponent<EntityBasicInfo>();
		}
	}

	Entity::~Entity() {
		m_Id = NULL_ENTITY;
		m_Scene = nullptr;
	}

	EntityId Entity::id() const noexcept {
		return m_Id;
	}

	milo::Scene* Entity::scene() const noexcept {
		return m_Scene;
	}

	bool Entity::valid() const noexcept {
		return m_Id != NULL_ENTITY && m_Scene != nullptr && m_Scene->registry().valid(m_Id);
	}

	void Entity::destroy() noexcept {
		if(!valid()) return;
		m_Scene->destroyEntity(m_Id);
	}

	bool Entity::operator==(const Entity& rhs) const {
		return m_Id == rhs.m_Id;
	}

	bool Entity::operator!=(const Entity& rhs) const {
		return !(rhs == *this);
	}

	const String& Entity::name() const {
		return getComponent<EntityBasicInfo>().name();
	}

	void Entity::setName(const String& name) {
		getComponent<EntityBasicInfo>().m_Name = name;
	}

	bool Entity::hasParent() const {
		return parent().id() != NULL_ENTITY;
	}

	Entity Entity::parent() const {
		EntityBasicInfo& relationships = getComponent<EntityBasicInfo>();
		return {relationships.parentId(), m_Scene};
	}

	void Entity::setParent(EntityId parentId) {
		if(parentId == m_Id) return;
		if(isAncestorOf(parentId) || isDescendantOf(parentId)) return;
		EntityBasicInfo& relationships = getComponent<EntityBasicInfo>();
		relationships.m_ParentId = parentId;
	}

	const ArrayList<EntityId>& Entity::children() const {
		EntityBasicInfo& relationships = getComponent<EntityBasicInfo>();
		return relationships.children();
	}

	bool Entity::isChild(EntityId childId) {
		EntityBasicInfo& relationships = getComponent<EntityBasicInfo>();
		return std::find(relationships.m_Children.begin(), relationships.m_Children.end(), childId) != relationships.m_Children.end();
	}

	void Entity::addChild(EntityId childId) {
		if(isAncestorOf(childId) || isDescendantOf(childId)) return;
		EntityBasicInfo& relationships = getComponent<EntityBasicInfo>();
		relationships.m_Children.push_back(childId);
		Entity childEntity = {childId, m_Scene};
		EntityBasicInfo& childRelationships = childEntity.getComponent<EntityBasicInfo>();
		if(childRelationships.m_ParentId != NULL_ENTITY) {
			Entity oldParent = {childRelationships.m_ParentId, m_Scene};
			oldParent.removeChild(childId);
		}
		childRelationships.m_ParentId = id();
	}

	void Entity::removeChild(EntityId childId) {
		if(!isChild(childId)) return;
		EntityBasicInfo& relationships = getComponent<EntityBasicInfo>();
		relationships.m_Children.erase(std::find(relationships.m_Children.begin(), relationships.m_Children.end(), childId));
	}

	void Entity::removeAllChildren() {
		EntityBasicInfo& relationships = getComponent<EntityBasicInfo>();
		relationships.m_Children.clear();
	}

	bool Entity::isAncestorOf(EntityId otherId) {

		Entity other = {otherId, m_Scene};

		const auto& children = this->children();

		if(children.empty()) return false;

		for (EntityId child : children) {
			if (child == otherId || other.isDescendantOf(child)) return true;
		}

		return false;
	}

	bool Entity::isDescendantOf(EntityId other) {
		Entity otherEntity = {other, m_Scene};
		return otherEntity.isAncestorOf(id());
	}
}