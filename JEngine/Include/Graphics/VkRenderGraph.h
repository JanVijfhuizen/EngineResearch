#pragma once
#include "Jlb/Array.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

namespace je::vk
{
	struct RenderGraphResource final
	{
		enum class Type
		{
			texture,
			buffer,
			mesh
		} type;

		enum class Usage
		{
			read,
			write,
			all
		} usage;

		StringView name;
	};

	struct RenderGraphNode
	{
		[[nodiscard]] virtual Array<RenderGraphResource> DefineInputs(Arena& arena);
		[[nodiscard]] virtual Array<RenderGraphResource> DefineOutputs(Arena& arena);
	};

	class RenderGraph final
	{
	public:
		RenderGraph(Arena& arena, Arena& tempArena, const View<RenderGraphNode*>& nodes, const View<RenderGraphResource>& externalResources);
		~RenderGraph();
	};
}
