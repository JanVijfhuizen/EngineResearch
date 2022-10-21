#include "pch.h"
#include "Graphics/VkPipeline.h"

namespace je::vk
{
	Pipeline::Pipeline(Arena& tempArena, const App& app)
	{
	}

	Pipeline::Pipeline(Pipeline&& other) noexcept
	{
	}

	Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
	{
		return *this;
	}

	Pipeline::~Pipeline()
	{
	}
}
