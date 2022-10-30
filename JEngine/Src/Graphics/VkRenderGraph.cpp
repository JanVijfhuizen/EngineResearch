#include "pch.h"
#include "Graphics/VkRenderGraph.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkSwapChain.h"

namespace je::vk
{
	Array<StringView> RenderNode::DefineInputs(Arena& arena) const
	{
		return {};
	}

	Array<RenderNode::Output> RenderNode::DefineOutputs(Arena& arena) const
	{
		return {};
	}

	RenderGraph::RenderGraph(App& app, Arena& arena, Arena& tempArena, SwapChain& swapChain, const View<RenderNode>& nodes) :
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
			tempNode.node = &node;
			tempNode.parents = LinkedList<TempNode*>{tempArena};
			tempNode.inputs = node.DefineInputs(tempArena);
			tempNode.outputs = node.DefineOutputs(tempArena);
		}

		// Link parents by comparing inputs and outputs.
		const auto view = tempNodes.GetView();
		for (auto& tempNode : view)
			for (auto& other : view)
			{
				if (&tempNode == &other)
					continue;

				const auto outputView = other.outputs.GetView();
				for (auto& input : tempNode.inputs.GetView())
					for (auto& output : outputView)
						if (input == output.name)
						{
							tempNode.parents.Add(&other);
							goto NEXT;
						}

			NEXT:
				continue;
			}

		// Define nodes and sync structs for individual frames.
		_nodes = Array<Node>(arena, length);

		{
			size_t index = 0;

			for (auto& node : _nodes.GetView())
			{
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
	}

	RenderGraph::~RenderGraph()
	{
		for (int32_t i = static_cast<int32_t>(_nodes.GetLength()) - 1; i >= 0; --i)
		{
			auto& node = _nodes[i];
			for (const auto& frame : node.frames->GetView())
			{
				vkDestroySemaphore(_app.device, frame.semaphore, nullptr);
			}
			_arena.Delete(node.frames);
		}
	}
}
