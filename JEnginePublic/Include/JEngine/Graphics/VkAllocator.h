#pragma once
#include "VkMemory.h"
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		// Stack allocator for vulkan-gpu memory.
		class Allocator final
		{
		public:
			explicit Allocator(Arena& arena, const App& app, size_t pageSize = 4096);
			~Allocator();

			// Allocate a chunk of memory based on the given properties.
			[[nodiscard]] Memory Alloc(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, size_t count = 1) const;
			// Only works if the allocated memory was the last one allocated.
			[[nodiscard]] bool Free(const Memory& memory) const;

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
				LinkedList<Page> pages{};
			};

			Arena& _arena;
			const App& _app;
			size_t _pageSize;
			Array<Pool> _pools{};

			[[nodiscard]] size_t GetPoolId(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		};
	}
}
