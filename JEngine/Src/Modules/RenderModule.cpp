﻿#include "pch.h"
#include "Modules/RenderModule.h"
#include "EngineInfo.h"
#include "ModuleFinder.h"
#include "Graphics/Vertex.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkInitializer.h"
#include "Modules/WindowModule.h"
#include "Graphics/VkAllocator.h"
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

		vk::PipelineCreateInfo createInfo{};
		createInfo.tempArena = &info.tempArena;
		createInfo.app = &_app;
		createInfo.layouts = {};
		createInfo.renderPass = _swapChain->GetRenderPass();
		createInfo.shader = _shader;
		createInfo.resolution = _swapChain->GetResolution();
		_pipeline = info.persistentArena.New<vk::Pipeline>(1, createInfo);
		
		Array<vk::Vertex> verts{};
		Array<vk::Vertex::Index> inds{};
		CreateQuadShape(info.tempArena, verts, inds);
		_mesh = info.persistentArena.New<vk::Mesh>(1, _app, *_allocator, verts, inds);
	}

	void RenderModule::OnExit(Info& info)
	{
		const auto result = vkDeviceWaitIdle(_app.device);
		assert(!result);

		info.persistentArena.Delete(_mesh);
		info.persistentArena.Delete(_pipeline);
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
