#pragma once
#include "VkImage.h"
#include "Jlb/Array.h"

namespace je::texture
{
	void GenerateAtlas(Arena& arena, Arena& tempArena, const Array<const char*>& filePaths, const char* imageFilePath, const char* metaFilePath);
	[[nodiscard]] vk::Image LoadAtlas(const vk::App& app, const vk::Allocator& allocator, const char* imageFilePath);
	[[nodiscard]] Array<glm::ivec2> LoadAtlasCoordinates(Arena& arena, const char* metaFilePath);
}
