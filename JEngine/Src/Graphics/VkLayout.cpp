#include "pch.h"
#include "Graphics/VkLayout.h"
#include "Graphics/VkApp.h"
#include "Jlb/Array.h"

namespace je::vk
{
	Layout::Layout(App& app, Arena& tempArena, const Array<Binding>& bindings) : _app(&app)
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
		
		const auto result = vkCreateDescriptorSetLayout(app.device, &layoutInfo, nullptr, &_layout);
		assert(!result);
	}

	Layout::Layout(Layout&& other) noexcept : _layout(other._layout), _app(other._app)
	{
		other._app = nullptr;
	}

	Layout& Layout::operator=(Layout&& other) noexcept
	{
		_app = other._app;
		_layout = other._layout;
		other._app = nullptr;
		return *this;
	}

	Layout::~Layout()
	{
		if (!_app)
			return;

		vkDestroyDescriptorSetLayout(_app->device, _layout, nullptr);
	}

	Layout::operator VkDescriptorSetLayout() const
	{
		return _layout;
	}
}
