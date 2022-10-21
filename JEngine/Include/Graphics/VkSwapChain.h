#pragma once
#include "Jlb/Array.h"

namespace je
{
	namespace engine
	{
		class WindowModule;
	}

	class Arena;

	namespace vk
	{
		struct App;

		constexpr size_t SWAPCHAIN_MAX_FRAMES_IN_FLIGHT = 4;

		class SwapChain final
		{
		public:
			SwapChain(Arena& arena, Arena& tempArena, const App& app, const engine::WindowModule& windowModule);
			~SwapChain();

			VkCommandBuffer WaitForImage();
			[[nodiscard]] VkCommandBuffer BeginFrame(bool manuallyCallWaitForImage = false);
			void EndFrame(const View<VkSemaphore>& waitSemaphores = {});

			[[nodiscard]] size_t GetLength() const;

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
			const App& _app;
			const engine::WindowModule& _windowModule;

			VkSurfaceFormatKHR _surfaceFormat{};
			VkPresentModeKHR _presentMode{};
			VkExtent2D _extent{};

			VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
			VkRenderPass _renderPass = VK_NULL_HANDLE;

			Array<Image> _images;
			Array<Frame> _frames;

			size_t _frameIndex = 0;
			uint32_t _imageIndex;

			void Cleanup() const;
			void Recreate();
		};
	}
}
