#pragma once
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

namespace je::vk
{
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
		RenderGraph(App& app, Arena& arena, Arena& tempArena, SwapChain& swapChain, const View<RenderNode*>& nodes);
		~RenderGraph();

		[[nodiscard]] View<VkSemaphore> Update(Arena& tempArena) const;

	private:
		struct TempNode final
		{
			size_t index = SIZE_MAX;
			bool isLeaf = true;
			size_t depth = 0;
			const RenderNode* node = nullptr;
			LinkedList<TempNode*> parents{};
			LinkedList<TempNode*> children{};
			Array<StringView> inputs{};
			Array<RenderNode::Output> outputs{};
		};

		struct TempResource final
		{
			RenderNode::Resource resource;
			size_t parallelUsages = 0;
		};

		struct Node final
		{
			struct Frame final
			{
				VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
				VkSemaphore semaphore = VK_NULL_HANDLE;
				Array<VkSemaphore>* waitSemaphores = nullptr;
			};

			RenderNode* target = nullptr;
			Array<Frame>* frames;
		};

		App& _app;
		Arena& _arena;
		SwapChain& _swapChain;

		Array<Node> _nodes{};
		Array<Array<VkSemaphore>*> _output{};

		static void DefineDepth(TempNode& node, size_t depth);
		static bool SortDepthNodes(TempNode*& a, TempNode*& b);
	};
}
