#include "pch.h"
#include "Graphics/VkLayout.h"
#include "Graphics/VkApp.h"
#include "Jlb/Array.h"

namespace je::vk
{
	VkLayout::VkLayout(App& app, Arena& tempArena, const View<Binding>& bindings) : _app(&app)
	{
		const Array<VkDescriptorSetLayoutBinding> sets{tempArena, bindings.GetLength()};
		for (size_t i = 0; i < bindings.GetLength(); ++i)
		{
			const auto& binding = bindings[i];
			auto& set = sets[i];

			set.binding = static_cast<uint32_t>(i);
			set.descriptorCount = binding.count;

			switch (binding.stage)
			{
			case Binding::Stage::vertex:
				set.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				break;
			case Binding::Stage::fragment:
				set.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			}

			switch (binding.type)
			{
				case Binding::Type::buffer:
					set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;
				case Binding::Type::storageBuffer:
					set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					break;
				case Binding::Type::image:
					set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					break;
			}
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.bindingCount = static_cast<uint32_t>(sets.GetLength());
		layoutInfo.pBindings = sets.GetData();
		
		const auto result = vkCreateDescriptorSetLayout(app.device, &layoutInfo, nullptr, &_layout);
		assert(!result);
	}

	VkLayout::VkLayout(VkLayout&& other) noexcept : _app(other._app), _layout(other._layout)
	{
		other._app = nullptr;
	}

	VkLayout& VkLayout::operator=(VkLayout&& other) noexcept
	{
		_app = other._app;
		_layout = other._layout;
		other._app = nullptr;
		return *this;
	}

	VkLayout::~VkLayout()
	{
		if (!_app)
			return;

		vkDestroyDescriptorSetLayout(_app->device, _layout, nullptr);
	}

	VkLayout::operator VkDescriptorSetLayout() const
	{
		return _layout;
	}
}
