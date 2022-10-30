#pragma once
#include "Jlb/Array.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

namespace je::vk
{
	struct App;

	class RenderNode
	{
		friend class RenderGraph;

	protected:
		struct UpdateInfo final
		{
			View<VkSemaphore> waitSemaphores;
			VkSemaphore renderFinishedSemaphore;
		};

		virtual void Render(const UpdateInfo& info) = 0;
		[[nodiscard]] virtual Array<StringView> DefineInputs(Arena& arena) const;
		[[nodiscard]] virtual Array<StringView> DefineOutputs(Arena& arena) const;
	};

	class RenderGraph final
	{
	public:
		RenderGraph(App& app, Arena& arena, Arena& tempArena, const View<RenderNode>& nodes);
		~RenderGraph();
	};
}
