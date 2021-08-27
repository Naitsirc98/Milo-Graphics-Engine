#include "milo/Milo.h"
#include <iostream>

using namespace milo;

class MiloApplication : public Application {
public:

	MiloApplication(const AppConfiguration& config) : Application(config) {}

	void onStart() override {

		Mesh* mesh = Assets::meshes().getSphere();
		Material* material = Assets::materials().load("Plastic", "resources/materials/Plastic/M_Plastic.mat");

		Scene* scene = SceneManager::activeScene();

		Entity entity = scene->createEntity();

		MeshView& meshView = entity.createComponent<MeshView>();
		meshView.mesh = mesh;
		meshView.material = material;

		Transform& transform = entity.getComponent<Transform>();
		transform.translation.z = 1;

		struct MyScript : public NativeScript {

			void onUpdate(EntityId entityId) override {
				//Entity entity = SceneManager::activeScene()->find(entityId);
				//entity.getComponent<Transform>().translation -= 0.1f;
			}
		};

		NativeScriptView& script = entity.createComponent<NativeScriptView>();
		script.bind<MyScript>();

		Entity camera = scene->createEntity();
		camera.createComponent<Camera>();
		auto& camScript = camera.createComponent<NativeScriptView>();
		camScript.bind<CameraController>();

		scene->setMainCamera(camera.id());
	}

};

int main() {

	AppConfiguration config;
	config.applicationName = "Milo Engine";

	MiloApplication app(config);

	MiloExitResult exitResult = MiloEngine::launch(app);

	return exitResult.exitCode;
}