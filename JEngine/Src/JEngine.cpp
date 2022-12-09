#include "pch.h"
#include <iostream>

#include "JEngine/Engine.h"
#include "JEngine/Archetype.h"
#include "JEngine/Modules/JobSystem.h"
#include "JEngine/Cecsar.h"
#include "JEngine/Modules/SceneModule.h"
#include "Modules/BasicRenderSystem.h"

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
		initializer.Add<game::BasicRenderSystem>(renderCapacity);
	};

	je::Engine engine{};
	return static_cast<int>(engine.Run(runInfo)); 
}