#pragma once
#include "JEngine/Module.h"

namespace je::engine
{
	// Window module that manages platform specific windowing.
	class WindowModule final : public Module
	{
	public:
		void (*onWindowResized)(glm::ivec2 resolution) = nullptr;
		void (*onKeyCallback)(size_t key, size_t action) = nullptr;
		void (*onMouseCallback)(size_t key, size_t action) = nullptr;
		void (*onScrollCallback)(glm::vec<2, double> offset) = nullptr;

		[[nodiscard]] static const char** GetRequiredExtensions(size_t& count);
		[[nodiscard]] static VkSurfaceKHR CreateSurface(VkInstance instance);

		[[nodiscard]] glm::ivec2 GetResolution() const;
		void SetResolution(const glm::ivec2& resolution);
		void SetName(const char* name);

	private:
		const char* _name = "JEngine";
		glm::ivec2 _resolution{800, 600};

		void OnInitialize(Info& info) override;
		void OnUpdate(Info& info) override;
		void OnExit(Info& info) override;
	};
}
