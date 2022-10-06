#include "pch.h"
#include "WindowModule.h"

#include "EngineInfo.h"

namespace je::engine
{
	class WindowInternal final
	{
	public:
		static void GLFWKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
		{
			const auto engine = static_cast<WindowModule*>(glfwGetWindowUserPointer(window));
			if (engine->onKeyCallback)
				engine->onKeyCallback(key, action);
		}

		static void GLFWMouseKeyCallback(GLFWwindow* window, const int button, const int action, const int mods)
		{
			const auto engine = static_cast<WindowModule*>(glfwGetWindowUserPointer(window));
			if (engine->onMouseCallback)
				engine->onMouseCallback(button, action);
		}

		static void GLFWScrollCallback(GLFWwindow* window, const double xOffset, const double yOffset)
		{
			const auto engine = static_cast<WindowModule*>(glfwGetWindowUserPointer(window));
			if (engine->onScrollCallback)
				engine->onScrollCallback({ xOffset, yOffset });
		}

		static void FrameBufferResizeCallback(GLFWwindow* window, const int width, const int height)
		{
			const auto engine = static_cast<WindowModule*>(glfwGetWindowUserPointer(window));
			if (engine->onWindowResized)
				engine->onWindowResized({ width, height });
		}

		GLFWwindow* window = nullptr;
		bool running = false;
	} internalWindow;

	void WindowModule::OnBegin(EngineInfo& info)
	{
		Module::OnBegin(info);

		assert(!internalWindow.running);
		internalWindow.running = true;

		const int result = glfwInit();
		assert(result);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, false);

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		// Create window.
		auto& window = internalWindow.window;
		auto& overrideResolution = createInfo.overrideResolution;
		const bool fullscreen = overrideResolution.x == 0 && overrideResolution.y == 0;
		overrideResolution = fullscreen ? glm::ivec2{ mode->width, mode->height } : overrideResolution;
		window = glfwCreateWindow(overrideResolution.x, overrideResolution.y, createInfo.name, fullscreen ? monitor : nullptr, nullptr);
		assert(window);
		glfwSetWindowUserPointer(window, this);

		glfwSetFramebufferSizeCallback(window, WindowInternal::FrameBufferResizeCallback);
		glfwSetKeyCallback(window, WindowInternal::GLFWKeyCallback);
		glfwSetMouseButtonCallback(window, WindowInternal::GLFWMouseKeyCallback);
		glfwSetScrollCallback(window, WindowInternal::GLFWScrollCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		glfwMakeContextCurrent(window);
	}

	void WindowModule::OnExit(EngineInfo& info)
	{
		glfwDestroyWindow(internalWindow.window);
		glfwTerminate();
		internalWindow.running = false;

		Module::OnExit(info);
	}

	const char** WindowModule::GetRequiredExtensions(size_t& count)
	{
		uint32_t glfwExtensionCount = 0;
		const auto buffer = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		count = static_cast<size_t>(glfwExtensionCount);
		return buffer;
	}

	void WindowModule::OnUpdate(EngineInfo& info)
	{
		Module::OnUpdate(info);

		assert(internalWindow.running);

		// Check for events.
		glfwPollEvents();

		// Check if the user pressed the close button.
		const auto& window = internalWindow.window;
		info.quit = info.quit ? info.quit : glfwWindowShouldClose(window);
		if (info.quit)
			return;

		int32_t width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
	}
}
