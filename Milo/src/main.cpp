#include "milo/Milo.h"
#include "Example1.h"

using namespace milo;

int main() {

	AppConfiguration config;
	config.applicationName = "My Application";

	MyApplication app(config);

	MiloExitResult exitResult = MiloEngine::launch(app);

	return exitResult.exitCode;
}