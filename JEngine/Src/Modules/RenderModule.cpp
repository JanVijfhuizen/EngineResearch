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
#include "Graphics/VkShader.h"
#include "Graphics/VkShape.h"
#include "Graphics/VkSwapChain.h"

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
		binding.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
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
		CreateQuadShape(info.tempArena, verts, inds);
		_mesh = info.persistentArena.New<vk::Mesh>(1, _app, *_allocator, verts, inds);
		_image = info.persistentArena.New<vk::Image>(1, _app, *_allocator, "Textures/test.jpg");

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.image = *_image;
		viewCreateInfo.format = _image->GetFormat();

		const auto result = vkCreateImageView(_app.device, &viewCreateInfo, nullptr, &_view);
		assert(!result);
	}

	void RenderModule::OnExit(Info& info)
	{
		const auto result = vkDeviceWaitIdle(_app.device);
		assert(!result);

		vkDestroyImageView(_app.device, _view, nullptr);

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
		const auto cmd = _swapChain->BeginFrame();
		
		_pipeline->Bind(cmd);
		_mesh->Bind(cmd);

		vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
	}

	void RenderModule::OnPostUpdate(Info& info)
	{
		Module::OnPostUpdate(info);
		_swapChain->EndFrame();
	}
}
