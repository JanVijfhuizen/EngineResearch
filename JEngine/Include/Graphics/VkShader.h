#pragma once
#include "Jlb/Arena.h"
#include "Jlb/StringView.h"

namespace je::vk
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

		Shader(Arena& arena, const App& app, StringView vertexPath, StringView fragmentPath);
		Shader(Shader&& other) noexcept;
		~Shader();

	private:
		VkShaderModule _modules[2];
		const App* _app;
	};
}
