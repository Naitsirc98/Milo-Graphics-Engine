#include "milo/Milo.h"

int main() {

	milo::AppConfiguration config;
	config.applicationName = "Milo Engine";

	milo::Application app(config);

	milo::MiloExitResult exitResult = milo::MiloEngine::launch(app);

	return exitResult.exitCode;
}