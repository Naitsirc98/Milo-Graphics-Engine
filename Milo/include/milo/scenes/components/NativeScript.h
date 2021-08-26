#pragma once

#include "milo/scenes/EntityComponentSystem.h"

namespace milo {

	class Entity;

	class NativeScript {
		friend class Scene;
		friend class NativeScriptView;
	protected:
		virtual void onCreate(EntityId entityId) {};
		virtual void onUpdate(EntityId entityId) {};
		virtual void onLateUpdate(EntityId entityId) {};
	};

	struct NativeScriptView {
		friend class Scene;
	public:
		NativeScript* script = nullptr;
	private:
		Function<void> create = {};
		Function<void> destroy = {};
	public:
		template<typename T, typename ...Args>
		inline void bind(Args&& ...args) {
			create = [&]() {
				script = new T(std::forward<Args>(args)...);
			};
			destroy = [&]() {
				delete script;
				script = nullptr;
			};
		}
	private:
		inline void createIfNotExists(EntityId entityId) {
			if(script == nullptr) {
				create();
				script->onCreate(entityId);
			}
		}
	};
}