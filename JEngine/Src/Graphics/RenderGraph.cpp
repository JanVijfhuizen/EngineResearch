#include "pch.h"
#include "Graphics/RenderGraph.h"
#include "Jlb/JMath.h"
#include "Jlb/LinkedList.h"

namespace je
{
	struct NodeInfo final
	{
		RenderGraph::Node* node = nullptr;
		View<RenderGraph::Resource> inputs{};
		View<RenderGraph::Resource> outputs{};
		LinkedList<NodeInfo*>* children = nullptr;
		size_t depth = 0;
		bool visited = false;
	};

	Array<RenderGraph::Resource> RenderGraph::Node::DefineInputs(Arena& arena)
	{
		return { arena, 0 };
	}

	Array<RenderGraph::Resource> RenderGraph::Node::DefineOutputs(Arena& arena)
	{
		return { arena, 0 };
	}

	void DefineDepth(const NodeInfo& current)
	{
		for (const auto& child : *current.children)
			if(child->depth < current.depth + 1)
			{
				child->depth = current.depth + 1;
				DefineDepth(*child);
			}
	}

	void LinkNodes(Arena& arena, const NodeInfo& current, const View<NodeInfo>& nodes)
	{
		if (current.visited)
			return;

		for (const auto& input : current.inputs)
		{
			for (auto& node : nodes)
				for (auto& output : node.outputs)
					if (input.name == output.name)
					{
						node.depth = math::Max(node.depth, current.depth + 1);
						current.children->Add(&node);
						DefineDepth(node);
						LinkNodes(arena, node, nodes);
						break;
					}
		}
	}

	RenderGraph::RenderGraph(Arena& arena, Arena& tempArena, const View<Node>& nodes, Node& presentNode)
	{
		{
			// Link all nodes and define the depth of every node.
			const auto _ = tempArena.CreateScope();
			const Array<NodeInfo> nodeInfos{ tempArena, nodes.GetLength()};

			NodeInfo root{};
			root.children = tempArena.New<LinkedList<NodeInfo*>>(1, tempArena);
			root.node = &presentNode;

			const size_t length = nodes.GetLength();
			for (size_t i = 0; i < length; ++i)
			{
				nodeInfos[i].children = tempArena.New<LinkedList<NodeInfo*>>(1, tempArena);
				nodeInfos[i].node = &nodes[i];
			}

			root.inputs = presentNode.DefineInputs(tempArena).GetView();
			root.outputs = presentNode.DefineOutputs(tempArena).GetView();

			for (auto& nodeInfo : nodeInfos.GetView())
			{
				nodeInfo.inputs = nodeInfo.node->DefineInputs(tempArena).GetView();
				nodeInfo.outputs = nodeInfo.node->DefineOutputs(tempArena).GetView();
			}

			LinkNodes(tempArena, root, nodeInfos);
		}
	}
}
