#pragma once
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace vk::layout
	{
		struct App;

		struct Binding
		{
			VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			size_t size = sizeof(int32_t);
			uint32_t count = 1;
			VkShaderStageFlagBits flag;
		};

		[[nodiscard]] VkDescriptorSetLayout Create(Arena& tempArena, App& app, const Array<Binding>& bindings);
	}
}
