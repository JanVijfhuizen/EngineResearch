#include "pch.h"
#include "Modules/BasicRenderSystem.h"
#include "JEngine/Graphics/VkRenderNode.h"

namespace game
{
	BasicRenderSystem::BasicRenderSystem(const size_t capacity): JobSystem<BasicRenderTask>(capacity, 0)
	{
	}

	je::vk::RenderNode BasicRenderSystem::DefineRenderNode()
	{
		return {};
	}

	void BasicRenderSystem::OnBegin(je::engine::Info& info)
	{
		JobSystem<BasicRenderTask>::OnBegin(info);
	}

	void BasicRenderSystem::OnUpdate(je::engine::Info& info, const Jobs& jobs)
	{
	}

	void BasicRenderSystem::OnExit(je::engine::Info& info)
	{
		JobSystem<BasicRenderTask>::OnExit(info);
	}
}
