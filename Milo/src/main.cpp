#include "milo/Milo.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"
#include <iostream>

using namespace milo;

int main() {

	milo::AppConfiguration config;
	config.applicationName = "Milo Engine";

	milo::Application app(config);

	milo::MiloExitResult exitResult = milo::MiloEngine::launch(app);

	return exitResult.exitCode;
}