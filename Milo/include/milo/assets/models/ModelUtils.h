#pragma once

#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "Model.h"

namespace milo::ModelUtils {

	Entity createModelEntityTree(Scene* scene, Model* model);
	void createEntityModelTree(Scene* scene, Model* model, const Model::Node* node, Entity entity);
}