﻿#pragma once
#include "JEngine/Graphics/SubTexture.h"
#include "JEngine/Modules/JobSystem.h"
#include "JEngine/Modules/RenderModule.h"

namespace game
{
	// Defines a task for the basic render system.
	struct BasicRenderTask final
	{
		glm::vec2 position;
		glm::vec2 scale{ 1 };
		SubTexture subTexture{};
		glm::vec4 color{ 1 };
	};

	// Used to define the basic render system.
	struct BasicRenderSystemCreateInfo final
	{
		size_t capacity = 256;
		const char* vertPath = "Shaders/vert-basic.spv";
		const char* fragPath = "Shaders/frag-basic.spv";
		const char* atlasPath = "Textures/atlas-basic.png";
		const char* atlasCoordsPath = "Textures/atlas-basic.txt";

#ifdef _DEBUG
		// Frame arena.
		je::Array<const char*> texturePaths{};
#endif
	};

	// Camera for the basic render system.
	struct BasicRenderCamera final
	{
		glm::vec2 cameraPosition{};
		float zoom = 0;
		float rotation = 0;
	};

	// Manages simple 2d rendering.
	// Uses instanced-bindless rendering, so is extremely performant.
	class BasicRenderSystem final : public je::JobSystem<BasicRenderTask>, public je::engine::IRenderNode
	{
	public:
		BasicRenderCamera camera{};

		explicit BasicRenderSystem(const BasicRenderSystemCreateInfo& info);

		// Get a sub texture based on the given index.
		[[nodiscard]] SubTexture GetSubTexture(size_t index) const;

	private:
		struct PushConstants final
		{
			BasicRenderCamera camera;
			glm::vec2 resolution;
		};

		const BasicRenderSystemCreateInfo _info;

		glm::vec2 _resolution{};
		VkDescriptorSetLayout _layout{};
		VkShaderModule _modules[2]{};
		je::vk::Mesh _mesh{};
		je::vk::Image _image{};
		je::Array<SubTexture> _subTextures{};
		VkImageView _view{};
		VkDescriptorPool _descriptorPool{};
		je::Array<VkDescriptorSet_T*> _descriptorSets{};
		VkSampler _sampler{};
		je::Array<je::vk::Buffer> _instanceBuffers{};

		void CreateRenderResources(je::Arena& arena, je::Arena& tempArena, const je::vk::App& app,
		    const je::vk::Allocator& allocator, size_t swapChainLength, glm::ivec2 swapChainResolution) override;
		void DestroyRenderResources(je::Arena& arena, const je::vk::App& app,
			const je::vk::Allocator& allocator) override;
		[[nodiscard]] je::vk::RenderNode DefineNode(je::Arena& frameArena, glm::ivec2 swapChainResolution) override;

		static void BindRenderGraphResources(const je::vk::App& app, const je::Array<VkImageView>& views, size_t frameCount, void* userPtr);
		static void Render(const je::vk::App& app, VkCommandBuffer cmd, VkPipelineLayout layout, void* userPtr, size_t frameIndex);
	};
}
