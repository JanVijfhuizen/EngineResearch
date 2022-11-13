#pragma once
#include "KeyPair.h"
#include "Swap.h"

namespace je
{
	// Allows for quick lookup of values.
	template <typename T>
	struct Map final
	{
		KeyPair<T>* data = nullptr;
		size_t length = 0;
		size_t count = 0;

		void Insert(const T& instance, size_t key);
		[[nodiscard]] T* Contains(size_t key) const;
		void Erase(T& value);

	private:
		[[nodiscard]] size_t GetHash(size_t key) const;
	};

	template <typename T>
	void Map<T>::Erase(T& value)
	{
		size_t index;
		const bool contains = Contains(value, index);
		assert(contains);
		assert(count > 0);
		
		auto& keyPair = data[index];

		// Check how big the key group is.
		size_t i = 1;
		while (i < length)
		{
			const size_t otherIndex = (index + i) % length;
			auto& otherKeyPair = data[otherIndex];
			if (otherKeyPair.key != keyPair.key)
				break;
			++i;
		}

		// Setting the key pair value to the default value.
		keyPair = {};
		// Move the key group one place backwards by swapping the first and last index.
		Swap(data, index, index + i - 1);
		--count;
	}

	template <typename T>
	void Map<T>::Insert(const T& instance, const size_t key)
	{
		assert(count < length);

		// If it already contains this value, replace the old one with the newer value.
		if (Contains(key))
			return;
		
		const size_t hash = GetHash(key);

		for (size_t i = 0; i < length; ++i)
		{
			const size_t index = (hash + i) % length;
			auto& keyPair = data[index];
			// Set to true the first time the key group has been found.
			if (keyPair.key != SIZE_MAX)
				continue;

			keyPair.key = key;
			keyPair.value = instance;
			++count;
			break;
		}
	}

	template <typename T>
	T* Map<T>::Contains(const size_t key) const
	{
		assert(count <= length);

		// Get and use the hash as an index.
		const size_t hash = GetHash(key);

		for (size_t i = 0; i < length; ++i)
		{
			const size_t index = (hash + i) % length;
			auto& keyPair = data[index];

			// If the hash is different, continue.
			if (keyPair.key == key)
				return &keyPair.value;
		}

		return nullptr;
	}

	template <typename T>
	size_t Map<T>::GetHash(const size_t key) const
	{
		return key % length;
	}

	template <typename T>
	[[nodiscard]] Map<T> CreateMap(Arena* arena, const size_t length)
	{
		Map<T> instance{};
		instance.data = arena->New<T>(length);
		instance.length = length;
		return instance;
	}

	template <typename T>
	void DestroyMap(Map<T>* instance, Arena* arena)
	{
		arena->Free(instance->data);
	}
}
