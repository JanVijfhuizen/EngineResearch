#pragma once
#include "Graphics/SubTexture.h"
#include "JEngine/Modules/JobSystem.h"

namespace je::vk
{
	struct RenderNode;
}

namespace game
{
	struct BasicRenderTask final
	{
		glm::vec2 position;
		glm::vec2 scale{ 1 };
		SubTexture subTexture{};
		glm::vec4 color{ 1 };
	};

	class BasicRenderSystem final : public je::JobSystem<BasicRenderTask>
	{
	public:
		explicit BasicRenderSystem(size_t capacity);

		[[nodiscard]] je::vk::RenderNode DefineRenderNode(je::Arena& frameArena);
	};
}