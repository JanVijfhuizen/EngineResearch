#include "pch.h"
#include "Modules/BasicRenderSystem.h"

#include "JEngine/Graphics/ObjLoader.h"
#include "JEngine/Graphics/Texture.h"
#include "JEngine/Graphics/VkLayout.h"
#include "JEngine/Graphics/VkShader.h"

struct RenderResources;

namespace game
{
	BasicRenderSystem::BasicRenderSystem(const size_t capacity): JobSystem<BasicRenderTask>(capacity, 0)
	{
	}

	void BasicRenderSystem::CreateRenderResources(je::Arena& arena, je::Arena& tempArena, const je::vk::App& app,
		const je::vk::Allocator& allocator, size_t swapChainLength, glm::ivec2 swapChainResolution)
	{
		const auto _ = tempArena.CreateScope();

		je::vk::Binding binding;
		binding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		je::Array<je::vk::Binding> bindings{};
		bindings.data = &binding;
		bindings.length = 1;
		_layout = CreateLayout(tempArena, app, bindings);

		_modules[0] = CreateShaderModule(tempArena, app, "Shaders/vert2.spv");
		_modules[1] = CreateShaderModule(tempArena, app, "Shaders/frag2.spv");

		{
			const auto _ = tempArena.CreateScope();

			je::Array<je::vk::Vertex> verts{};
			je::Array<je::vk::Vertex::Index> inds{};
			constexpr float scale = .5f;
			je::obj::Load(tempArena, "Meshes/cube.obj", verts, inds, scale);
			_mesh = CreateMesh(app, allocator, verts, inds);
		}

#ifdef _DEBUG
		const auto textures = je::CreateArray<const char*>(tempArena, 4);
		textures.data[0] = "Textures/humanoid.png";
		textures.data[1] = "Textures/moveArrow.png";
		textures.data[2] = "Textures/bash-card.png";
		textures.data[3] = "Textures/tile.png";
		je::texture::GenerateAtlas(arena, tempArena, textures, "Textures/atlas.png", "Textures/atlas.txt");
#endif


		_image = je::texture::Load(app, allocator, "Textures/atlas.png");
		const auto coords = je::texture::LoadAtlasCoordinates(tempArena, "Textures/atlas.txt");

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = _image.aspectFlags;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.image = _image.image;
		viewCreateInfo.format = _image.format;

		auto result = vkCreateImageView(app.device, &viewCreateInfo, nullptr, &_view);
		assert(!result);

		// Create descriptor pool.
		VkDescriptorPoolSize poolSize;
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = static_cast<uint32_t>(swapChainLength);
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(swapChainLength);

		result = vkCreateDescriptorPool(app.device, &poolInfo, nullptr, &_descriptorPool);
		assert(!result);

		auto layouts = je::CreateArray<VkDescriptorSetLayout>(tempArena, swapChainLength);
		for (auto& layout : layouts)
			layout = _layout;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = layouts.length;
		allocInfo.pSetLayouts = layouts.data;

		_descriptorSets = je::CreateArray<VkDescriptorSet>(arena, swapChainLength);
		result = vkAllocateDescriptorSets(app.device, &allocInfo, _descriptorSets.data);
		assert(!result);

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(app.physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		// Since I'm going for pixelart, this will be the default.
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 0;

		result = vkCreateSampler(app.device, &samplerInfo, nullptr, &_sampler);
	}

	void BasicRenderSystem::DestroyRenderResources(je::Arena& arena, const je::vk::App& app,
		const je::vk::Allocator& allocator)
	{
		vkDestroySampler(app.device, _sampler, nullptr);
		vkDestroyDescriptorSetLayout(app.device, _layout, nullptr);
		DestroyArray(_descriptorSets, arena);
		vkDestroyDescriptorPool(app.device, _descriptorPool, nullptr);
		vkDestroyImageView(app.device, _view, nullptr);
		DestroyImage(_image, app, allocator);
		DestroyMesh(_mesh, app, allocator);
		for (const auto& mod : _modules)
			vkDestroyShaderModule(app.device, mod, nullptr);
	}

	void BasicRenderSystem::BindRenderGraphResources(const je::vk::App& app, const je::Array<VkImageView>& views, const size_t frameCount, void* userPtr)
	{
		const auto ptr = static_cast<BasicRenderSystem*>(userPtr);

		// Bind descriptor sets for the render node.
		for (size_t i = 0; i < frameCount; ++i)
		{
			VkWriteDescriptorSet write{};

			/*
			// Bind instance buffer.
			VkDescriptorBufferInfo instanceInfo{};
			instanceInfo.buffer = _instanceBuffers[i].buffer;
			instanceInfo.offset = 0;
			instanceInfo.range = sizeof(Job) * JobSystem<Job>::GetLength();

			auto& instanceWrite = writes[0];
			instanceWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			instanceWrite.dstBinding = 0;
			instanceWrite.dstSet = _descriptorSets[i];
			instanceWrite.descriptorCount = 1;
			instanceWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			instanceWrite.pBufferInfo = &instanceInfo;
			instanceWrite.dstArrayElement = 0;
			*/
			// Bind texture atlas.
			VkDescriptorImageInfo  atlasInfo{};
			atlasInfo.imageLayout = ptr->_image.layout;
			atlasInfo.imageView = ptr->_view;
			atlasInfo.sampler = ptr->_sampler;

			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstBinding = 0;
			write.dstSet = ptr->_descriptorSets[i];
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &atlasInfo;
			write.dstArrayElement = 0;

			vkUpdateDescriptorSets(app.device, 1, &write, 0, nullptr);
		}
	}
	void  BasicRenderSystem::Render(const VkCommandBuffer cmd, const VkPipelineLayout layout, void* userPtr, const size_t frameIndex)
	{
		const auto ptr = static_cast<BasicRenderSystem*>(userPtr);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			0, 1, &ptr->_descriptorSets[frameIndex], 0, nullptr);
		ptr->_mesh.Draw(cmd, 1);
	}

	je::vk::RenderNode BasicRenderSystem::DefineNode(je::Arena& frameArena, glm::ivec2 swapChainResolution)
	{
		const auto modules = je::CreateArray<je::vk::PipelineCreateInfo::Module>(frameArena, 2);
		modules[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		modules[0].module = _modules[0];
		modules[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		modules[1].module = _modules[1];
		const auto outputs = je::CreateArray<je::vk::RenderNode::Output>(frameArena, 1);
		outputs[0].name = "Result";
		outputs[0].resource.resolution = swapChainResolution;
		
		je::vk::RenderNode node{};
		node.outputs = outputs;
		node.renderFunc = Render;
		node.bindResourcesFunc = BindRenderGraphResources;
		node.userPtr = this;
		node.modules = modules;
		node.layouts.length = 1;
		node.layouts.data = &_layout;
		return node;
	}
}
