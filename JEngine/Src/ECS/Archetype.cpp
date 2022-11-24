#include "pch.h"
#include "ECS/Archetype.h"

namespace je::ecs
{
	size_t Archetype::Remove(const size_t index)
	{
		--_count;

		const size_t c = _batches.GetCount();

		const auto& dstBatch = _batches[c - 1 - index / _capacity];
		const auto& srcBatch = _batches[c - 1 - _count / _capacity];

		const size_t dstIndex = index % _capacity;
		const size_t srcIndex = _count % _capacity;

		for (size_t i = 0; i < _sizes.length; ++i)
		{
			auto dstPtr = static_cast<unsigned char*>(dstBatch[i]);
			auto srcPtr = static_cast<unsigned char*>(srcBatch[i]);

			const size_t size = _sizes[i];
			dstPtr += size * dstIndex;
			srcPtr += size * srcIndex;

			memcpy(dstPtr, srcPtr, size);
		}

		return _count;
	}

	void Archetype::AddBatch()
	{
		const auto batch = _arena->New<void*>(_sizes.length);
		for (size_t i = 0; i < _sizes.length; ++i)
			batch[i] = _arena->Alloc(_sizes[i] * _capacity);

		LinkedListAdd(_batches, *_arena, batch);
	}
}
