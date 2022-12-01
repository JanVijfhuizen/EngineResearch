#pragma once
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		struct Binding final
		{
			VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			size_t size = sizeof(int32_t);
			uint32_t count = 1;
			VkShaderStageFlagBits flag = VK_SHADER_STAGE_ALL;
		};

		[[nodiscard]] VkDescriptorSetLayout CreateLayout(Arena& tempArena, const App& app, const Array<Binding>& bindings);
	}
}
