#include "pch.h"
#include "Modules/RenderModule.h"
#include "Modules/WindowModule.h"
#include "JEngine/EngineInfo.h"
#include "JEngine/Graphics/Vertex.h"
#include "JEngine/Graphics/VkApp.h"
#include "Graphics/VkInitializer.h"
#include "JEngine/Graphics/VkAllocator.h"
#include "JEngine/Graphics/VkLayout.h"
#include "JEngine/Graphics/VkMesh.h"
#include "JEngine/Graphics/VkPipeline.h"
#include "Graphics/VkRenderGraph.h"
#include "JEngine/Graphics/VkShader.h"
#include "JEngine/Graphics/VkShapes.h"
#include "Graphics/VkSwapChain.h"

namespace je::engine
{
	RenderModule::RenderModule(const RenderModuleCreateInfo& info) : _info(info)
	{
		assert(info.defineResources);
		assert(info.destroyResources);
		assert(info.defineRenderGraph);
		assert(info.bindRenderGraphResources);
	}

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

		_info.defineResources(info.persistentArena, info.tempArena, _app, *_allocator, _swapChain->GetLength(), _swapChain->GetResolution(), _info.userPtr);

		// Temp.
		_modules[0] = CreateShaderModule(info.tempArena, _app, "Shaders/vert.spv");
		_modules[1] = CreateShaderModule(info.tempArena, _app, "Shaders/frag.spv");

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

		vk::PipelineCreateInfo::Module shaderModules[2];
		shaderModules[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderModules[0].module = _modules[0];
		shaderModules[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderModules[1].module = _modules[1];
		Array<vk::PipelineCreateInfo::Module> shader{};
		shader.data = shaderModules;
		shader.length = 2;

		vk::PipelineCreateInfo createInfo{};
		createInfo.layouts = pipelineLayouts;
		createInfo.renderPass = _swapChain->GetRenderPass();
		createInfo.modules = shader;
		createInfo.resolution = _swapChain->GetResolution();
		_pipeline = CreatePipeline(createInfo, info.tempArena, _app);

		{
			const auto _ = info.tempArena.CreateScope();

			Array<vk::Vertex> verts{};
			Array<vk::Vertex::Index> inds{};
			CreateQuadShape(info.tempArena, verts, inds, .5f);
			_mesh = CreateMesh(_app, *_allocator, verts, inds);
		}

		// Create descriptor pool.
		VkDescriptorPoolSize poolSize;
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = static_cast<uint32_t>(_swapChain->GetLength());
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(_swapChain->GetLength());

		auto result = vkCreateDescriptorPool(_app.device, &poolInfo, nullptr, &_descriptorPool);
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

		const auto nodes = _info.defineRenderGraph(info.tempArena, _swapChain->GetLength(), _swapChain->GetResolution(), _info.userPtr);
		_renderGraph = info.persistentArena.New<vk::RenderGraph>(1, info.persistentArena, info.tempArena, _app, *_allocator, *_swapChain, nodes);
		_info.bindRenderGraphResources(nodes, _app, _swapChain->GetLength(), _info.userPtr);

		// Bind descriptor sets to the render graph output.
		for (size_t i = 0; i < _swapChain->GetLength(); ++i)
		{
			VkWriteDescriptorSet write{};

			// Bind render graph output.
			VkDescriptorImageInfo  atlasInfo{};
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
		
		info.persistentArena.Delete(_renderGraph);
		DestroyMesh(_mesh, _app, *_allocator);
		DestroyPipeline(_pipeline, _app);
		vkDestroyDescriptorSetLayout(_app.device, _layout, nullptr);
		for (const auto& mod : _modules)
			vkDestroyShaderModule(_app.device, mod, nullptr);

		_info.destroyResources(info.persistentArena, _app, *_allocator, _info.userPtr);
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
}
