#pragma once
#include "VkMemory.h"
#include "Jlb/StringView.h"
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
		struct CreateInfo final
		{
			App* app;
			Allocator* allocator;

			View<unsigned char> pixels{};
			glm::ivec3 resolution;
			StringView path{};

			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
			VkImageAspectFlagBits aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
			VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		};

		Image() = default;
		explicit Image(const CreateInfo& info);
		Image(Image&& other) noexcept;
		Image& operator=(Image&& other) noexcept;
		~Image();

		void TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkImageAspectFlags aspectFlags);
		[[nodiscard]] glm::ivec3 GetResolution() const;
		[[nodiscard]] VkFormat GetFormat() const;
		[[nodiscard]] VkImageLayout GetLayout() const;
		[[nodiscard]] operator VkImage() const;

	private:
		const App* _app = nullptr;
		Allocator* _allocator;
		VkImage _image;
		VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFormat _format;
		VkImageAspectFlagBits _flag;
		glm::ivec3 _resolution;
		Memory _memory;

		void Load(const View<unsigned char>& pixels, VkImageUsageFlags usageFlags, VkImageLayout layout);
		void CreateImage(VkImageUsageFlags usageFlags);

		void DeepCopy(Image&& other);
	};
}
