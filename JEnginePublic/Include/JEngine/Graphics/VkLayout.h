#pragma once
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		// Input value for a shader/pipeline.
		struct Binding final
		{
			VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			size_t size = sizeof(int32_t);
			uint32_t count = 1;
			VkShaderStageFlagBits flag = VK_SHADER_STAGE_ALL;
		};

		// Defines a layout for a shader pipeline. One pipeline can have up to four layouts, and layouts can be reused in multiple pipelines.
		[[nodiscard]] VkDescriptorSetLayout CreateLayout(Arena& tempArena, const App& app, const Array<Binding>& bindings);
	}
}
