#include "pch.h"
#include "Graphics/VkRenderGraph.h"
#include "Jlb/JVector.h"
#include "Jlb/LinkedList.h"

namespace je::vk
{
	struct DepthNode final
	{
		size_t depth = 0;
		RenderGraphNode* node = nullptr;
		LinkedList<DepthNode*>* parents = nullptr;
		View<RenderGraphResource> inputs{};
		View<RenderGraphResource> outputs{};
	};

	Array<RenderGraphResource> RenderGraphNode::DefineInputs(Arena& arena)
	{
		return {arena, 0 };
	}

	Array<RenderGraphResource> RenderGraphNode::DefineOutputs(Arena& arena)
	{
		return {arena, 0 };
	}

	RenderGraph::RenderGraph(Arena& arena, Arena& tempArena, const View<RenderGraphNode*>& nodes, const View<RenderGraphResource>& externalResources)
	{
		const auto _ = tempArena.CreateScope();
		Vector<DepthNode> depthNodes{tempArena, nodes.GetLength()};

		for (const auto& node : nodes)
		{
			auto& depthNode = depthNodes.Add();
			depthNode.node = node;
			depthNode.parents = tempArena.New<LinkedList<DepthNode*>>(1, tempArena);
			// Unsafe, but thanks to the scope it doesn't matter.
			depthNode.inputs = node->DefineInputs(tempArena);
			depthNode.outputs = node->DefineOutputs(tempArena);
		}

		// Find parents.
		const auto depthNodesView = depthNodes.GetView();
		for (const auto& node : depthNodesView)
			for (auto& otherNode : depthNodesView)
			{
				bool linked = false;
				for (auto& input : node.inputs)
				{
					for (auto& output : node.outputs)
						if(input.name == output.name)
						{
							linked = true;
							break;
						}

					if (linked)
						break;
				}

				if(linked)
					node.parents->Add(&otherNode);
			}

		// Create semaphores for every node.
	}

	RenderGraph::~RenderGraph()
	{
	}
}
