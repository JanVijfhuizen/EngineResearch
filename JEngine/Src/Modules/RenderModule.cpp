#include "pch.h"
#include "Modules/RenderModule.h"

#include "EngineInfo.h"
#include "Graphics/VulkanApp.h"
#include "Graphics/VulkanInitializer.h"

namespace je::engine
{
	void RenderModule::OnInitialize(Info& info)
	{
		Module::OnInitialize(info);

		vkinit::Info vkInfo{};
		vkInfo.tempArena = &info.tempArena;

		_app = CreateApp(vkInfo);
	}

	void RenderModule::OnExit(Info& info)
	{
		vkinit::DestroyApp(_app);

		Module::OnExit(info);
	}
}
