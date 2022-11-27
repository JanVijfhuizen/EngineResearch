#include "pch.h"

#include <iostream>

#include "Engine.h"
#include "ECS/Archetype.h"
#include "Modules/JobSystem.h"
#include "ECS/Cecsar.h"

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
		for (const auto& batch : jobs)
			for (const auto& task : batch)
				std::cout << task.number << std::endl;
		std::cout << "end of frame" << std::endl;
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

	je::ecs::Cecsar cecsar{info.dumpArena};
	const size_t testArchetype = cecsar.DefineArchetype<int, float, bool>();

	je::Tuple<int, float, bool> prototype{0, 14, true};

	for (int i = 0; i < 17; ++i)
	{
		++Get(prototype);
		cecsar.Add(testArchetype, prototype);
	}

	struct Info final
	{
		size_t i = 0;
	} in;

	auto scope = cecsar.CreateScope<int>();
	scope.Iterate([&in](int& i)
		{
			std::cout << i << " " << in.i++ << std::endl;
			return true;
		});

	info.quit = true;
}

class MyEngine final : public je::Engine
{
protected:
	void DefineAdditionalModules(je::Finder<je::Module>::Initializer& initializer) override
	{
		size_t capacity = 2;
		size_t chunkCapacity = 4;
		initializer.Add<SomeSystem>(capacity, chunkCapacity);
	}
};

int main()
{
	MyEngine engine{};
	return static_cast<int>(engine.Run());
}
