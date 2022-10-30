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
		Image(const App& app, const Allocator& allocator, const View<unsigned char>& pixels, glm::ivec2 resolution);
		Image(const App& app, const Allocator& allocator, const StringView& path);
		~Image();

		void TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkImageAspectFlags aspectFlags);

	private:
		const App* _app = nullptr;
		VkImage _image;
		VkImageView _view;
		VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;

		void Load(const App& app, const Allocator& allocator, const View<unsigned char>& pixels, glm::ivec2 resolution);
	};
}
