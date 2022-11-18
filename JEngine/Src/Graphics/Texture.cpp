#include "pch.h"
#include "Graphics/Texture.h"
#include "Algorithms/PackingFFDH.h"
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

			outfile << shape.x << std::endl;
			outfile << shape.y << std::endl;

			outfile << position.x << std::endl;
			outfile << position.y << std::endl;
		}

		outfile.close();
	}

	vk::Image LoadAtlas(vk::App& app, vk::Allocator& allocator, const char* imageFilePath, const char* metaFilePath)
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
		const auto image = vk::CreateImage(app, allocator, info, aPixels, glm::ivec3(texWidth, texHeight, 4));

		// Free pixels.
		stbi_image_free(pixels);
		return image;
	}
}
