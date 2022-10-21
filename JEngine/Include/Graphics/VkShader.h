#pragma once
#include "Jlb/StringView.h"

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		class Shader final
		{
		public:
			enum class Stage
			{
				vertex,
				fragment
			};

			[[nodiscard]] VkShaderModule operator[](Stage stage) const;

			Shader() = default;
			Shader(Arena& tempArena, const App& app, StringView vertexPath, StringView fragmentPath);
			Shader(Shader&& other) noexcept;
			Shader& operator=(Shader&& other) noexcept;
			~Shader();

		private:
			VkShaderModule _modules[2];
			const App* _app = nullptr;
		};
	}
}
