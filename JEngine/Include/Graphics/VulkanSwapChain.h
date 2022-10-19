#pragma once
#include "Jlb/Array.h"

namespace je
{
	namespace engine
	{
		class WindowModule;
	}

	class Arena;
	struct VulkanApp;

	constexpr size_t SWAPCHAIN_MAX_FRAMES_IN_FLIGHT = 4;

	class VulkanSwapChain final
	{
	public:
		VulkanSwapChain(Arena& arena, Arena& tempArena, const VulkanApp& app, const engine::WindowModule& windowModule);
		~VulkanSwapChain();

	private:
		struct Image final
		{
			
		};

		struct Frame final
		{
			
		};

		Arena& _arena;
		Arena& _tempArena;
		const VulkanApp& _app;
		const engine::WindowModule& _windowModule;

		VkSurfaceFormatKHR _surfaceFormat{};
		VkPresentModeKHR _presentMode{};
		VkExtent2D _extent{};

		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkRenderPass _renderPass = VK_NULL_HANDLE;

		Array<Image>* _images = nullptr;
		Array<Frame>* _frames = nullptr;

		void Cleanup();
		void Recreate();
	};
}
