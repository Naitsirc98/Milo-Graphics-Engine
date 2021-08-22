#pragma once

#include "milo/common/Common.h"
#include <filesystem>
#include <fstream>

#define MILO_MAX_PATH_LENGTH 256

namespace milo {

	using Path = std::filesystem::path;
	using FileStream = std::fstream;
	using InputStream = std::ifstream;
	using OutputStream = std::ofstream;

	class Files {
	public:
		static bool exists(const String& filename);
		static void create(const String& filename);
		static void createDirectory(const String& filename);
		static uint64 length(const String& filename);
		static bool isDirectory(const String& path);
		static ArrayList<String> listFiles(const String& directory);
		static ArrayList<int8> readAllBytes(const String& filename);
		static String readAllText(const String& filename);
		static ArrayList<String> readAllLines(const String& filename, uint32_t numLinesAprox = 16);
		static void writeAllBytes(const String& filename, const ArrayList<int8>& bytes);
		static void writeAllBytes(const String& filename, const int8* bytes, uint32 size);
		static void writeAllText(const String& filename, const String& str);
		static void writeAllLines(const String& filename, const ArrayList<String>& lines);
	};
}