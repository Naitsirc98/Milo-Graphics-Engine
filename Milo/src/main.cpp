#include "milo/Milo.h"
#include <iostream>

using namespace milo;

class MiloApplication : public Application {
public:

	MiloApplication(const AppConfiguration& config) : Application(config) {}

	void onStart() override {

		Mesh* mesh = Assets::meshes().getCube();
		Material* material = Assets::materials().load("Plastic", "resources/materials/Plastic/M_Plastic.mat");

		Scene* scene = SceneManager::activeScene();

		Entity entity = scene->createEntity();

		struct MyScript : public NativeScript {

			void onUpdate(EntityId entityId) override {
				//Log::debug("OnUpdate");
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