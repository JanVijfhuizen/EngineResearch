#include "pch.h"

#include <iostream>

#include "Engine.h"
#include "ECS/Archetype.h"
#include "Modules/JobSystem.h"
#include "ECS/Cecsar.h"
#include "Modules/RenderModule.h"
#include "Modules/SceneModule.h"

struct SomeTask final
{
	size_t number = SIZE_MAX;
};

class SomeSystem final : public je::JobSystem<SomeTask>
{
public:
	SomeSystem(const size_t capacity, const size_t chunkCapacity)
		: JobSystem<SomeTask>(capacity, chunkCapacity)
	{
		
	}

	void OnBegin(je::engine::Info& info) override;

protected:
	void OnUpdate(je::engine::Info& info, const Jobs& jobs) override
	{
		/*
		for (const auto& batch : jobs)
			for (const auto& task : batch)
				std::cout << task.number << std::endl;
		std::cout << "end of frame" << std::endl;
		*/
	}
};

void SomeSystem::OnBegin(je::engine::Info& info)
{
	JobSystem<SomeTask>::OnBegin(info);

	SomeTask aTask{};
	aTask.number = 8;
	SomeTask bTask{};
	bTask.number = 14;
	SomeTask cTask{};
	cTask.number = 1;

	TryAdd(aTask);
	TryAdd(bTask);
	TryAdd(cTask);
}

class MyEngine final : public je::Engine
{
	void DefineAdditionalModules(je::Arena& dumpArena, je::Finder<je::Module>::Initializer& initializer) override
	{
		size_t capacity = 2;
		size_t chunkCapacity = 4;
		initializer.Add<SomeSystem>(capacity, chunkCapacity);

		const auto scenes = je::CreateArray<je::SceneInfo>(dumpArena, 1);

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
	}
};

int main()
{
	struct RenderResources final
	{
		static void DefineResources(je::Arena& arena, const je::vk::App& app, const je::vk::Allocator& allocator, void* userPtr)
		{
			
		}
		static void DestroyResources(je::Arena& arena, const je::vk::App& app, const je::vk::Allocator& allocator, void* userPtr)
		{
			
		}
		static je::Array<je::vk::RenderNode> DefineRenderGraph(je::Arena& dumpArena, void* userPtr)
		{
			return {};
		}
		static void BindRenderGraphResources(const je::Array<je::vk::RenderNode>& nodes, const je::vk::App& app, void* userPtr)
		{
			
		}
	} renderResources;

	je::engine::RenderModuleCreateInfo renderModuleCreateInfo{};
	renderModuleCreateInfo.defineResources = RenderResources::DefineResources;
	renderModuleCreateInfo.destroyResources = RenderResources::DestroyResources;
	renderModuleCreateInfo.defineRenderGraph = RenderResources::DefineRenderGraph;
	renderModuleCreateInfo.bindRenderGraphResources = RenderResources::BindRenderGraphResources;
	renderModuleCreateInfo.userPtr = &renderResources;

	je::EngineRunInfo runInfo{};
	runInfo.renderModuleCreateInfo = &renderModuleCreateInfo;

	MyEngine engine{};
	return static_cast<int>(engine.Run(runInfo));
}
