#include "pch.h"
#include "Modules/BasicRenderSystem.h"

namespace game
{
	BasicRenderSystem::BasicRenderSystem(const size_t capacity): JobSystem<BasicRenderTask>(capacity, 0)
	{
	}
}
