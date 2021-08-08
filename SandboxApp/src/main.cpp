#include <iostream>
#include "milo/Milo.h"
#include "milo/io/Files.h"

using namespace milo;

int main()
{
	Log::debug("Hello, {}", Random::nextInt(0, 10));

	const ArrayList<String>& lines = Files::readAllLines("file.txt");
	for(const String& line : lines) {
		Log::info(line);
	}

	return 0;
}