#pragma once
#include "VulkanMemory.h"
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"

namespace je
{
	struct VulkanApp;
	class Arena;

	class VulkanAllocator final
	{
	public:
		explicit VulkanAllocator(Arena& arena, const VulkanApp& app, size_t pageSize = 4096);
		~VulkanAllocator();

		[[nodiscard]] VulkanMemory Alloc(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, size_t count = 1) const;
		bool Free(const VulkanMemory& memory) const;

	private:
		struct Page final
		{
			VkDeviceMemory memory = VK_NULL_HANDLE;
			VkDeviceSize size = 0;
			VkDeviceSize remaining = 0;
			size_t alignment = 0;
		};

		struct Pool final
		{
			VkFlags memPropertyFlags;
			LinkedList<Page>* pages = nullptr;
		};

		Arena& _arena;
		const VulkanApp& _app;
		size_t _pageSize;
		Array<Pool>* _pools = nullptr;

		[[nodiscard]] size_t GetPoolId(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
	};
}
