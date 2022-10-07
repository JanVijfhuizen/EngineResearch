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
		vkInfo.persistentArena = &info.persistentArena;
		vkInfo.tempArena = &info.tempArena;

		auto app = CreateApp(vkInfo);
	}
}
