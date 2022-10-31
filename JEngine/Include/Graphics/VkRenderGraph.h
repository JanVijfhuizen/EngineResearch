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
		struct UpdateInfo final
		{
			VkCommandBuffer cmdBuffer;
			View<VkSemaphore> waitSemaphores;
			VkSemaphore renderFinishedSemaphore;
		};

		struct Output final
		{
			StringView name;
		};

		virtual void Render(const UpdateInfo& info) = 0;
		[[nodiscard]] virtual Array<StringView> DefineInputs(Arena& arena) const;
		[[nodiscard]] virtual Array<Output> DefineOutputs(Arena& arena) const;
	};

	class RenderGraph final
	{
	public:
		RenderGraph(App& app, Arena& arena, Arena& tempArena, SwapChain& swapChain, const View<RenderNode*>& nodes);
		~RenderGraph();

		[[nodiscard]] View<VkSemaphore> Update() const;

	private:
		struct TempNode final
		{
			size_t index = SIZE_MAX;
			bool isLeaf = true;
			const RenderNode* node = nullptr;
			LinkedList<TempNode*> parents{};
			Array<StringView> inputs{};
			Array<RenderNode::Output> outputs{};
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
	};
}
