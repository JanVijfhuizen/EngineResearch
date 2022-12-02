#pragma once
#include "VkMemory.h"
#include "Jlb/Array.h"

namespace je::vk
{
	class Allocator;
	struct App;

	// Image that can be used for things like textures or post effects.
	struct Image final
	{
		VkImage image;
		VkImageLayout layout;
		VkFormat format;
		VkImageAspectFlags aspectFlags;
		glm::ivec3 resolution;
		Memory memory;
	};

	struct ImageCreateInfo final
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	};

	[[nodiscard]] Image CreateImage(const App& app, const Allocator& allocator, const ImageCreateInfo& info, glm::ivec3 resolution);
	[[nodiscard]] Image CreateImage(const App& app, const Allocator& allocator, const ImageCreateInfo& info, const Array<unsigned char>& pixels, glm::ivec3 resolution);
	// Transition the layout for it to be used in different ways, like for a depth attachment, or a sampled image.
	void ImageTransitionLayout(Image& image, VkCommandBuffer cmd, VkImageLayout newLayout, VkImageAspectFlags aspectFlags);
	void DestroyImage(const Image& image, const App& app, const Allocator& allocator);
}
