#include "pch.h"
#include "Graphics/ObjLoader.h"
#include <string>
#include "Jlb/FileLoader.h"
#include "Jlb/JMove.h"

namespace je::obj
{
	Arena::Scope Load(Arena& tempArena, const StringView& path, Array<vk::Vertex>& outVertices, Array<vk::Vertex::Index>& outIndices)
	{
		auto scope = tempArena.CreateScope();
		const auto data = file::Load(tempArena, path);
		const auto lines = file::Subdivide(tempArena, data.GetData(), data.GetLength(), '\n');

		const auto dataView = data.GetView();
		const auto lineView = lines.GetView();
		const size_t length = lines.GetLength();

		// Calculate the amount of vertices.
		size_t vertCount = 0;
		size_t prev = 0;
		for (int32_t i = static_cast<int32_t>(length) - 1; i >= 0; --i)
		{
			const auto& line = lineView[i];

			if (dataView[prev] != 'f')
				break;
			
			const size_t lineLength = line - prev;
			for (size_t j = 2; j < lineLength; ++j)
				vertCount += dataView[prev + j] == ' ';

			prev = line;
		}

		const size_t vertIndex = length - vertCount / 4;
		size_t index = 0;

		for (; index < vertIndex; ++index)
			if (lineView[index][0] == 'v')
				break;

		const size_t posIndex = index;

		for (; index < vertIndex; ++index)
			if (lineView[index][1] == 'n')
				break;

		const size_t normalIndex = index;

		for (; index < vertIndex; ++index)
			if (lineView[index][1] == 't')
				break;

		const size_t texCoordsIndex = index;
		
		outVertices = {tempArena, vertCount};
		outIndices = { tempArena, vertCount };

		const auto verticesView = outVertices.GetView();
		const auto indicesView = outIndices.GetView();

		for (size_t i = 0; i < vertCount; ++i)
			indicesView[i] = static_cast<uint16_t>(i);
		
		for (size_t i = 0; i < vertCount; ++i)
		{
			const auto _ = tempArena.CreateScope();

			auto& vertex = verticesView[i];
			const size_t w = 2 + i % 4 * 6;
			const size_t h = vertIndex + i / 4;
			const auto& line = lineView[h];

			const size_t pIndex = posIndex + static_cast<unsigned char>(line[w]);
			const size_t nIndex = normalIndex + static_cast<unsigned char>(line[w + 2]);
			const size_t tIndex = texCoordsIndex + static_cast<unsigned char>(line[w + 4]);

			const auto& strPos = lineView[pIndex];
			const auto& strNormal = lineView[nIndex];
			const auto& strTexCoords = lineView[tIndex];

			const auto pDiv = file::Subdivide(tempArena, strPos.GetData(), strPos.GetLength(), ' ');
			const auto nDiv = file::Subdivide(tempArena, strNormal.GetData(), strNormal.GetLength(), ' ');
			const auto tDiv = file::Subdivide(tempArena, strTexCoords.GetData(), strTexCoords.GetLength(), ' ');

			vertex.position = { std::stof(pDiv[0].GetData()), std::stof(pDiv[1].GetData()), std::stof(pDiv[2].GetData()) };
			vertex.normal = { std::stof(nDiv[0].GetData()), std::stof(nDiv[1].GetData()), std::stof(nDiv[2].GetData()) };
			vertex.textureCoordinates = { std::stof(tDiv[0].GetData()), std::stof(tDiv[1].GetData()) };
		}

		return Move(scope);
	}
}
