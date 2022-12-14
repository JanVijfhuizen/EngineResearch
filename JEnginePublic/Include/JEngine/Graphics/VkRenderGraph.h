#pragma once
#include "JEngine/Graphics/VkRenderNode.h"
#include "JEngine/Graphics/VkImage.h"
#include "JEngine/Graphics/VkPipeline.h"
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"
#include "Jlb/Queue.h"

namespace je::vk
{
	class Allocator;
	class SwapChain;
	struct App;

	// Tool to simplify and abstract render steps like post effects, deferred lighting and more.
	class RenderGraph final
	{
	public:
		RenderGraph(Arena& arena, Arena& tempArena, App& app, Allocator& allocator, SwapChain& swapChain, const Array<RenderNode>& nodes);
		~RenderGraph();

		// Goes through the render graph nodes and renders them.
		[[nodiscard]] VkSemaphore Update() const;
		// Returns the image view for the final image of the render graph.
		[[nodiscard]] VkImageView GetResult(size_t frameIndex) const;

	private:
		struct TempResource final
		{
			struct Variation final
			{
				const char* name;
				size_t lifeTimeStart = 0;
				size_t lifeTimeEnd = 0;
				size_t imageIndex = SIZE_MAX;
			};

			RenderNode::Resource resource{};
			LinkedList<Variation> variations{};
			Queue<size_t> imageQueue{};
			size_t count = 0;
			size_t lifeTimeEnd = 0;
		};

		struct TempNode final
		{
			const RenderNode* renderNode = nullptr;

			size_t index = SIZE_MAX;
			size_t depth = 0;
			bool isRoot = true;

			LinkedList<TempNode*> children{};
			LinkedList<TempResource*> inputResources{};
			LinkedList<TempResource*> outputResources{};
			LinkedList<TempResource::Variation*> inputResourceVariations{};
			LinkedList<TempResource::Variation*> outputResourceVariations{};
		};

		struct Node final
		{
			RenderFunc renderFunc = nullptr;
			void* userPtr = nullptr;
			VkRenderPass renderPass;
			Array<VkFramebuffer> frameBuffers{};
			Pipeline pipeline{};
			glm::ivec2 resolution;
			size_t inputCount = 0;
			size_t outputCount = 0;
		};

		struct Layer final
		{
			struct Frame final
			{
				VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
				VkSemaphore semaphore = VK_NULL_HANDLE;
			};

			size_t index;
			Array<Frame> frames{};
		};

		struct Attachment final
		{
			Image image{};
			VkImageView view = VK_NULL_HANDLE;
		};

		Arena& _arena;
		App& _app;
		Allocator& _allocator;
		SwapChain& _swapChain;

		Array<size_t> _attachmentIndexes{};
		Array<Attachment> _attachments{};
		Array<Node> _nodes{};
		Array<Layer> _layers{};

		static void DefineDepth(TempNode& node, size_t depth);
		static bool SortDepthNodes(TempNode*& a, TempNode*& b);
	};
}
