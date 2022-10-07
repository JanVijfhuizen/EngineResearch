#pragma once
#include "Module.h"
#include "Graphics/VulkanApp.h"

namespace je::engine
{
	class RenderModule final : public Module
	{
	public:
		void OnInitialize(Info& info) override;
		void OnExit(Info& info) override;

	private:
		VulkanApp _app{};
	};
}
