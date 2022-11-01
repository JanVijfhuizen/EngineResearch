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
		Image* _image;
		Arena& _arena;

		void Render(const UpdateInfo& info) override;
	};
}
