#pragma once
#include "Jlb/Array.h"
#include "Jlb/StringView.h"

namespace je::render_graph
{
	struct Resource final
	{
		StringView name{};
	};

	class Node
	{
		friend class RenderGraph;

	protected:
		[[nodiscard]] virtual Array<Resource> DefineInputs(Arena& arena);
		[[nodiscard]] virtual Array<Resource> DefineOutputs(Arena& arena);
	};

	class RenderGraph final
	{
	public:
		explicit RenderGraph(Arena& arena, Arena& tempArena, const View<Node*>& nodes, Node& presentNode);
	};
}
