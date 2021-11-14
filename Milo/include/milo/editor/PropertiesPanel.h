#pragma once

#include "milo/scenes/Scene.h"
#include "milo/scenes/Entity.h"

namespace milo {

	class PropertiesPanel {
	private:
		Function<void, Material*> m_OnEditMaterialButtonClicked;
	public:
		PropertiesPanel();
		~PropertiesPanel();
	public:
		void render(Entity selected);
		void setOnEditMaterialButtonClicked(Function<void, Material*> callback);
	private:
		void handleAddComponents(Entity entity);
	private:
		static bool drawVector3Control(const String& label, Vector3& vector, float resetValue = 1.0f, float columnWidth = 100.0f);
	};

}