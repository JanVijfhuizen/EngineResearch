#include "pch.h"
#include "ECS/Archetype.h"

namespace je::ecs
{
	size_t Archetype::Remove(const size_t index)
	{
		--_count;

		const size_t c = _batches.GetCount();

		const auto& dstBatch = _batches[c - index / _capacity];
		const auto& srcBatch = _batches[c - _count / _capacity];

		const size_t dstIndex = index % _capacity;
		const size_t srcIndex = _count % _capacity;

		for (size_t i = 0; i < _sizes.length; ++i)
		{
			auto dstPtr = static_cast<unsigned char*>(dstBatch.components[i]);
			auto srcPtr = static_cast<unsigned char*>(srcBatch.components[i]);

			const size_t size = _sizes[i];
			dstPtr += size * dstIndex;
			srcPtr += size * srcIndex;

			memcpy(dstPtr, srcPtr, size);
		}

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
