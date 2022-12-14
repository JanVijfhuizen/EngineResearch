#include "JEngine/pch.h"
#include "JEngine/Archetype.h"

namespace je::ecs
{
	void Archetype::AddBatch()
	{
		const auto batch = static_cast<void**>(_arena->Alloc(_sizes.length * sizeof(void*)));
		for (size_t i = 0; i < _sizes.length; ++i)
			batch[i] = _arena->Alloc(_sizes[i] * _batchSize);
		LinkedListAdd(_batches, *_arena, batch);
	}

	void Archetype::Remove(const size_t index)
	{
		--_count;

		const size_t c = _batches.GetCount();

		const auto& dstBatch = _batches[c - 1 - index / _batchSize];
		const auto& srcBatch = _batches[c - 1 - _count / _batchSize];

		const size_t dstIndex = index % _batchSize;
		const size_t srcIndex = _count % _batchSize;

		for (size_t i = 0; i < _sizes.length; ++i)
		{
			auto dstPtr = static_cast<unsigned char*>(dstBatch[i]);
			auto srcPtr = static_cast<unsigned char*>(srcBatch[i]);

			const size_t size = _sizes[i];
			dstPtr += size * dstIndex;
			srcPtr += size * srcIndex;

			memcpy(dstPtr, srcPtr, size);
		}
	}
}
