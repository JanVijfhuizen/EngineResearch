#pragma once
#include "Module.h"
#include "Graphics/VulkanApp.h"
#include "Jlb/View.h"

namespace je
{
	class Arena;

	namespace render_graph
	{
		class Node;
		class RenderGraph;
	}

	class VulkanAllocator;

	namespace engine
	{
		class RenderModule final : public Module
		{
			VulkanApp _app{};
			VulkanAllocator* _allocator = nullptr;
			render_graph::RenderGraph* _renderGraph = nullptr;
			
			void OnInitialize(Info& info) override;
			void OnExit(Info& info) override;
			[[nodiscard]] static View<render_graph::Node*> DefineDefaultGraphNodes(Arena& arena, render_graph::Node*& outPresentNode);
		};
	}
}
