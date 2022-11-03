#include "pch.h"
#include "Modules/RenderModule.h"
#include "EngineInfo.h"
#include "ModuleFinder.h"
#include "Graphics/Vertex.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkInitializer.h"
#include "Modules/WindowModule.h"
#include "Graphics/VkAllocator.h"
#include "Graphics/VkImage.h"
#include "Graphics/VkLayout.h"
#include "Graphics/VkMesh.h"
#include "Graphics/VkPipeline.h"
#include "Graphics/VkRenderGraph.h"
#include "Graphics/VkShader.h"
#include "Graphics/VkShape.h"
#include "Graphics/VkSwapChain.h"
#include "Graphics/TestRenderNode.h"

namespace je::engine
{
	void RenderModule::OnInitialize(Info& info)
	{
		Module::OnInitialize(info);

		size_t windowExtensionCount;
		const auto windowExtensions = WindowModule::GetRequiredExtensions(windowExtensionCount);
		const Array<StringView> windowExtensionsArr{info.tempArena, windowExtensionCount};
		memcpy(windowExtensionsArr.GetData(), windowExtensions, sizeof(const char*) * windowExtensionCount);

		vk::init::Info vkInfo{};
		vkInfo.tempArena = &info.tempArena;
		vkInfo.createSurface = WindowModule::CreateSurface;
		vkInfo.instanceExtensions = windowExtensionsArr;

		_app = CreateApp(vkInfo);
		_allocator = info.persistentArena.New<vk::Allocator>(1, info.persistentArena, _app);
		_swapChain = info.persistentArena.New<vk::SwapChain>(1, info.persistentArena, info.tempArena, _app, *info.finder.Get<WindowModule>());

		// Temp.
		_shader = info.persistentArena.New<vk::Shader>(1, info.tempArena, _app, "Shaders/vert.spv", "Shaders/frag.spv");

		vk::Layout::Binding binding;
		binding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		_layout = info.persistentArena.New<vk::Layout>(1, _app, info.tempArena, binding);
		auto vkLayout = static_cast<VkDescriptorSetLayout>(*_layout);

		vk::PipelineCreateInfo createInfo{};
		createInfo.tempArena = &info.tempArena;
		createInfo.app = &_app;
		createInfo.layouts = vkLayout;
		createInfo.renderPass = _swapChain->GetRenderPass();
		createInfo.shader = _shader;
		createInfo.resolution = _swapChain->GetResolution();
		_pipeline = info.persistentArena.New<vk::Pipeline>(1, createInfo);
		
		Array<vk::Vertex> verts{};
		Array<vk::Vertex::Index> inds{};
		CreateQuadShape(info.tempArena, verts, inds, .5f);
		_mesh = info.persistentArena.New<vk::Mesh>(1, _app, *_allocator, verts, inds);

		vk::Image::CreateInfo imageCreateInfo{};
		imageCreateInfo.app = &_app;
		imageCreateInfo.allocator = _allocator;
		imageCreateInfo.path = "Textures/test.jpg";
		_image = info.persistentArena.New<vk::Image>(1, imageCreateInfo);

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = _image->GetAspectFlags();
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.image = *_image;
		viewCreateInfo.format = _image->GetFormat();

		auto result = vkCreateImageView(_app.device, &viewCreateInfo, nullptr, &_view);
		assert(!result);

		// Create descriptor pool.
		VkDescriptorPoolSize poolSize;
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = _swapChain->GetLength();
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = _swapChain->GetLength();

		result = vkCreateDescriptorPool(_app.device, &poolInfo, nullptr, &_descriptorPool);
		assert(!result);

		Array<VkDescriptorSetLayout> layouts{info.tempArena, _swapChain->GetLength()};
		for (auto& layout : layouts.GetView())
			layout = vkLayout;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = layouts.GetLength();
		allocInfo.pSetLayouts = layouts;

		_descriptorSets = { info.persistentArena, _swapChain->GetLength() };
		result = vkAllocateDescriptorSets(_app.device, &allocInfo, _descriptorSets.GetData());
		assert(!result);

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(_app.physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		// Since I'm going for pixelart, this will be the defaukt.
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

		result = vkCreateSampler(_app.device, &samplerInfo, nullptr, &_sampler);

		// Bind descriptor sets to the instance data.
		for (size_t i = 0; i < _swapChain->GetLength(); ++i)
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
			atlasInfo.imageLayout = _image->GetLayout();
			atlasInfo.imageView = _view;
			atlasInfo.sampler = _sampler;
			
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstBinding = 0;
			write.dstSet = _descriptorSets[i];
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &atlasInfo;
			write.dstArrayElement = 0;

			vkUpdateDescriptorSets(_app.device, 1, &write, 0, nullptr);
		}

		_testRenderNode = info.persistentArena.New<vk::TestRenderNode>(1, info.persistentArena, _app, *_allocator);
		vk::RenderNode* nodes = _testRenderNode;
		_renderGraph = info.persistentArena.New<vk::RenderGraph>(1, info.persistentArena, info.tempArena, _app, *_allocator, *_swapChain, nodes);
	}

	void RenderModule::OnExit(Info& info)
	{
		const auto result = vkDeviceWaitIdle(_app.device);
		assert(!result);

		vkDestroySampler(_app.device, _sampler, nullptr);
		vkDestroyDescriptorPool(_app.device, _descriptorPool, nullptr);
		vkDestroyImageView(_app.device, _view, nullptr);

		info.persistentArena.Delete(_renderGraph);
		info.persistentArena.Delete(_testRenderNode);
		info.persistentArena.Delete(_image);
		info.persistentArena.Delete(_mesh);
		info.persistentArena.Delete(_pipeline);
		info.persistentArena.Delete(_layout);
		info.persistentArena.Delete(_shader);
		info.persistentArena.Delete(_swapChain);
		info.persistentArena.Delete(_allocator);
		vk::init::DestroyApp(_app);

		Module::OnExit(info);
	}

	void RenderModule::OnUpdate(Info& info)
	{
		Module::OnUpdate(info);

		_swapChain->WaitForImage();

		auto renderGraphSemaphore = _renderGraph->Update();
		const auto cmd = _swapChain->BeginFrame(true);
		
		_pipeline->Bind(cmd);
		_mesh->Bind(cmd);

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->GetLayout(),
			0, 1, &_descriptorSets[_swapChain->GetIndex()], 0, nullptr);

		vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

		_swapChain->EndFrame(info.tempArena, renderGraphSemaphore);
	}
}
