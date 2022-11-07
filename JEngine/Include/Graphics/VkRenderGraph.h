﻿#pragma once
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"
#include "Jlb/Queue.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

namespace je::vk
{
	class Pipeline;
	class Shader;
	class Image;
	class Allocator;
	class SwapChain;
	struct App;

	struct RenderNode final
	{
		struct Resource final
		{
			glm::ivec2 resolution;
			[[nodiscard]] bool operator==(const Resource& other) const;
		};

		struct Output final
		{
			StringView name;
			Resource resource;
		};

		View<StringView> inputs{};
		View<Output> outputs{};
		View<VkDescriptorSetLayout> layouts{};
		Shader* shader = nullptr;
		
		void (*renderFunc)(VkCommandBuffer cmdBuffer, void* userPtr) = nullptr;
		void* userPtr = nullptr;
	};

	class RenderGraph final
	{
	public:
		struct Result final
		{
			VkImageLayout layout;
			VkImageView view;
		};

		RenderGraph(Arena& arena, Arena& tempArena, App& app, Allocator& allocator, SwapChain& swapChain, const View<RenderNode>& nodes);
		~RenderGraph();

		[[nodiscard]] VkSemaphore Update() const;
		[[nodiscard]] Result GetResult(size_t frameIndex) const;

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
			void (*renderFunc)(VkCommandBuffer cmdBuffer, void* userPtr) = nullptr;
			void* userPtr = nullptr;
			VkRenderPass renderPass;
			Array<VkFramebuffer>* frameBuffers = nullptr;
			Pipeline* pipeline = nullptr;
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
