#include "milo/Milo.h"

using namespace milo;

struct A {
	int x;
	virtual ~A() {}
};

struct B : public A {

};

int main() {

	milo::AppConfiguration config;
	config.applicationName = "Milo Engine";

	milo::Application app(config);

	milo::MiloExitResult exitResult = milo::MiloEngine::launch(app);

	return exitResult.exitCode;
}