#include "milo/Milo.h"
#include <iostream>

using namespace milo;

class MiloApplication : public Application {
public:

	MiloApplication(const AppConfiguration& config) : Application(config) {}

	void onStart() override {

		Scene* scene = SceneManager::activeScene();

		Entity entity = scene->createEntity();

		struct MyScript : public NativeScript {

			void onCreate(EntityId entityId) override {
				Log::debug("OnCreate");
			}

			void onUpdate(EntityId entityId) override {
				Log::debug("OnUpdate");
			}

			void onLateUpdate(EntityId entityId) override {
				Log::debug("OnLateUpdate");
			}
		};

		NativeScriptView& script = entity.createComponent<NativeScriptView>();
		script.bind<MyScript>();
	}

};

int main() {

	AppConfiguration config;
	config.applicationName = "Milo Engine";

	MiloApplication app(config);

	MiloExitResult exitResult = MiloEngine::launch(app);

	return exitResult.exitCode;
}