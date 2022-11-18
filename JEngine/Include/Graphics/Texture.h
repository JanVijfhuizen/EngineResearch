#pragma once
#include "VkImage.h"
#include "Jlb/Array.h"

namespace je::texture
{
	void GenerateAtlas(Arena& arena, Arena& tempArena, const Array<const char*>& filePaths, const char* imageFilePath, const char* metaFilePath);
	[[nodiscard]] vk::Image LoadAtlas(vk::App& app, vk::Allocator& allocator, const char* imageFilePath, const char* metaFilePath);
}
