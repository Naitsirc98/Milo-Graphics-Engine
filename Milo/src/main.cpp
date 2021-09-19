#include "milo/Milo.h"
#include <iostream>
#include "milo/assets/models/ModelUtils.h"

using namespace milo;

class MiloApplication : public Application {
public:

	MiloApplication(const AppConfiguration& config) : Application(config) {}

	void onStart() override {

		Scene* scene = SceneManager::activeScene();

		Skybox* skybox = Assets::skybox().getIndoorSkybox();

		Entity skyboxEntity = scene->createEntity("Skybox");
		SkyboxView& skyboxView = skyboxEntity.addComponent<SkyboxView>();
		skyboxView.skybox = skybox;
		skyboxView.type = SkyType::Static;

		scene->setSkyEntity(skyboxEntity.id());

		Entity s1 = createSphere(scene, {-2.5f, 0, 0}, Assets::materials().load("Plastic", "resources/materials/Plastic/M_Plastic.mat"));
		Entity s2 = createSphere(scene, {2.5f, 0, 0}, Assets::materials().load("Gold", "resources/materials/Gold/M_Gold.mat"));

		Entity h = scene->find(ModelUtils::createModelEntityTree(scene, Assets::models().getDamagedHelmet()).children()[0]);
		h.getComponent<Transform>().translation({0, 5, -2});
		h.getComponent<Transform>().rotate(radians(45.0f), {1, 0, 0});

		//Entity light1 = createSphere(scene, {0, 0, 0}, Assets::materials().getDefault());
		//light1.setName("Point Light 1");
		//light1.getComponent<Transform>().scale({0.1f, 0.1f, 0.1f});
		//light1.addComponent<PointLight>();
//
		//Entity light2 = createSphere(scene, {-2.5f, 0, 0}, Assets::materials().getDefault());
		//light2.setName("Point Light 2");
		//light2.getComponent<Transform>().scale({0.1f, 0.1f, 0.1f});
		//light2.addComponent<PointLight>();

		Entity sun = createSphere(scene, {0, 20, 0}, Assets::materials().getDefault());
		sun.setName("Sun");
		sun.getComponent<Transform>().scale({0.1f, 0.1f, 0.1f});
		sun.addComponent<DirectionalLight>().direction = {0, 1, 0};

		s1.addChild(s2.id());

		//ModelUtils::createModelEntityTree(scene, Assets::models().getSponza());

		Entity camera = scene->createEntity();
		camera.setName("Main Camera");
		camera.addComponent<Camera>();
		auto& camScript = camera.addComponent<NativeScriptView>();
		camScript.bind<CameraController>();

		scene->setMainCamera(camera.id());
	}

	Entity createSphere(Scene* scene, const Vector3& position, Material* material) {

		Mesh* mesh = Assets::meshes().getSphere();

		Entity entity = scene->createEntity();
		entity.setName("Sphere " + material->name());

		MeshView& meshView = entity.addComponent<MeshView>();
		meshView.mesh = mesh;
		meshView.material = material;

		Transform& transform = entity.getComponent<Transform>();
		transform.translation(position);

		return entity;
	}

};

int main() {

	AppConfiguration config;
	config.applicationName = "Milo Engine";

	MiloApplication app(config);

	MiloExitResult exitResult = MiloEngine::launch(app);

	return exitResult.exitCode;
}