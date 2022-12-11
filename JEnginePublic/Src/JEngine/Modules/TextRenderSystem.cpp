#include "JEngine/pch.h"
#include "JEngine/Modules/TextRenderSystem.h"
#include <cstring>

namespace je
{
	TextRenderSystem::TextRenderSystem(const TextRenderSystemCreateInfo& info) :
		JobSystem<TextRenderTask>(info.capacity, info.chunkCapacity),
		_symbolSize(info.symbolSize),
		_alphabetTextureIndex(info.alphabetTextureIndex),
		_numbersTextureIndex(info.numbersTextureIndex),
		_symbolsTextureIndex(info.symbolsTextureIndex)
	{
	}

	void TextRenderSystem::OnUpdate(engine::Info& info, const Jobs& jobs)
	{
		JobSystem<TextRenderTask>::OnUpdate(info, jobs);
		
		const auto baseRenderSys = info.finder.Get<BasicRenderSystem>();
		const float symbolPctSize = static_cast<float>(_symbolSize) / static_cast<float>(baseRenderSys->GetAtlasResolution().x);

		const auto alphabet = baseRenderSys->GetSubTexture(_alphabetTextureIndex);
		const auto numbers = baseRenderSys->GetSubTexture(_numbersTextureIndex);
		const auto symbols = baseRenderSys->GetSubTexture(_symbolsTextureIndex);

		BasicRenderTask task{};
		task.subTexture = baseRenderSys->GetSubTexture(_alphabetTextureIndex);

		for (const auto& batch : jobs)
			for (const auto& job : batch)
			{
				assert(job.text);
				task.position = job.position;
				task.scale = glm::vec2(job.scale);
				const float spacing = job.scale * (1.f + static_cast<float>(job.spacing) / _symbolSize);

				const size_t len = strlen(job.text);
				for (size_t i = 0; i < len; ++i)
				{
					const auto& c = job.text[i];

					if(c != ' ')
					{
						const bool isSymbol = c < '0';
						const bool isInteger = !isSymbol && c < 'a';

						// Assert if it's a valid character.
						assert(isInteger ? c >= '0' && c <= '9' : isSymbol ? c >= ',' && c <= '/' : c >= 'a' && c <= 'z');
						const auto position = c - (isInteger ? '0' : isSymbol ? ',' : 'a');

						auto subTexture = isInteger ? numbers : isSymbol ? symbols : alphabet;
						subTexture.lTop.x += symbolPctSize * static_cast<float>(position);
						subTexture.rBot.x = subTexture.lTop.x + symbolPctSize;

						task.subTexture = subTexture;

						if (!baseRenderSys->TryAdd(task))
							goto FINISHED;
					}
					
					task.position.x += spacing;
				}
			}

		FINISHED:
		;
	}
}
