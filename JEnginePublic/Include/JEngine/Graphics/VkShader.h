#pragma once

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		[[nodiscard]] VkShaderModule CreateShaderModule(Arena& tempArena, const App& app, const char* path);
	}
}
