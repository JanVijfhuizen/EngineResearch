#pragma once
#include "Module.h"
#include "Graphics/VulkanApp.h"

namespace je
{
	class VulkanSwapChain;
	class Arena;

	class VulkanAllocator;

	namespace engine
	{
		class RenderModule final : public Module
		{
			VulkanApp _app{};
			VulkanAllocator* _allocator = nullptr;
			VulkanSwapChain* _swapChain = nullptr;

			void OnInitialize(Info& info) override;
			void OnExit(Info& info) override;
		};
	}
}
