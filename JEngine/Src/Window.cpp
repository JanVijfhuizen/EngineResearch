#include "pch.h"
#include "Window.h"

namespace je::engine
{
	bool running = false;
	GLFWwindow* window = nullptr;
	int GLFWwindowMouseState;

	void GLFWKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
	{
		const auto engine = reinterpret_cast<IWindow*>(glfwGetWindowUserPointer(window));
		engine->OnKeyCallback(key, action);
	}

	void GLFWMouseKeyCallback(GLFWwindow* window, const int button, const int action, const int mods)
	{
		const auto engine = reinterpret_cast<IWindow*>(glfwGetWindowUserPointer(window));
		engine->OnMouseCallback(button, action);
	}

	void GLFWScrollCallback(GLFWwindow* window, const double xOffset, const double yOffset)
	{
		const auto engine = reinterpret_cast<IWindow*>(glfwGetWindowUserPointer(window));
		engine->OnScrollCallback(xOffset, yOffset);
	}

	void FramebufferResizeCallback(GLFWwindow* window, const int width, const int height)
	{
		const auto engine = reinterpret_cast<IWindow*>(glfwGetWindowUserPointer(window));
		engine->OnWindowResized(width, height);
	}

	size_t Run(const CreateInfo& info)
	{
		assert(!running);
		running = true;
		
		const int result = glfwInit();
		assert(result);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, false);

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		// Create window.
		window = glfwCreateWindow(mode->width, mode->height, info.name, monitor, nullptr);
		assert(window);
		glfwSetWindowUserPointer(window, info.window);

		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
		glfwSetKeyCallback(window, GLFWKeyCallback);
		glfwSetMouseButtonCallback(window, GLFWMouseKeyCallback);
		glfwSetScrollCallback(window, GLFWScrollCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		glfwMakeContextCurrent(window);

		bool quit = false;
		while(true)
		{
			// Check for events.
			glfwPollEvents();

			// Check if the user pressed the close button.
			quit = glfwWindowShouldClose(window);
			if (quit)
				break;

			int32_t width = 0, height = 0;
			glfwGetFramebufferSize(window, &width, &height);
			while (width == 0 || height == 0)
			{
				glfwGetFramebufferSize(window, &width, &height);
				glfwWaitEvents();
			}

			info.window->OnBeginFrame(quit);
		}

		glfwDestroyWindow(window);
		glfwTerminate();

		running = false;

		return EXIT_SUCCESS;
	}

	const char** GetRequiredExtensions(size_t& count)
	{
		uint32_t glfwExtensionCount = 0;
		const auto buffer = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		count = static_cast<size_t>(glfwExtensionCount);
		return buffer;
	}
}
