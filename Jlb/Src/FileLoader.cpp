#include "jpch.h"
#include "Jlb/FileLoader.h"
#include <fstream>

#include "Jlb/JMove.h"

namespace je::file
{
	Array<char> Load(Arena& arena, const StringView& path)
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

	Array<size_t> Subdivide(Arena& arena, const char* data, const size_t length, const char d)
	{
		size_t lineCount = 0;
		
		for (size_t i = 0; i < length; ++i)
			lineCount += data[i] == d;

		Array<size_t> lines{arena, lineCount};
		
		size_t index = 0;
		for (size_t i = 0; i < length; ++i)
		{
			const auto& c = data[i];

			if (c == d)
				lines[++index] = i + 1;
		}
		return Move(lines);
	}
}
