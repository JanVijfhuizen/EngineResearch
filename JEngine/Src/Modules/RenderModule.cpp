#include "pch.h"
#include "Modules/RenderModule.h"
#include "EngineInfo.h"
#include "Graphics/ObjLoader.h"
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
#include "Graphics/VkShapes.h"
#include "Graphics/VkSwapChain.h"

namespace je::engine
{
	void RenderModule::OnInitialize(Info& info)
	{
		Module::OnInitialize(info);

		const auto _ = info.tempArena.CreateScope();

		size_t windowExtensionCount;
		const auto windowExtensions = WindowModule::GetRequiredExtensions(windowExtensionCount);
		const auto windowExtensionsArr = CreateArray<const char*>(info.tempArena, windowExtensionCount);
		memcpy(windowExtensionsArr.data, windowExtensions, sizeof(const char*) * windowExtensionCount);

		vk::init::Info vkInfo{};
		vkInfo.tempArena = &info.tempArena;
		vkInfo.createSurface = WindowModule::CreateSurface;
		vkInfo.instanceExtensions = windowExtensionsArr;

		_app = CreateApp(vkInfo);
		_allocator = info.persistentArena.New<vk::Allocator>(1, info.persistentArena, _app);
		_swapChain = info.persistentArena.New<vk::SwapChain>(1, info.persistentArena, info.tempArena, _app, *info.finder.Get<WindowModule>());

		// Temp.
		_modules[0] = CreateShaderModule(info.tempArena, _app, "Shaders/vert.spv");
		_modules[1] = CreateShaderModule(info.tempArena, _app, "Shaders/frag.spv");
		_modules[2] = CreateShaderModule(info.tempArena, _app, "Shaders/vert2.spv");
		_modules[3] = CreateShaderModule(info.tempArena, _app, "Shaders/frag2.spv");

		vk::Binding binding;
		binding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		Array<vk::Binding> bindings{};
		bindings.data = &binding;
		bindings.length = 1;
		_layout = CreateLayout(info.tempArena, _app, bindings);
		Array<VkDescriptorSetLayout> pipelineLayouts{};
		pipelineLayouts.data = &_layout;
		pipelineLayouts.length = 1;

		vk::PipelineCreateInfo::Module shader1Modules[2];
		shader1Modules[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader1Modules[0].module = _modules[0];
		shader1Modules[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader1Modules[1].module = _modules[1];
		Array<vk::PipelineCreateInfo::Module> shader1{};
		shader1.data = shader1Modules;
		shader1.length = 2;

		vk::PipelineCreateInfo createInfo{};
		createInfo.layouts = pipelineLayouts;
		createInfo.renderPass = _swapChain->GetRenderPass();
		createInfo.modules = shader1;
		createInfo.resolution = _swapChain->GetResolution();
		_pipeline = CreatePipeline(createInfo, info.tempArena, _app);

		{
			const auto _ = info.tempArena.CreateScope();

			Array<vk::Vertex> verts{};
			Array<vk::Vertex::Index> inds{};
			CreateQuadShape(info.tempArena, verts, inds, .5f);
			_mesh = CreateMesh(_app, *_allocator, verts, inds);
		}

		{
			const auto _ = info.tempArena.CreateScope();
			
			Array<vk::Vertex> verts{};
			Array<vk::Vertex::Index> inds{};
			constexpr float scale = .5f;
			obj::Load(info.tempArena, "Meshes/cube.obj", verts, inds, scale);
			_mesh2 = CreateMesh(_app, *_allocator, verts, inds);
		}

		vk::ImageCreateInfo imageCreateInfo{};
		_image = CreateImage(_app, *_allocator, imageCreateInfo, "Textures/test.jpg");

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

		auto layouts = CreateArray<VkDescriptorSetLayout>(info.tempArena, _swapChain->GetLength());
		for (auto& layout : layouts)
			layout = _layout;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = layouts.length;
		allocInfo.pSetLayouts = layouts.data;

		_descriptorSets = CreateArray<VkDescriptorSet>(info.persistentArena, _swapChain->GetLength());
		result = vkAllocateDescriptorSets(_app.device, &allocInfo, _descriptorSets.data);
		assert(!result);

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(_app.physicalDevice, &properties);

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

		result = vkCreateSampler(_app.device, &samplerInfo, nullptr, &_sampler);

		vk::RenderNode::Output output{};
		output.name = "Result";
		output.resource.resolution = _swapChain->GetResolution();// glm::ivec3{ 800, 600, 3 };
		Array<vk::RenderNode::Output> outputs{};
		outputs.data = &output;
		outputs.length = 1;

		auto views = CreateArray<VkImageView>(info.tempArena, _swapChain->GetLength() * 0);


		vk::PipelineCreateInfo::Module shader2Modules[2];
		shader2Modules[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader2Modules[0].module = _modules[2];
		shader2Modules[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader2Modules[1].module = _modules[3];
		Array<vk::PipelineCreateInfo::Module> shader2{};
		shader2.data = shader2Modules;
		shader2.length = 2;

		vk::RenderNode node{};
		node.outputs = outputs;
		node.renderFunc = Render;
		node.userPtr = this;
		node.modules = shader2;
		node.outImageViews = views;
		Array<vk::RenderNode> nodes{};
		nodes.data = &node;
		nodes.length = 1;

		_renderGraph = info.persistentArena.New<vk::RenderGraph>(1, info.persistentArena, info.tempArena, _app, *_allocator, *_swapChain, nodes);

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
			//atlasInfo.imageLayout = _image->GetLayout();
			//atlasInfo.imageView = _view;
			const auto renderGraphResult = _renderGraph->GetResult(0);
			atlasInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			atlasInfo.imageView = renderGraphResult;
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
	}

	void RenderModule::OnExit(Info& info)
	{
		const auto result = vkDeviceWaitIdle(_app.device);
		assert(!result);

		vkDestroySampler(_app.device, _sampler, nullptr);
		DestroyArray(_descriptorSets, info.persistentArena);
		vkDestroyDescriptorPool(_app.device, _descriptorPool, nullptr);
		vkDestroyImageView(_app.device, _view, nullptr);

		info.persistentArena.Delete(_renderGraph);
		DestroyImage(_image, _app, *_allocator);
		DestroyMesh(_mesh2);
		DestroyMesh(_mesh);
		DestroyPipeline(_pipeline, _app);
		vkDestroyDescriptorSetLayout(_app.device, _layout, nullptr);
		for (const auto& mod : _modules)
			vkDestroyShaderModule(_app.device, mod, nullptr);
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
		
		_pipeline.Bind(cmd);

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.layout,
			0, 1, &_descriptorSets[_swapChain->GetIndex()], 0, nullptr);
		
		_mesh.Draw(cmd, 1);

		Array<VkSemaphore> semaphores{};
		semaphores.data = &renderGraphSemaphore;
		semaphores.length = 1;
		_swapChain->EndFrame(info.tempArena, semaphores);
	}

	void RenderModule::Render(const VkCommandBuffer cmd, void* userPtr, const size_t frameIndex)
	{
		const auto ptr = static_cast<RenderModule*>(userPtr);
		ptr->_mesh2.Draw(cmd, 1);
	}
}
