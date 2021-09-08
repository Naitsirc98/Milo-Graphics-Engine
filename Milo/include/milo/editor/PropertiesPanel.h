#pragma once

#include "milo/scenes/Scene.h"
#include "milo/scenes/Entity.h"

namespace milo {

	class PropertiesPanel {
	public:
		PropertiesPanel();
		~PropertiesPanel();
	public:
		void render(Entity selected);
	private:
		void handleAddComponents(Entity entity);
	private:
		static bool drawVector3Control(const String& label, Vector3& vector, float resetValue = 1.0f, float columnWidth = 100.0f);
	};

}