#include "pch.h"
#include "Graphics/VkLayout.h"
#include "Graphics/VkApp.h"
#include "Jlb/Array.h"

namespace je::vk
{
	VkDescriptorSetLayout CreateLayout(Arena& tempArena, const App& app, const Array<Binding>& bindings)
	{
		const auto _ = tempArena.CreateScope();
		const auto sets = CreateArray<VkDescriptorSetLayoutBinding>(tempArena, bindings.length);
		for (size_t i = 0; i < bindings.length; ++i)
		{
			const auto& binding = bindings.data[i];
			auto& set = sets.data[i];

			set.stageFlags = binding.flag;
			set.binding = static_cast<uint32_t>(i);
			set.descriptorCount = binding.count;
			set.descriptorType = binding.type;
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.bindingCount = static_cast<uint32_t>(sets.length);
		layoutInfo.pBindings = sets.data;

		VkDescriptorSetLayout layout;
		const auto result = vkCreateDescriptorSetLayout(app.device, &layoutInfo, nullptr, &layout);
		assert(!result);
		return layout;
	}
}
