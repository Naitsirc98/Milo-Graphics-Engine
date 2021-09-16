#include "milo/assets/models/ModelUtils.h"

namespace milo {

	Entity ModelUtils::createModelEntityTree(Scene* scene, Model* model) {
		Entity entity = scene->createEntity(model->name());
		createEntityModelTree(scene, model, model->root(), entity);
		return entity;
	}

	void ModelUtils::createEntityModelTree(Scene* scene, Model* model, const Model::Node* node, Entity entity) {

		Transform& transform = entity.getComponent<Transform>();
		transform.setMatrix(node->transform);

		if(node->mesh != nullptr) {
			MeshView& meshView = entity.addComponent<MeshView>();
			meshView.mesh = node->mesh;
			meshView.material = node->material;
		}

		for(uint32_t i = 0;i < node->children.size();++i) {
			const Model::Node* childNode = model->get(node->children[i]);
			Entity childEntity = scene->createEntity(childNode->name);
			entity.addChild(childEntity.id());
			createEntityModelTree(scene, model, childNode, childEntity);
		}
	}
}