#include "pch.h"
#include <iostream>

#include "JEngine/Engine.h"
#include "JEngine/Archetype.h"
#include "JEngine/Modules/JobSystem.h"
#include "JEngine/Cecsar.h"
#include "JEngine/Modules/SceneModule.h"
#include "JEngine/Modules/TimeModule.h"
#include "Modules/BasicRenderSystem.h"

class DemoSys final : public je::Module
{
protected:
	void OnUpdate(je::engine::Info& info) override
	{
		const float t = info.finder.Get<je::engine::TimeModule>()->GetTime();

		game::BasicRenderTask task{};
		auto res = info.finder.Get<game::BasicRenderSystem>()->TryAdd(task);
		task.position.x = sin(t * .01f);
		task.scale = glm::vec2(.5f + cos(t * .02f) * .25f);
		task.color = glm::vec4(sin(t * .03f), cos(t * .01f), sin(t * .02f), 1);
		task.subTexture.rBot *= .8f + sin(t * 0.05f) * .1f;
		res = info.finder.Get<game::BasicRenderSystem>()->TryAdd(task);
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
		constexpr size_t renderCapacity = 256;
		initializer.Add<DemoSys>();
		initializer.Add<game::BasicRenderSystem>(renderCapacity);
	};

	je::Engine engine{};
	return static_cast<int>(engine.Run(runInfo)); 
}