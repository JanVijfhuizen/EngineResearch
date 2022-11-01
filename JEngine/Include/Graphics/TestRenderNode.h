#pragma once
#include "VkRenderGraph.h"

namespace je::vk
{
	class Allocator;
	class Image;

	class TestRenderNode final : public RenderNode
	{
	public:
		TestRenderNode(Arena& arena, App& app, Allocator& allocator);
		~TestRenderNode();

	private:
		Arena& _arena;
		App& _app;
		Image* _image;
		VkRenderPass _renderPass;
		VkImageView _view;
		VkFramebuffer _frameBuffer;

		void Render(VkCommandBuffer cmdBuffer) override;
	};
}
