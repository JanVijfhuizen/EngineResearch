#pragma once
#include "Module.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkImage.h"
#include "Graphics/VkRenderGraph.h"
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace vk
	{
		class Layout;
		class Pipeline;
		class Shader;
		class Mesh;
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

			vk::Shader* _shader = nullptr;
			vk::Shader* _shader2 = nullptr;
			vk::Layout* _layout = nullptr;
			vk::Pipeline* _pipeline = nullptr;

			vk::Mesh* _mesh = nullptr;
			vk::Image* _image = nullptr;
			vk::RenderGraph* _renderGraph;

			VkImageView _view;
			VkDescriptorPool _descriptorPool;
			Array<VkDescriptorSet> _descriptorSets{};
			VkSampler _sampler;

			void OnInitialize(Info& info) override;
			void OnExit(Info& info) override;

			void OnUpdate(Info& info) override;

			static void Render(VkCommandBuffer cmd, void* userPtr, const size_t frameIndex);
		};
	}
}
