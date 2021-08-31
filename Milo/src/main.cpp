#include "milo/Milo.h"
#include <iostream>

using namespace milo;

class MiloApplication : public Application {
public:

	MiloApplication(const AppConfiguration& config) : Application(config) {}

	void onStart() override {

		Scene* scene = SceneManager::activeScene();

		Skybox* skybox = Assets::skybox().getDefault();

		scene->setSkybox(skybox);

		createSphere(scene, {0, 0, -3}, Assets::materials().getDefault());
		createSphere(scene, {3, 0, -3}, Assets::materials().load("Plastic", "resources/materials/Plastic/M_Plastic.mat"));

		Entity camera = scene->createEntity();
		camera.createComponent<Camera>();
		auto& camScript = camera.createComponent<NativeScriptView>();
		camScript.bind<CameraController>();

		scene->setMainCamera(camera.id());
	}

	void createSphere(Scene* scene, const Vector3& position, Material* material) {

		Mesh* mesh = Assets::meshes().getSphere();

		Entity entity = scene->createEntity();

		MeshView& meshView = entity.createComponent<MeshView>();
		meshView.mesh = mesh;
		meshView.material = material;

		Transform& transform = entity.getComponent<Transform>();
		transform.translation = position;
	}

};

int main() {

	AppConfiguration config;
	config.applicationName = "Milo Engine";

	MiloApplication app(config);

	MiloExitResult exitResult = MiloEngine::launch(app);

	return exitResult.exitCode;
}