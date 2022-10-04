#pragma once
#include "Array.h"
#include "KeyPair.h"
#include "Utils.h"

namespace je
{
	// Allows for quick lookup of values.
	template <typename T>
	class Map final : public Array<KeyPair<T>>
	{
	public:
		Map(Arena& arena, size_t length);
		Map(Map<T>&& other) noexcept;

		void Insert(const T& instance, size_t key);

		[[nodiscard]] T* Contains(size_t key) const;
		void Erase(T& value);
		[[nodiscard]] size_t GetCount() const;

	protected:
		[[nodiscard]] size_t GetHash(size_t key) const;

	private:
		size_t _count = 0;
	};

	template <typename T>
	void Map<T>::Erase(T& value)
	{
		size_t index;
		const bool contains = Contains(value, index);
		assert(contains);

		const size_t length = Array<KeyPair<T>>::GetLength();
		assert(_count > 0);

		auto data = Array<KeyPair<T>>::GetData();
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
		--_count;
	}

	template <typename T>
	Map<T>::Map(Arena& arena, size_t length): Array<KeyPair<T>>(arena, length)
	{
	}

	template <typename T>
	Map<T>::Map(Map<T>&& other) noexcept : Array<KeyPair<T>>(Move(other)), _count(other._count)
	{

	}

	template <typename T>
	void Map<T>::Insert(const T& instance, const size_t key)
	{
		const size_t length = Array<KeyPair<T>>::GetLength();
		assert(_count < length);

		// If it already contains this value, replace the old one with the newer value.
		if (Contains(key))
			return;

		auto data = Array<KeyPair<T>>::GetData();

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
			++_count;
			break;
		}
	}

	template <typename T>
	T* Map<T>::Contains(const size_t key) const
	{
		const size_t length = Array<KeyPair<T>>::GetLength();
		assert(_count <= length);

		// Get and use the hash as an index.
		const size_t hash = GetHash(key);

		auto data = Array<KeyPair<T>>::GetData();

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
	size_t Map<T>::GetCount() const
	{
		return _count;
	}

	template <typename T>
	size_t Map<T>::GetHash(const size_t key) const
	{
		return key % Array<KeyPair<T>>::GetLength();
	}
}
