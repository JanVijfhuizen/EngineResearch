#pragma once
#include "Jlb/View.h"

namespace je::vk
{
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
	};

	class RenderGraph final
	{
	
	};
}
