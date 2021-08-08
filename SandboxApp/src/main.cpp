#include <iostream>
#include "milo/Milo.h"

int main()
{
#ifdef _DEBUG
	std::cout << "Debug" << std::endl;
#else
	std::cout << "Release\n";
#endif

	std::cout << getPhysicalDeviceCount() << std::endl;

	return 0;
}