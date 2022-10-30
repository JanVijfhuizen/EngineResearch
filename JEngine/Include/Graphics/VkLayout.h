#pragma once
#include "Jlb/View.h"

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		class VkLayout final
		{
		public:
			struct Binding final
			{
				enum class Type
				{
					buffer,
					storageBuffer,
					image
				} type;

				enum class Stage
				{
					vertex,
					fragment
				} stage;

				size_t size = sizeof(int32_t);
				uint32_t count = 1;
			};

			explicit VkLayout(App& app, Arena& tempArena, const View<Binding>& bindings);
			VkLayout(VkLayout&& other) noexcept;
			VkLayout& operator=(VkLayout&& other) noexcept;
			~VkLayout();

			[[nodiscard]] operator VkDescriptorSetLayout() const;

		private:
			VkDescriptorSetLayout _layout;
			App* _app = nullptr;
		};
	}
}