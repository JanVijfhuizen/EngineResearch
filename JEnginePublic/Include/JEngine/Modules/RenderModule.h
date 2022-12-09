#pragma once
#include "JEngine/Module.h"
#include "JEngine/Graphics/VkApp.h"
#include "JEngine/Graphics/VkMesh.h"
#include "JEngine/Graphics/VkRenderGraph.h"
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace vk
	{
		class SwapChain;
		class Allocator;
	}

	namespace engine
	{
		class IRenderNode
		{
			friend class RenderModule;

		protected:
			// Create the resources used for rendering.
			virtual void CreateRenderResources(Arena& arena, Arena& tempArena, const vk::App& app, const vk::Allocator& allocator,
				size_t swapChainLength, glm::ivec2 swapChainResolution) = 0;
			// Destroy the resources used for rendering.
			virtual void DestroyRenderResources(Arena& arena, const vk::App& app, const vk::Allocator& allocator) = 0;
			// Define a render node to be used for a render graph.
			[[nodiscard]] virtual vk::RenderNode DefineNode(Arena& frameArena, glm::ivec2 swapChainResolution) = 0;
		};

		// Manages the rendering and all relevant vulkan classes.
		class RenderModule final : public Module
		{
			vk::App _app{};
			vk::Allocator* _allocator = nullptr;
			vk::SwapChain* _swapChain = nullptr;
			
			vk::Pipeline _pipeline;
			vk::Mesh _mesh;
			vk::RenderGraph* _renderGraph;

			VkShaderModule _modules[2]{};
			VkDescriptorSetLayout _layout;
			VkDescriptorPool _descriptorPool;
			Array<VkDescriptorSet> _descriptorSets{};
			Array<VkDescriptorSet> _descriptorSetsNode{};
			VkSampler _sampler;

			void OnInitialize(Info& info) override;
			void OnExit(Info& info) override;
			void OnUpdate(Info& info) override;
		};
	}
}
