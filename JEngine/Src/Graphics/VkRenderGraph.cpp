﻿#include "pch.h"
#include "Graphics/VkRenderGraph.h"
#include "Graphics/VkApp.h"
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

	RenderGraph::RenderGraph(App& app, Arena& arena, Arena& tempArena, SwapChain& swapChain, const View<RenderNode*>& nodes) :
		_app(app), _arena(arena), _swapChain(swapChain)
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
			tempNode.parents = LinkedList<TempNode*>{tempArena};
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
							tempNode.parents.Add(&other);
							other.children.Add(&tempNode);
							other.isLeaf = false;
							goto NEXT;
						}

			NEXT:
				continue;
			}
		}

		// Define nodes and sync structs for individual frames.
		_nodes = Array<Node>(arena, length);

		{
			size_t index = 0;

			for (auto& node : _nodes.GetView())
			{
				node.target = nodes[index];

				node.frames = arena.New<Array<Node::Frame>>(1, arena, frameCount);
				for (auto& frame : node.frames->GetView())
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

					auto& tempNode = tempNodes[index];
					const size_t waitSemaphoresCount = tempNode.parents.GetCount();
					frame.waitSemaphores = arena.New<Array<VkSemaphore>>(1, arena, waitSemaphoresCount);
				}
				++index;
			}
		}

		// Define depth of every node.
		for (auto& tempNode : view)
		{
			if (tempNode.parents.GetCount() > 0)
				continue;
			DefineDepth(tempNode, 0);
		}

		// Sort based on depth.
		Array<TempNode*> depthSorted{ tempArena, length };
		for (size_t i = 0; i < length; ++i)
		{
			auto& tempNode = tempNodes[i];
			depthSorted[i] = &tempNode;
		}
		LinSort(depthSorted.GetData(), depthSorted.GetLength(), SortDepthNodes);

		// Find the amount of batches.
		size_t batchCount = 0;
		for (auto& depthNode : depthSorted.GetView())
			batchCount = math::Max(batchCount, depthNode->depth);
		++batchCount;

		// Define batch indices.
		Array<size_t> indices{ tempArena, batchCount };
		{
			size_t idx = 0;
			for (auto& depthNode : depthSorted.GetView())
			{
				const size_t newBatch = depthNode->depth > idx;
				idx += newBatch;
				++indices[idx];
			}
		}

		// Find all different resource types.
		LinkedList<TempResource> tempResources{tempArena};
		for (auto& tempNode : view)
		{
			const auto outputView = tempNode.outputs.GetView();
			if (!outputView)
				continue;

			for (auto& output : outputView)
			{
				bool contained = false;

				for (auto& tempResource : tempResources)
					if(tempResource.resource == output.resource)
					{
						contained = true;
						break;
					}

				if (!contained)
				{
					auto& tempResource = tempResources.Add();
					tempResource.resource = output.resource;
				}
			}
		}

		// Define parallel usage.


		const auto nodesView = _nodes.GetView();

		// Link nodes through sync structs.
		for (size_t i = 0; i < length; ++i)
		{
			const auto& node = _nodes[i];
			auto& tempNode = tempNodes[i];
			const size_t waitSemaphoresCount = tempNode.parents.GetCount();
			const auto framesView = node.frames->GetView();

			for (size_t j = 0; j < frameCount; ++j)
			{
				const auto& frame = framesView[j];
				const auto waitSemaphoresView = frame.waitSemaphores->GetView();

				for (size_t k = 0; k < waitSemaphoresCount; ++k)
				{
					const size_t idx = tempNode.parents[i]->index;
					waitSemaphoresView[k] = nodesView[idx].frames[j].GetView()[k].semaphore;
				}
			}
		}

		// Find amount of leafs and collect their semaphores as outputs.
		size_t leafCount = 0;
		for (const auto& tempNode : view)
			leafCount += tempNode.isLeaf;

		_output = Array<Array<VkSemaphore>*>(arena, frameCount);
		for (auto& semaphores : _output.GetView())
			semaphores = arena.New<Array<VkSemaphore>>(1, arena, leafCount);

		for (size_t i = 0; i < length; ++i)
		{
			const auto& node = _nodes[i];
			auto& tempNode = tempNodes[i];

			if (!tempNode.isLeaf)
				continue;

			for (size_t j = 0; j < frameCount; ++j)
			{
				const auto& frame = node.frames->GetView()[j];
				_output[j]->GetView()[i] = frame.semaphore;
			}
		}
	}

	RenderGraph::~RenderGraph()
	{
		for (int32_t i = static_cast<int32_t>(_output.GetLength()) - 1; i >= 0; --i)
		{
			const auto& output = _output[i];
			_arena.Delete(output);
		}

		for (int32_t i = static_cast<int32_t>(_nodes.GetLength()) - 1; i >= 0; --i)
		{
			const auto& node = _nodes[i];
			for (const auto& frame : node.frames->GetView())
				vkDestroySemaphore(_app.device, frame.semaphore, nullptr);
			_arena.Delete(node.frames);
		}
	}

	View<VkSemaphore> RenderGraph::Update(Arena& tempArena) const
	{
		const size_t idx = _swapChain.GetIndex();
		for (const auto& node : _nodes.GetView())
		{
			const auto& frame = node.frames->GetView()[idx];

			VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkResetCommandBuffer(frame.cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
			vkBeginCommandBuffer(frame.cmdBuffer, &cmdBufferBeginInfo);

			node.target->Render(frame.cmdBuffer);

			auto result = vkEndCommandBuffer(frame.cmdBuffer);
			assert(!result);

			const auto waitSemaphores = frame.waitSemaphores->GetView();
			const Array<VkPipelineStageFlags> waitStages{ tempArena, waitSemaphores.GetLength() };
			for (auto& waitStage : waitStages.GetView())
				waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &frame.cmdBuffer;
			submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.GetLength());
			submitInfo.pWaitSemaphores = waitSemaphores.GetData();
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &frame.semaphore;
			submitInfo.pWaitDstStageMask = waitStages;

			result = vkQueueSubmit(_app.queues[App::renderQueue], 1, &submitInfo, nullptr);
			assert(!result);
		}

		return _output.GetView()[idx]->GetView();
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
}
