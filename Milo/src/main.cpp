#include "milo/Milo.h"

using namespace milo;

int main() {

	milo::AppConfiguration config;
	config.applicationName = "Milo Engine";

	milo::Application app(config);

	Tag tag = "";

	milo::MiloExitResult exitResult = milo::MiloEngine::launch(app);

	return exitResult.exitCode;
}