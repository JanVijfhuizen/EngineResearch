#include "pch.h"
#include "Modules/RenderModule.h"
#include "EngineInfo.h"
#include "ModuleFinder.h"
#include "Graphics/VulkanApp.h"
#include "Graphics/VulkanInitializer.h"
#include "Modules/WindowModule.h"
#include "Graphics/VulkanAllocator.h"
#include "Graphics/VulkanSwapChain.h"

namespace je::engine
{
	void RenderModule::OnInitialize(Info& info)
	{
		Module::OnInitialize(info);

		size_t windowExtensionCount;
		const auto windowExtensions = WindowModule::GetRequiredExtensions(windowExtensionCount);
		const Array<StringView> windowExtensionsArr{info.tempArena, windowExtensionCount};
		memcpy(windowExtensionsArr.GetData(), windowExtensions, sizeof(const char*) * windowExtensionCount);

		vkinit::Info vkInfo{};
		vkInfo.tempArena = &info.tempArena;
		vkInfo.createSurface = WindowModule::CreateSurface;
		vkInfo.instanceExtensions = windowExtensionsArr;

		_app = CreateApp(vkInfo);
		_allocator = info.persistentArena.New<VulkanAllocator>(1, info.persistentArena, _app);
		_swapChain = info.persistentArena.New<VulkanSwapChain>(1, info.persistentArena, info.tempArena, _app, *info.finder.Get<WindowModule>());
	}

	void RenderModule::OnExit(Info& info)
	{
		info.persistentArena.Delete(_swapChain);
		info.persistentArena.Delete(_allocator);
		vkinit::DestroyApp(_app);

		Module::OnExit(info);
	}

	void RenderModule::OnUpdate(Info& info)
	{
		Module::OnUpdate(info);
		_swapChain->BeginFrame();
	}

	void RenderModule::OnPostUpdate(Info& info)
	{
		Module::OnPostUpdate(info);
		_swapChain->EndFrame();
	}
}
