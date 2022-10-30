#include "pch.h"
#include "Graphics/VkRenderGraph.h"
#include "Jlb/LinkedList.h"

namespace je::vk
{
	struct TempNode final
	{
		const RenderNode* node;
		LinkedList<RenderNode*> parents{};
		Array<StringView> inputs{};
		Array<StringView> outputs{};
	};

	Array<StringView> RenderNode::DefineInputs(Arena& arena) const
	{
		return {};
	}

	Array<StringView> RenderNode::DefineOutputs(Arena& arena) const
	{
		return {};
	}

	RenderGraph::RenderGraph(App& app, Arena& arena, Arena& tempArena, const View<RenderNode>& nodes)
	{
		const auto _ = tempArena.CreateScope();

		const size_t length = nodes.GetLength();
		const Array<TempNode> tempNodes{tempArena, length };

		for (size_t i = 0; i < length; ++i)
		{
			const auto& node = nodes[i];
			auto& tempNode = tempNodes[i];

			tempNode.node = &node;
			tempNode.parents = LinkedList<RenderNode*>{tempArena};
			tempNode.inputs = node.DefineInputs(tempArena);
			tempNode.outputs = node.DefineOutputs(tempArena);
		}
	}

	RenderGraph::~RenderGraph()
	{
	}
}
