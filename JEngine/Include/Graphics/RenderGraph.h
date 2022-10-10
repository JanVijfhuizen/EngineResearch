#pragma once
#include "Jlb/Array.h"
#include "Jlb/StringView.h"

namespace je
{
	class RenderGraph final
	{
	public:
		struct Resource final
		{
			StringView name{};
		};

		class Node
		{
			friend RenderGraph;

		public:
			[[nodiscard]] virtual Array<Resource> DefineInputs(Arena& arena);
			[[nodiscard]] virtual Array<Resource> DefineOutputs(Arena& arena);
		};

		explicit RenderGraph(Arena& arena, Arena& tempArena, const View<Node>& nodes, Node& presentNode);
	};
}
