#pragma once

#include "milo/Milo.h"
#include <iostream>
#include "milo/assets/models/ModelUtils.h"

using namespace milo;

class MyApplication : public Application {
public:

	MyApplication(const AppConfiguration& config) : Application(config) {}

	void onStart() override {

		Scene* scene = SceneManager::activeScene();

		Skybox* skybox = Assets::skybox().getPreethamSky();

		Entity skyboxEntity = scene->createEntity("Skybox");
		SkyboxView& skyboxView = skyboxEntity.addComponent<SkyboxView>();
		skyboxView.skybox = skybox;
		skyboxView.type = SkyType::Dynamic;

		scene->setSkyEntity(skyboxEntity.id());

		{
			Entity floor = scene->createEntity("Floor");
			Transform& t = floor.getComponent<Transform>();
			t.translation({0, -8, 0});
			t.scale({100, 0.2f, 100});
			MeshView& meshView = floor.addComponent<MeshView>();
			meshView.mesh = Assets::meshes().getCube();
			meshView.material = Assets::materials().getDefault();
			meshView.castShadows = false;
		}

		ArrayList<Material*> materials;
		materials.push_back(Assets::materials().load("Plastic", "resources/materials/Plastic/M_Plastic.mat"));
		materials.push_back(Assets::materials().load("Gold", "resources/materials/Gold/M_Gold.mat"));
		materials.push_back(Assets::materials().load("Rusted Iron", "resources/materials/RustedIron/M_RustedIron.mat"));

		for(uint32_t i = 0;i < 1000;++i) {
			Vector3 pos = {Random::nextInt(0, 100), Random::nextInt(0, 100), Random::nextInt(0, 100)};
			uint32_t mat = Random::nextInt(0) % (uint32_t)materials.size();
			Entity s1 = createSphere(scene, pos, materials[mat], i + 1);
		}

		Entity light1 = createSphere(scene, {0, 0, 0}, Assets::materials().load("EmissiveWhite", "resources/materials/M_WhiteEmissive.mat"));
		light1.setName("Point Light 1");
		light1.getComponent<Transform>().scale({0.1f, 0.1f, 0.1f});
		light1.getComponent<Transform>().translation({0.0f, 0.0f, 1.0f});
		PointLight& p1 = light1.addComponent<PointLight>();
		p1.multiplier = 3;

		Entity light2 = createSphere(scene, {7, 3, 0}, Assets::materials().load("EmissiveWhite", "resources/materials/M_WhiteEmissive.mat"));
		light2.setName("Point Light 2");
		light2.getComponent<Transform>().scale({0.1f, 0.1f, 0.1f});
		light2.getComponent<Transform>().translation({0.0f, 1.0f, 0.0f});
		PointLight& p2 = light2.addComponent<PointLight>();
		p2.multiplier = 3;

		Entity sun = createSphere(scene, {0, 12, 0}, Assets::materials().getDefault());
		sun.setName("Sun");
		sun.getComponent<Transform>().scale({0.1f, 0.1f, 0.1f});
		DirectionalLight& dirLight = sun.addComponent<DirectionalLight>();
		dirLight.direction = {0.2f, 0.8f, 0};
		dirLight.color = {5, 5, 5};

		//ModelUtils::createModelEntityTree(scene, Assets::models().getSponza());

		Entity camera = scene->createEntity();
		camera.setName("Main Camera");
		camera.addComponent<Camera>();
		auto& camScript = camera.addComponent<NativeScriptView>();
		camScript.bind<CameraController>();

		scene->setMainCamera(camera.id());
	}

	Entity createSphere(Scene* scene, const Vector3& position, Material* material, uint32_t index = 0) {

		Mesh* mesh = Assets::meshes().getSphere();

		Entity entity = scene->createEntity();
		if(index != 0) {
			entity.setName("Object " + str(index));
		} else {
			entity.setName("Entity " + material->name());
		}

		MeshView& meshView = entity.addComponent<MeshView>();
		meshView.mesh = mesh;
		meshView.material = material;

		Transform& transform = entity.getComponent<Transform>();
		transform.translation(position);

		return entity;
	}

};