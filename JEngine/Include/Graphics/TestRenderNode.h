#pragma once
#include "VkRenderGraph.h"

namespace je::vk
{
	class Allocator;
	class Image;

	class TestRenderNode final : public RenderNode
	{
	public:
		TestRenderNode(Arena& arena, App& app);

	private:
		Arena& _arena;
		App& _app;

		void Render(VkCommandBuffer cmdBuffer) override;
		[[nodiscard]] Array<Output> DefineOutputs(Arena& arena) const override;
		void DefineRenderPass(const App& app, VkRenderPass& outRenderPass) override;
	};
}
