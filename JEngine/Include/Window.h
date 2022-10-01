#pragma once
#include "Jlb/StringView.h"

namespace je::engine
{
	class IWindow
	{
	public:
		virtual void OnWindowResized(size_t width, size_t height) = 0;
		virtual void OnKeyCallback(size_t key, size_t action) = 0;
		virtual void OnMouseCallback(size_t key, size_t action) = 0;
		virtual void OnScrollCallback(double xOffset, double yOffset) = 0;
		virtual void OnBeginFrame(bool& outQuit) = 0;
	};

	struct CreateInfo final
	{
		StringView name = "JEngine";
		IWindow* window = nullptr;
	};

	[[nodiscard]] size_t Run(const CreateInfo& info = {});
	[[nodiscard]] const char** GetRequiredExtensions(size_t& count);
}
