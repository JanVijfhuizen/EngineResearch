#pragma once
#include "Module.h"
#include "Jlb/StringView.h"

namespace je::engine
{
	class Window : public Module
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

		void OnBegin(EngineInfo& info) override;
		void OnExit(EngineInfo& info) override;

		[[nodiscard]] static const char** GetRequiredExtensions(size_t& count);

	protected:
		void OnUpdate(EngineInfo& info) override;
	};
}
