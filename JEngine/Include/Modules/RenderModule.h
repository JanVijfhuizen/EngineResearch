#pragma once
#include "JEngine/Module.h"
#include "JEngine/Graphics/VkApp.h"
#include "JEngine/Graphics/VkMesh.h"
#include "Graphics/VkRenderGraph.h"
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
		struct RenderModuleCreateInfo final
		{
			void(*defineResources)(Arena& arena, Arena& tempArena, const vk::App& app, const vk::Allocator& allocator, size_t swapChainLength, glm::ivec2 swapChainResolution, void* userPtr) = nullptr;
			void(*destroyResources)(Arena& arena, const vk::App& app, const vk::Allocator& allocator, void* userPtr) = nullptr;
			Array<vk::RenderNode>(*defineRenderGraph)(Arena& tempArena, size_t swapChainLength, glm::ivec2 swapChainResolution, void* userPtr) = nullptr;
			void(*bindRenderGraphResources)(const Array<vk::RenderNode>& nodes, const vk::App& app, size_t swapChainLength, void* userPtr) = nullptr;
			void* userPtr = nullptr;
		};

		class RenderModule final : public Module
		{
		public:
			explicit RenderModule(const RenderModuleCreateInfo& info);

		private:
			const RenderModuleCreateInfo _info;

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
