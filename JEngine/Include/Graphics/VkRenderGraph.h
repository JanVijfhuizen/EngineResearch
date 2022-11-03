#pragma once
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"
#include "Jlb/Queue.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

namespace je::vk
{
	class Image;
	class Allocator;
	class SwapChain;
	struct App;

	class RenderNode
	{
		friend class RenderGraph;

	protected:
		struct Resource final
		{
			glm::ivec3 resolution;

			enum class Type
			{
				color
			} type = Type::color;

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
		virtual void DefineRenderPass(const App& app, VkRenderPass& outRenderPass) = 0;
	};

	class RenderGraph final
	{
	public:
		RenderGraph(Arena& arena, Arena& tempArena, App& app, Allocator& allocator, SwapChain& swapChain, const View<RenderNode*>& nodes);
		~RenderGraph();

		[[nodiscard]] VkSemaphore Update() const;

	private:
		struct TempResource final
		{
			struct Variation final
			{
				StringView name{};
				size_t lifeTimeStart = 0;
				size_t lifeTimeEnd = 0;
				size_t imageIndex = SIZE_MAX;
			};

			RenderNode::Resource resource{};
			LinkedList<Variation>* variations = nullptr;
			Queue<size_t>* imageQueue = nullptr;
			size_t count = 0;
			size_t lifeTimeEnd = 0;
		};

		struct TempNode final
		{
			RenderNode* node = nullptr;

			size_t index = SIZE_MAX;
			size_t depth = 0;
			bool isRoot = true;

			LinkedList<TempNode*> children{};
			Array<StringView> inputs{};
			Array<RenderNode::Output> outputs{};
			LinkedList<TempResource*> inputResources{};
			LinkedList<TempResource*> outputResources{};
			LinkedList<TempResource::Variation*> inputResourceVariations{};
			LinkedList<TempResource::Variation*> outputResourceVariations{};
		};

		struct Node final
		{
			RenderNode* renderNode = nullptr;
			Array<VkFramebuffer>* frameBuffers = nullptr;
			VkRenderPass renderPass;
			glm::ivec3 resolution;
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
			Array<Frame>* frames = nullptr;
		};

		struct Attachment final
		{
			Image* image = nullptr;
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
