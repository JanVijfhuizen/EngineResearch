#pragma once
#include "Module.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkImage.h"
#include "Graphics/VkMesh.h"
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
		class RenderModule final : public Module
		{
			vk::App _app{};
			vk::Allocator* _allocator = nullptr;
			vk::SwapChain* _swapChain = nullptr;
			
			vk::Pipeline _pipeline;
			vk::Mesh _mesh, _mesh2;
			vk::Image _image;
			vk::RenderGraph* _renderGraph;

			VkShaderModule _modules[4]{};
			VkDescriptorSetLayout _layout;
			VkImageView _view;
			VkDescriptorPool _descriptorPool;
			VkDescriptorPool _descriptorPoolNode;
			Array<VkDescriptorSet> _descriptorSets{};
			Array<VkDescriptorSet> _descriptorSetsNode{};
			VkSampler _sampler;

			void OnInitialize(Info& info) override;
			void OnExit(Info& info) override;

			void OnUpdate(Info& info) override;

			static void Render(VkCommandBuffer cmd, VkPipelineLayout layout, void* userPtr, size_t frameIndex);
		};
	}
}
