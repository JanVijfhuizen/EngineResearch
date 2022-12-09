#pragma once
#include "Graphics/SubTexture.h"
#include "JEngine/Modules/JobSystem.h"
#include "JEngine/Modules/RenderModule.h"

namespace game
{
	struct BasicRenderTask final
	{
		glm::vec2 position;
		glm::vec2 scale{ 1 };
		SubTexture subTexture{};
		glm::vec4 color{ 1 };
	};

	class BasicRenderSystem final : public je::JobSystem<BasicRenderTask>, public je::engine::IRenderNode
	{
	public:
		explicit BasicRenderSystem(size_t capacity);

	private:
		VkDescriptorSetLayout _layout{};
		VkShaderModule _modules[2]{};
		je::vk::Mesh _mesh{};
		je::vk::Image _image{};
		VkImageView _view{};
		VkDescriptorPool _descriptorPool{};
		je::Array<VkDescriptorSet_T*> _descriptorSets{};
		VkSampler _sampler{};

		void CreateRenderResources(je::Arena& arena, je::Arena& tempArena, const je::vk::App& app,
			const je::vk::Allocator& allocator, size_t swapChainLength, glm::ivec2 swapChainResolution) override;
		void DestroyRenderResources(je::Arena& arena, const je::vk::App& app,
			const je::vk::Allocator& allocator) override;
		[[nodiscard]] je::vk::RenderNode DefineNode(je::Arena& frameArena, glm::ivec2 swapChainResolution) override;

		static void BindRenderGraphResources(const je::vk::App& app, const je::Array<VkImageView>& views, size_t frameCount, void* userPtr);
		static void Render(VkCommandBuffer cmd, VkPipelineLayout layout, void* userPtr, size_t frameIndex);
	};
}
