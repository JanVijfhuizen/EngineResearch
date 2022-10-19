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

		VkCommandBuffer BeginFrame();
		void EndFrame(const View<VkSemaphore>& waitSemaphores= {});

	private:
		struct Image final
		{
			VkImage image;
			VkImageView view;
			VkCommandBuffer cmdBuffer;
			VkFramebuffer frameBuffer;
			VkFence fence = VK_NULL_HANDLE;
		};

		struct Frame final
		{
			VkSemaphore imageAvailableSemaphore;
			VkSemaphore renderFinishedSemaphore;
			VkFence inFlightFence;
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

		size_t _frameIndex = 0;
		uint32_t _imageIndex;

		void Cleanup() const;
		void Recreate();
	};
}
