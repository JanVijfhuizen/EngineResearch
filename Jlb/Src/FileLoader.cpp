#include "Jlb/pch.h"
#include "Jlb/FileLoader.h"
#include <fstream>

namespace je::file
{
	Array<char> Load(Arena& arena, const char* path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		assert(file.is_open());

		// Dump contents in the buffer.
		const size_t fileSize = file.tellg();
		const auto buffer = CreateArray<char>(arena, fileSize);

		file.seekg(0);
		file.read(buffer.data, static_cast<std::streamsize>(fileSize));
		file.close();

		return buffer;
	}
}
