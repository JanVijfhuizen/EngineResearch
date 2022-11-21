#include "pch.h"

#include <iostream>

#include "Engine.h"
#include "Modules/JobSystem.h"

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
