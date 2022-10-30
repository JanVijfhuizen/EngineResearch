#include "jpch.h"
#include "Jlb/FileLoader.h"
#include <fstream>

#include "Jlb/JMove.h"

namespace je::file
{
	Array<char> Load(Arena& arena, const StringView path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		assert(file.is_open());

		// Dump contents in the buffer.
		const size_t fileSize = file.tellg();
		Array<char> buffer{arena, fileSize};

		file.seekg(0);
		file.read(buffer.GetData(), static_cast<std::streamsize>(fileSize));
		file.close();
		return Move(buffer);
	}
}
