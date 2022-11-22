#include "pch.h"
#include "ECS/Archetype.h"

namespace je::ecs
{
	const Archetype::Batch& Archetype::Iterator::operator*() const
	{
		return linked->instance;
	}

	const Archetype::Batch& Archetype::Iterator::operator->() const
	{
		return linked->instance;
	}

	const Archetype::Iterator& Archetype::Iterator::operator++()
	{
		linked = linked->next;
		count = capacity;
		return *this;
	}

	Archetype::Iterator Archetype::Iterator::operator++(int)
	{
		Iterator temp{};
		temp.linked = linked;
		temp.count = count;
		temp.capacity = capacity;
		linked = linked->next;
		count = capacity;
		return temp;
	}

	size_t Archetype::Remove(const size_t index)
	{
		--_count;

		const size_t c = _batches.GetCount();

		const auto& dstBatch = _batches[c - 1 - index / _capacity];
		auto& srcBatch = _batches[c - 1 - _count / _capacity];
		--srcBatch.count;

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

	Archetype::Iterator Archetype::begin() const
	{
		Iterator it{};
		it.linked = _batches.next;
		it.count = (_capacity + _count) % _capacity;
		it.capacity = _capacity;
		return it;
	}

	Archetype::Iterator Archetype::end()
	{
		return {};
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
