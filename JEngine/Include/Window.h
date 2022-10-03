#pragma once
#include "Jlb/StringView.h"

namespace je::engine
{
	class Window
	{
	public:
		void (*onWindowResized)(glm::ivec2 resolution) = nullptr;
		void (*onKeyCallback)(size_t key, size_t action) = nullptr;
		void (*onMouseCallback)(size_t key, size_t action) = nullptr;
		void (*onScrollCallback)(glm::vec<2, double> offset) = nullptr;

		explicit Window(const StringView& name = "JEngine", glm::ivec2 overrideResolution = {});
		~Window();

		[[nodiscard]] static const char** GetRequiredExtensions(size_t& count);
		void BeginFrame(bool& outQuit);
	};
}
