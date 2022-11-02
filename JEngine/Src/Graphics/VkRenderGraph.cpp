#include "pch.h"
#include "Graphics/VkRenderGraph.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkImage.h"
#include "Graphics/VkSwapChain.h"
#include "Jlb/JMath.h"
#include "Jlb/LinSort.h"

namespace je::vk
{
	bool RenderNode::Resource::operator==(const Resource& other) const
	{
		return resolution == other.resolution && format == other.format && flag == other.flag && usageFlags == other.usageFlags;
	}

	Array<StringView> RenderNode::DefineInputs(Arena& arena) const
	{
		return {};
	}

	Array<RenderNode::Output> RenderNode::DefineOutputs(Arena& arena) const
	{
		return {};
	}

	RenderGraph::RenderGraph(Arena& arena, Arena& tempArena, App& app, Allocator& allocator, SwapChain& swapChain, const View<RenderNode*>& nodes) :
		_arena(arena), _app(app), _allocator(allocator), _swapChain(swapChain)
	{
		const auto _ = tempArena.CreateScope();

		const size_t length = nodes.GetLength();
		const size_t frameCount = swapChain.GetLength();
		const Array<TempNode> tempNodes{tempArena, length };

		for (size_t i = 0; i < length; ++i)
		{
			const auto& node = nodes[i];
			auto& tempNode = tempNodes[i];

			tempNode.index = i;
			tempNode.node = node;
			tempNode.children = LinkedList<TempNode*>{tempArena};
			tempNode.inputs = node->DefineInputs(tempArena);
			tempNode.outputs = node->DefineOutputs(tempArena);
		}

		// Link parents by comparing inputs and outputs.
		const auto view = tempNodes.GetView();
		for (auto& tempNode : view)
		{
			const auto inputView = tempNode.inputs.GetView();
			if (!inputView)
				continue;

			for (auto& other : view)
			{
				if (&tempNode == &other)
					continue;

				const auto outputView = other.outputs.GetView();
				if (!outputView)
					continue;
				for (auto& input : inputView)
					for (auto& output : outputView)
						if (input == output.name)
						{
							tempNode.isRoot = false;
							other.children.Add(&tempNode);
							goto NEXT;
						}

			NEXT:
				continue;
			}
		}

		// Define depth of every node.
		for (auto& tempNode : view)
			if (tempNode.isRoot)
				DefineDepth(tempNode, 0);

		// Sort based on depth.
		Array<TempNode*> depthSorted{ tempArena, length };
		for (size_t i = 0; i < length; ++i)
		{
			auto& tempNode = tempNodes[i];
			depthSorted[i] = &tempNode;
		}
		LinSort(depthSorted.GetData(), depthSorted.GetLength(), SortDepthNodes);

		// Define nodes and sync structs for individual frames.
		_nodes = Array<RenderNode*>(arena, length);
		{
			size_t index = 0;
			for (auto& node : _nodes.GetView())
			{
				node = depthSorted[index]->node;
				++index;
			}
		}

		// Find the amount of batches.
		size_t batchCount = 0;
		for (auto& depthNode : depthSorted.GetView())
			batchCount = math::Max(batchCount, depthNode->depth);
		++batchCount;

		// Define batch indices.
		_layers = { arena, batchCount };
		{
			size_t idx = 0;
			for (auto& depthNode : depthSorted.GetView())
			{
				const size_t newBatch = depthNode->depth > idx;
				idx += newBatch;

				auto& layer = _layers[idx];
				++layer.index;
			}
		}


		// Find all different resource types.
		LinkedList<TempResource> tempResources{ tempArena };
		for (auto& tempNode : view)
		{
			const auto outputView = tempNode.outputs.GetView();
			if (!outputView)
				continue;

			for (auto& output : outputView)
			{
				bool contained = false;

				TempResource* resource = nullptr;

				for (auto& tempResource : tempResources)
					if (tempResource.resource == output.resource)
					{
						resource = &tempResource;
						contained = true;
						break;
					}

				if (!contained)
				{
					resource = &tempResources.Add();
					resource->resource = output.resource;
					resource->users = tempArena.New<LinkedList<TempNode*>>(1, tempArena);
				}

				resource->users->Add(&tempNode);
			}
		}

		// Sort users for resources.
		for (auto& tempResource : tempResources)
			tempResource.users->Sort(SortResourceUsers);

		// Find maximum parallel resource usage.
		for (auto& tempResource : tempResources)
		{
			size_t layer = 0;
			size_t usages = 0;
			size_t previousUsages = 0;

			for (auto& user : *tempResource.users)
			{
				if (user->depth != layer)
				{
					layer = user->depth;
					tempResource.parallelUsages = math::Max(tempResource.parallelUsages, usages + previousUsages);
					previousUsages = usages;
					usages = 0;
				}

				++usages;
			}

			tempResource.parallelUsages = math::Max(tempResource.parallelUsages, usages + previousUsages);
		}

		// Find total image count.
		size_t imageCount = 0;
		for (auto& tempResource : tempResources)
			imageCount += tempResource.parallelUsages;
		imageCount *= frameCount;
		_images = Array<Image*>(arena, imageCount);

		{
			Image::CreateInfo imageCreateInfo{};
			imageCreateInfo.app = &app;
			imageCreateInfo.allocator = &allocator;
			imageCreateInfo.layout = VK_IMAGE_LAYOUT_UNDEFINED;

			size_t index = 0;
			for (auto& tempResource : tempResources)
			{
				auto& resource = tempResource.resource;
				imageCreateInfo.resolution = resource.resolution;
				imageCreateInfo.format = resource.format;
				imageCreateInfo.flag = resource.flag;
				imageCreateInfo.usageFlags = resource.usageFlags;

				size_t amount = tempResource.parallelUsages * frameCount;
				for (size_t i = 0; i < amount; ++i)
				{
					auto& image = _images[i + index];
					image = arena.New<Image>(1, imageCreateInfo);
				}

				index += amount;
			}
		}

		// Create resources.
		_resources = Array<Resource>(arena, tempResources.GetCount());
		{
			size_t index = 0;
			for (auto& resource : _resources.GetView())
			{
				resource.frames = arena.New<Array<Resource::Frame>>(1, arena, frameCount);
				for (auto& frame : resource.frames->GetView())
				{
					frame.images = arena.New<Pool<Image*>>(1, arena, tempResources[index].parallelUsages);
				}

				++index;
			}
		}

		for (auto& layer : _layers.GetView())
		{
			layer.frames = arena.New<Array<Layer::Frame>>(1, arena, frameCount);

			for (auto& frame : layer.frames->GetView())
			{
				VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
				cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				cmdBufferAllocInfo.commandPool = app.commandPool;
				cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				cmdBufferAllocInfo.commandBufferCount = 1;

				auto result = vkAllocateCommandBuffers(app.device, &cmdBufferAllocInfo, &frame.cmdBuffer);
				assert(!result);

				VkSemaphoreCreateInfo semaphoreCreateInfo{};
				semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				result = vkCreateSemaphore(_app.device, &semaphoreCreateInfo, nullptr, &frame.semaphore);
				assert(!result);
			}
		}

		// ...
		// Create corresponding images and link them to the nodes.
	}

	RenderGraph::~RenderGraph()
	{
		for (int32_t i = static_cast<int32_t>(_layers.GetLength()) - 1; i >= 0; --i)
		{
			const auto& layer = _layers[i];
			for (const auto& frame : layer.frames->GetView())
				vkDestroySemaphore(_app.device, frame.semaphore, nullptr);
			_arena.Delete(layer.frames);
		}

		for (int32_t i = static_cast<int32_t>(_resources.GetLength()) - 1; i >= 0; --i)
		{
			const auto& resource = _resources[i];
			for (const auto& frame : resource.frames->GetView())
				_arena.Delete(frame.images);
			_arena.Delete(resource.frames);
		}

		for (int32_t i = static_cast<int32_t>(_images.GetLength()) - 1; i >= 0; --i)
			_arena.Delete(_images[i]);
	}

	VkSemaphore RenderGraph::Update() const
	{
		const size_t idx = _swapChain.GetIndex();

		VkSemaphore semaphore = VK_NULL_HANDLE;
		size_t current = 0;

		for (const auto& layer : _layers.GetView())
		{
			const auto& frame = layer.frames->GetView()[idx];

			VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkResetCommandBuffer(frame.cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
			vkBeginCommandBuffer(frame.cmdBuffer, &cmdBufferBeginInfo);
			
			while(current < layer.index)
			{
				_nodes[current]->Render(frame.cmdBuffer);
				++current;
			}

			auto result = vkEndCommandBuffer(frame.cmdBuffer);
			assert(!result);
			
			const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &frame.cmdBuffer;
			submitInfo.waitSemaphoreCount = semaphore != VK_NULL_HANDLE;
			submitInfo.pWaitSemaphores = &semaphore;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &frame.semaphore;
			submitInfo.pWaitDstStageMask = &waitStage;

			result = vkQueueSubmit(_app.queues[App::renderQueue], 1, &submitInfo, nullptr);
			assert(!result);

			semaphore = frame.semaphore;
		}

		return semaphore;
	}

	void RenderGraph::DefineDepth(TempNode& node, const size_t depth)
	{
		if (node.depth >= depth)
			return;

		node.depth = depth;
		for (const auto& child : node.children)
			DefineDepth(*child, depth + 1);
	}

	bool RenderGraph::SortDepthNodes(TempNode*& a, TempNode*& b)
	{
		return true;
	}

	bool RenderGraph::SortResourceUsers(TempNode*& a, TempNode*& b)
	{
		return a->depth < b->depth;
	}
}
