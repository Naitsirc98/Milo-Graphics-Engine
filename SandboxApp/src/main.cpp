#include <iostream>
#include "milo/Milo.h"

using namespace milo;
using namespace milo::math;

int main()
{
	float d = Random::nextFloat();
	
	Log::debug("Hello, {}", Random::nextInt(0, 10));

	return 0;
}