#include "pch.h"
#include "Graphics/VkShader.h"
#include "Graphics/VkApp.h"
#include "Jlb/FileLoader.h"

namespace je::vk
{
	Shader::~Shader()
	{
		if (!_app)
			return;
		for (const auto& mod : _modules)
			vkDestroyShaderModule(_app->device, mod, nullptr);
	}

	VkShaderModule Shader::operator[](Stage stage) const
	{
		return _modules[static_cast<size_t>(stage)];
	}

	Shader::Shader(Arena& tempArena, const App& app, StringView vertexPath, StringView fragmentPath) : _app(&app)
	{
		const auto _ = tempArena.CreateScope();
		const auto vertCode = file::Load(tempArena, vertexPath);
		const auto fragCode = file::Load(tempArena, fragmentPath);

		const View<char> code[2]
		{
			vertCode.GetView(),
			fragCode.GetView()
		};

		for (size_t i = 0; i < 2; ++i)
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code[i].GetLength();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code->GetData());

			const auto result = vkCreateShaderModule(app.device, &createInfo, nullptr, &_modules[i]);
			assert(!result);
		}
	}

	Shader::Shader(Shader&& other) noexcept : _app(other._app)
	{
		for (size_t i = 0; i < 2; ++i)
			_modules[i] = other._modules[i];
		other._app = nullptr;
	}

	Shader& Shader::operator=(Shader&& other) noexcept
	{
		_app = other._app;
		for (size_t i = 0; i < 2; ++i)
			_modules[i] = other._modules[i];
		other._app = nullptr;
		return *this;
	}
}
