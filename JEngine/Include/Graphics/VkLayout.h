#pragma once
#include "Jlb/View.h"

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		class Layout final
		{
		public:
			struct Binding final
			{
				VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				size_t size = sizeof(int32_t);
				uint32_t count = 1;
				VkShaderStageFlagBits flag;
			};

			explicit Layout(App& app, Arena& tempArena, const View<Binding>& bindings);
			Layout(Layout&& other) noexcept;
			Layout& operator=(Layout&& other) noexcept;
			~Layout();

			[[nodiscard]] operator VkDescriptorSetLayout() const;

		private:
			VkDescriptorSetLayout _layout;
			App* _app = nullptr;
		};
	}
}