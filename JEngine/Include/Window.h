#pragma once
#include "Jlb/StringView.h"

namespace je::engine
{
	class Window
	{
		friend class WindowInternal;

	public:
		[[nodiscard]] static const char** GetRequiredExtensions(size_t& count);
		void BeginFrame(bool& outQuit);

	protected:
		explicit Window(const StringView& name = "JEngine", glm::ivec2 overrideResolution = {});
		~Window();

		virtual void OnWindowResized(size_t width, size_t height);
		virtual void OnKeyCallback(size_t key, size_t action);
		virtual void OnMouseCallback(size_t key, size_t action);
		virtual void OnScrollCallback(double xOffset, double yOffset);
		virtual void OnBeginFrame(bool& outQuit);
	};
}
