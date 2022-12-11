#pragma once
#include "BasicRenderSystem.h"

namespace je
{
	struct TextRenderTask final
	{
		const char* text = nullptr;
		glm::vec2 position{};
		float scale = .2f;
		int32_t spacing = -2;
	};

	struct TextRenderSystemCreateInfo final
	{
		size_t symbolSize = 8;
		size_t alphabetTextureIndex = 0;
		size_t numbersTextureIndex = 1;
		size_t symbolsTextureIndex = 2;
		size_t capacity = 64;
		size_t chunkCapacity = 16;
	};

	class TextRenderSystem final : public JobSystem<TextRenderTask>
	{
	public:
		explicit TextRenderSystem(const TextRenderSystemCreateInfo& info = {});

	private:
		const size_t _symbolSize;
		const size_t _alphabetTextureIndex;
		const size_t _numbersTextureIndex;
		const size_t _symbolsTextureIndex;

		void OnUpdate(engine::Info& info, const Jobs& jobs) override;
	};
}
