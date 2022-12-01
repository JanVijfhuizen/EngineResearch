#include "pch.h"
#include "JEngine/Graphics/VkShader.h"
#include "JEngine/Graphics/VkApp.h"
#include "Jlb/FileLoader.h"

namespace je::vk
{
	VkShaderModule CreateShaderModule(Arena& tempArena, const App& app, const char* path)
	{
		const auto _ = tempArena.CreateScope();
		const auto code = file::Load(tempArena, path);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		createInfo.codeSize = code.length;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data);

		VkShaderModule mod;
		const auto result = vkCreateShaderModule(app.device, &createInfo, nullptr, &mod);
		assert(!result);
		return mod;
	}
}
