#pragma once
#include "Module.h"
#include "Jlb/StringView.h"

namespace je::engine
{
	// Window module that manages platform specific windowing.
	class WindowModule final : public Module
	{
	public:
		struct CreateInfo final
		{
			StringView name = "Window";
			glm::ivec2 overrideResolution{800, 600};
		} createInfo{};

		void (*onWindowResized)(glm::ivec2 resolution) = nullptr;
		void (*onKeyCallback)(size_t key, size_t action) = nullptr;
		void (*onMouseCallback)(size_t key, size_t action) = nullptr;
		void (*onScrollCallback)(glm::vec<2, double> offset) = nullptr;

		[[nodiscard]] static const char** GetRequiredExtensions(size_t& count);

	private:
		void OnBegin(Info& info) override;
		void OnUpdate(Info& info) override;
		void OnExit(Info& info) override;
	};
}
