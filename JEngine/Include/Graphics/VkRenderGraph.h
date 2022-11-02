#pragma once
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"
#include "Jlb/Pool.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

namespace je::vk
{
	class Allocator;
	class Image;
	class SwapChain;
	struct App;

	class RenderNode
	{
		friend class RenderGraph;

	protected:
		struct Resource final
		{
			glm::ivec3 resolution;
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
			VkImageAspectFlagBits flag = VK_IMAGE_ASPECT_COLOR_BIT;
			VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			[[nodiscard]] bool operator==(const Resource& other) const;
		};

		struct Output final
		{
			StringView name;
			Resource resource;
		};

		virtual void Render(VkCommandBuffer cmdBuffer) = 0;
		[[nodiscard]] virtual Array<StringView> DefineInputs(Arena& arena) const;
		[[nodiscard]] virtual Array<Output> DefineOutputs(Arena& arena) const;
	};

	class RenderGraph final
	{
	public:
		RenderGraph(Arena& arena, Arena& tempArena, App& app, Allocator& allocator, SwapChain& swapChain, const View<RenderNode*>& nodes);
		~RenderGraph();

		[[nodiscard]] VkSemaphore Update() const;

	private:
		struct TempNode final
		{
			size_t index = SIZE_MAX;
			size_t depth = 0;
			bool isRoot = true;
			RenderNode* node = nullptr;
			LinkedList<TempNode*> children{};
			Array<StringView> inputs{};
			Array<RenderNode::Output> outputs{};
		};

		struct TempResource final
		{
			RenderNode::Resource resource{};
			size_t parallelUsages = 0;
			LinkedList<TempNode*>* users = nullptr;
		};

		struct Layer final
		{
			struct Frame final
			{
				VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
				VkSemaphore semaphore = VK_NULL_HANDLE;
			};

			size_t index;
			Array<Frame>* frames = nullptr;
		};

		struct Resource final
		{
			struct Frame final
			{
				Pool<Image*>* images = nullptr;
			};

			Array<Frame>* frames = nullptr;
		};

		Arena& _arena;
		App& _app;
		Allocator& _allocator;
		SwapChain& _swapChain;

		Array<RenderNode*> _nodes{};
		Array<Layer> _layers{};
		Array<Image*> _images{};
		Array<Resource> _resources{};

		static void DefineDepth(TempNode& node, size_t depth);
		static bool SortDepthNodes(TempNode*& a, TempNode*& b);
		static bool SortResourceUsers(TempNode*& a, TempNode*& b);
	};
}
