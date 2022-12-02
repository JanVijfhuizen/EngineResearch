#pragma once
#include "VkImage.h"
#include "Jlb/Array.h"

namespace je::texture
{
	// Generate a single texture from multiple smaller ones.
	void GenerateAtlas(Arena& arena, Arena& tempArena, const Array<const char*>& filePaths, const char* imageFilePath, const char* metaFilePath);
	[[nodiscard]] vk::Image Load(const vk::App& app, const vk::Allocator& allocator, const char* imageFilePath);
	// Load coordinates that correspond with a texture atlas.
	[[nodiscard]] Array<glm::ivec2> LoadAtlasCoordinates(Arena& arena, const char* metaFilePath);
}
