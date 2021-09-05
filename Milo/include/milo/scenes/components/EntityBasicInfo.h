#pragma once

#include "milo/scenes/EntityComponentSystem.h"

namespace milo {

	class EntityBasicInfo {
		friend class Entity;
	private:
		String m_Name{"Unnamed"};
		EntityId m_ParentId{NULL_ENTITY};
		ArrayList<EntityId> m_Children;
	public:
		EntityBasicInfo() = default;
		~EntityBasicInfo() = default;
		inline const String& name() const {return m_Name;}
		inline EntityId parentId() const {return m_ParentId;}
		inline const ArrayList<EntityId>& children() const {return m_Children;}
	};
}