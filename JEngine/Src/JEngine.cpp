#include "pch.h"
#include <iostream>

#include "JEngine/Engine.h"
#include "JEngine/Archetype.h"
#include "JEngine/Modules/JobSystem.h"
#include "JEngine/Cecsar.h"
#include "JEngine/Modules/SceneModule.h"
#include "JEngine/Modules/TimeModule.h"
#include "JEngine/Modules/BasicRenderSystem.h"
#include "JEngine/Modules/TextRenderSystem.h"

class DemoSys final : public je::Module
{
protected:
	void OnUpdate(je::engine::Info& info) override
	{
		const float t = info.finder.Get<je::engine::TimeModule>()->GetTime();
		const auto basicRenderSys = info.finder.Get<je::BasicRenderSystem>();
		basicRenderSys->camera.rotation += 0.001f;
		basicRenderSys->camera.zoom = -.4f;

		je::BasicRenderTask task{};
		auto res = basicRenderSys->TryAdd(task);
		task.position.x = sin(t * .01f);
		task.scale = glm::vec2(.5f + cos(t * .02f) * .25f);
		task.color = glm::vec4(sin(t * .03f), cos(t * .01f), sin(t * .02f), 1);
		task.subTexture.rBot *= .8f + sin(t * 0.05f) * .1f;
		task.subTexture = basicRenderSys->GetSubTexture(static_cast<int>(2 + 2.f * sin(t * 0.025)));
		res = basicRenderSys->TryAdd(task);

		je::TextRenderTask tTask{};
		tTask.lineLength = 2;
		tTask.text = "hello 44 .-/,";
		info.finder.Get<je::TextRenderSystem>()->TryAdd(tTask);
	}
};

int main()
{
	je::EngineRunInfo runInfo{};
	runInfo.defineAdditionalModules = [](je::Arena& frameArena, je::Finder<je::Module>::Initializer& initializer)
	{
		const auto scenes = je::CreateArray<je::SceneInfo>(frameArena, 1);

		auto& scene = scenes[0];
		scene.onBegin = [](const je::Finder<je::Module>& finder, je::ecs::Cecsar& cecsar, void* userPtr)
		{
			const size_t testArchetype = cecsar.DefineArchetype<int, float, bool>();
			je::Tuple<je::ecs::Entity, int, float, bool> prototype{ {}, 0, 14, true };

			for (int i = 0; i < 17; ++i)
			{
				++je::Get<1>(prototype);
				cecsar.Add(testArchetype, prototype);
			}

			return true;
		};
		scene.onUpdate = [](const je::Finder<je::Module>& finder, je::ecs::Cecsar& cecsar, void* userPtr)
		{
			struct Info final
			{
				size_t i = 0;
			} in;

			auto scope = cecsar.CreateScope<int>();
			scope.Iterate([&in](int& i)
				{
					std::cout << i << " " << in.i++ << std::endl;
				});

			finder.Get<je::SceneModule>()->Unload(0);
			
			return true;
		};

		initializer.Add<je::SceneModule>(scenes);
		initializer.Add<DemoSys>();

		je::BasicRenderSystemCreateInfo rCreateInfo{};

#ifdef _DEBUG
		rCreateInfo.texturePaths = je::CreateArray<const char*>(frameArena, 7);
		rCreateInfo.texturePaths[0] = "Textures/alphabet.png";
		rCreateInfo.texturePaths[1] = "Textures/numbers.png";
		rCreateInfo.texturePaths[2] = "Textures/symbols.png";
		rCreateInfo.texturePaths[3] = "Textures/humanoid.png";
		rCreateInfo.texturePaths[4] = "Textures/moveArrow.png";
		rCreateInfo.texturePaths[5] = "Textures/bash-card.png";
		rCreateInfo.texturePaths[6] = "Textures/tile.png";
#endif

		initializer.Add<je::TextRenderSystem>();
		initializer.Add<je::BasicRenderSystem>(rCreateInfo);
	};

	je::Engine engine{};
	return static_cast<int>(engine.Run(runInfo)); 
}