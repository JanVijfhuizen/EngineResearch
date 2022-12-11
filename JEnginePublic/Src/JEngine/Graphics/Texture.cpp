#include "JEngine/pch.h"
#include "JEngine/Graphics/Texture.h"
#include "JEngine/Algorithms/PackingFFDH.h"
#include "JEngine/Graphics/VkImage.h"
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "JEngine/Graphics/SubTexture.h"

namespace je::texture
{
	void GenerateAtlas(Arena& arena, Arena& tempArena, const Array<const char*>& filePaths, 
		const char* imageFilePath, const char* metaFilePath)
	{
		const auto _ = tempArena.CreateScope();
		const auto shapes = CreateArray<glm::ivec2>(tempArena, filePaths.length);

		{
			size_t i = 0;
			for (const auto& path : filePaths)
			{
				// Load pixels.
				int texWidth, texHeight, texChannels;
				stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
				assert(pixels);
				assert(texChannels == 4);
				shapes[i++] = glm::ivec2(texWidth, texHeight);

				// Free pixels.
				stbi_image_free(pixels);
			}
		}

		glm::ivec2 area;
		const auto positions = packing::Pack(arena, tempArena, shapes, area);
		const auto atlasPixels = CreateArray<stbi_uc>(tempArena, static_cast<size_t>(area.x) * area.y * 4);

		const auto rowLength = static_cast<size_t>(area.x) * 4;

		for (size_t i = 0; i < filePaths.length; ++i)
		{
			const auto& path = filePaths[i];
			const auto& position = positions[i];

			// Load pixels.
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			assert(pixels);
			assert(texChannels == 4);

			const size_t width = static_cast<size_t>(texWidth) * texChannels;
			const size_t start = position.y * rowLength + static_cast<size_t>(position.x) * 4;
			for (int y = 0; y < texHeight; ++y)
				memcpy(&atlasPixels.data[start + rowLength * y], &pixels[static_cast<size_t>(texWidth) * 4 * y], width);

			// Free pixels.
			stbi_image_free(pixels);
		}

		stbi_write_png(imageFilePath, area.x, area.y, 4, atlasPixels.data, 0);

		std::ofstream outfile;
		outfile.open(metaFilePath);

		for (size_t i = 0; i < filePaths.length; ++i)
		{
			const auto& shape = shapes[i];
			const auto& position = positions[i];

			const auto lTop = glm::vec2(position) / glm::vec2(area);
			const auto rBot = lTop + glm::vec2(shape) / glm::vec2(area);

			outfile << lTop.x << std::endl;
			outfile << lTop.y << std::endl;

			outfile << rBot.x << std::endl;
			outfile << rBot.y << std::endl;
		}

		outfile.close();
	}
	
	vk::Image Load(const vk::App& app, const vk::Allocator& allocator, const char* imageFilePath)
	{
		// Load pixels.
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(imageFilePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		assert(pixels);
		assert(texChannels == 4);

		Array<unsigned char> aPixels{};
		aPixels.length = static_cast<size_t>(texWidth) * texHeight * texChannels;
		aPixels.data = pixels;

		constexpr vk::ImageCreateInfo info{};
		const auto image = CreateImage(app, allocator, info, aPixels, glm::ivec3(texWidth, texHeight, 4));

		// Free pixels.
		stbi_image_free(pixels);

		return image;
	}

	Array<SubTexture> LoadAtlasMetaData(Arena& arena, const char* metaFilePath)
	{
		std::ifstream inFile;
		inFile.open(metaFilePath);

		const size_t lineCount = std::count(
			std::istreambuf_iterator(inFile), std::istreambuf_iterator<char>(), '\n');
		inFile.seekg(0, std::ios::beg);

		const auto metaData = CreateArray<SubTexture>(arena, lineCount / 4);

		glm::vec2 lTop;
		glm::vec2 rBot;
		size_t i = 0;
		while (inFile >> lTop.x >> lTop.y >> rBot.x >> rBot.y)
		{
			auto& instance = metaData[i++];
			instance.lTop = lTop;
			instance.rBot = rBot;
		}

		return metaData;
	}
}
