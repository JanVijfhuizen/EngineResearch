#include "pch.h"
#include "Modules/BasicRenderSystem.h"
#include "JEngine/Graphics/VkRenderNode.h"

namespace game
{
	BasicRenderSystem::BasicRenderSystem(const size_t capacity): JobSystem<BasicRenderTask>(capacity, 0)
	{
	}

	je::vk::RenderNode BasicRenderSystem::DefineRenderNode(je::Arena& frameArena)
	{
		return {};
	}
}
