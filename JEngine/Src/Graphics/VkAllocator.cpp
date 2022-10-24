#include "pch.h"
#include "Graphics/VkAllocator.h"
#include "Graphics/VkApp.h"
#include "Jlb/JMath.h"

namespace je::vk
{
	Allocator::Allocator(Arena& arena, const App& app, const size_t pageSize) : _arena(arena), _app(app), _pageSize(pageSize)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(app.physicalDevice, &memProperties);
		_pools = Array<Pool>(arena, memProperties.memoryTypeCount);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			auto& pool = _pools[i];
			const auto& memType = memProperties.memoryTypes[i];
			pool.memPropertyFlags = memType.propertyFlags;
			pool.pages = arena.New<LinkedList<Page>>(1, arena);
		}
	}

	Allocator::~Allocator()
	{
		for (int32_t i = static_cast<int32_t>(_pools.GetLength()) - 1; i >= 0; --i)
		{
			const auto& pool = _pools[i];
			_arena.Free(pool.pages);
		}
		_arena.Free(_pools);
	}

	VkDeviceSize CalculateBufferSize(const VkDeviceSize size, const VkDeviceSize alignment)
	{
		return (size / alignment + (size % alignment > 0)) * alignment;
	}

	Memory Allocator::Alloc(const VkMemoryRequirements memRequirements, const VkMemoryPropertyFlags properties, const size_t count) const
	{
		const size_t poolId = GetPoolId(memRequirements.memoryTypeBits, properties);
		assert(poolId != SIZE_MAX);

		const size_t size = CalculateBufferSize(memRequirements.size * count, memRequirements.alignment);
		Page* ptr = nullptr;

		const auto& pool = _pools[poolId];
		for (auto& page : *pool.pages)
			if (page.remaining >= size && page.alignment == memRequirements.alignment)
			{
				ptr = &page;
				break;
			}

		// If no valid page exists, create one.
		if(!ptr)
		{
			auto& page = pool.pages->Add();
			ptr = &page;
			page.size = math::Max(size, _pageSize);
			page.remaining = page.size;
			page.alignment = memRequirements.alignment;

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = math::Max(size, _pageSize);
			allocInfo.memoryTypeIndex = static_cast<uint32_t>(poolId);

			const auto result = vkAllocateMemory(_app.device, &allocInfo, nullptr, &page.memory);
			assert(!result);
		}

		const VkDeviceSize offset = ptr->size - ptr->remaining;
		ptr->remaining -= size;

		Memory memory{};
		memory.memory = ptr->memory;
		memory.size = size;
		memory.offset = offset;
		memory.poolId = poolId;
		return memory;
	}

	bool Allocator::Free(const Memory& memory) const
	{
		const auto& pool = _pools[memory.poolId];
		for (auto& page : *pool.pages)
		{
			if (page.memory != memory.memory)
				continue;
			assert(page.size - page.remaining == memory.offset + memory.size);
			page.remaining = page.size - memory.offset;
			return true;
		}
		return false;
	}

	size_t Allocator::GetPoolId(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const
	{
		size_t id = 0;
		for (const auto& pool : _pools.GetView())
		{
			if (typeFilter & 1 << id)
				if ((pool.memPropertyFlags & properties) == properties)
					return id;
			++id;
		}

		return SIZE_MAX;
	}
}
