#pragma once
#include "Jlb/View.h"

namespace je
{
	struct StringView;
}

namespace je::vk
{
	class Allocator;
	struct App;

	class Image final
	{
	public:
		Image(const App& app, const Allocator& allocator, const View<unsigned char>& pixels, glm::ivec3 resolution, 
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkImageAspectFlagBits flag = VK_IMAGE_ASPECT_COLOR_BIT);
		Image(const App& app, const Allocator& allocator, const StringView& path, 
			VkImageAspectFlagBits flag = VK_IMAGE_ASPECT_COLOR_BIT);
		~Image();

		void TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkImageAspectFlags aspectFlags);
		[[nodiscard]] glm::ivec3 GetResolution() const;
		[[nodiscard]] VkFormat GetFormat() const;
		[[nodiscard]] operator VkImage() const;

	private:
		const App* _app = nullptr;
		VkImage _image;
		VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFormat _format;
		VkImageAspectFlagBits _flag;
		glm::ivec3 _resolution;

		void Load(const App& app, const Allocator& allocator, const View<unsigned char>& pixels, glm::ivec2 resolution);
	};
}
