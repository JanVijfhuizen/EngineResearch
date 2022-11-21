#include "pch.h"
#include "ECS/Archetype.h"

namespace je::ecs
{
	size_t Archetype::Remove(const size_t index)
	{
		--_count;

		// WIP.

		return _count;
	}

	void Archetype::AddBatch()
	{
		Batch batch{};

		const auto components = _arena->New<void*>(_sizes.length);
		batch.components = components;
		for (size_t i = 0; i < _sizes.length; ++i)
			components[i] = _arena->Alloc(_sizes[i] * _capacity);

		LinkedListAdd(_batches, *_arena, batch);
	}
}
