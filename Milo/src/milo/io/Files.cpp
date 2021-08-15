#include "milo/io/Files.h"

namespace milo {

	bool Files::exists(const String& filename) {
		return std::filesystem::exists(filename);
	}

	void Files::create(const String& filename) {
		Path path = Path(filename);
		std::filesystem::create_directories(path.parent_path());
		OutputStream output(filename);
	}

	void Files::createDirectory(const String& filename) {
		std::filesystem::create_directories(Path(filename));
	}

	uint64 Files::length(const String& filename) {
		if (!exists(filename)) return 0;
		InputStream inputStream(filename, InputStream ::ate | InputStream ::binary);
		return inputStream.tellg();
	}

	ArrayList<int8> Files::readAllBytes(const String& filename) {
		InputStream inputStream(filename, InputStream::binary | InputStream::ate);
		uint32 size = static_cast<uint32>(inputStream.tellg());
		ArrayList<int8> contents(size);
		inputStream.seekg(0, InputStream::beg);
		inputStream.read((char*)contents.data(), size);
		return contents;
	}

	String Files::readAllText(const String& filename) {
		InputStream inputStream(filename);
		if (!inputStream.is_open()) throw MILO_RUNTIME_EXCEPTION(String("Failed to open file ").append(filename));
		StringStream stringStream;
		stringStream << inputStream.rdbuf();
		return stringStream.str();
	}

	ArrayList<String> Files::readAllLines(const String& filename) {
		InputStream inputStream(filename);
		ArrayList<String> lines;
		lines.reserve(16);
		String line;
		while (getline(inputStream, line)) {
			lines.push_back(line);
		}
		return lines;
	}

	void Files::writeAllBytes(const String& filename, const ArrayList<int8>& bytes) {
		writeAllBytes(filename, bytes.data(), bytes.size());
	}

	void Files::writeAllBytes(const String& filename, const int8* bytes, uint32 size) {
		OutputStream outputStream(filename, OutputStream::binary);
		outputStream.write((char*)bytes, size);
	}

	void Files::writeAllText(const String& filename, const String& str) {
		OutputStream outputStream(filename);
		outputStream.write(str.c_str(), str.length());
	}

	void Files::writeAllLines(const String& filename, const ArrayList<String>& lines) {
		OutputStream outputStream(filename);
		for(const String& line : lines) {
			outputStream << line << '\n';
		}
	}
}