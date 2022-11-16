#pragma once
#include "Arena.h"
#include "JMap.h"
#include "KeyPair.h"
#include "LinkedList.h"

namespace je 
{
	template <typename T>
	class Finder final
	{
		struct Initializer final 
		{
			friend Finder;

			template <typename U>
			void Add();

		private:
			Finder* _finder = nullptr;
		};

		Arena& _arena;
		LinkedList<KeyPair<T*>> _linkedList{};
		Map<T*> _map{};

		Finder(Arena& arena, Initializer& outInitializer);
		~Finder();

		void Compile();
	};

	template <typename T>
	template <typename U>
	void Finder<T>::Initializer::Add()
	{
		assert(_finder && !_finder->_map.data);
		KeyPair<T*> pair{};
		pair.value = _finder._arena.New<U>();
		pair.key = typeid(T).hash_code();
		LinkedListAdd(_finder._linkedList, _finder._arena, pair);
	}

	template <typename T>
	Finder<T>::Finder(Arena& arena, Initializer& outInitializer) : _arena(arena)
	{
		_linkedList = CreateLinkedList<KeyPair<T*>>();
		outInitializer._finder = this;
	}

	template <typename T>
	Finder<T>::~Finder()
	{
		if (_map.data)
			DestroyMap(_map, _arena);
		DestroyLinkedList(_linkedList);
	}

	template <typename T>
	void Finder<T>::Compile()
	{
		assert(!_map.data);
		_map = CreateMap<T*>(_arena, _linkedList.GetCount());
		for (auto& instance : _linkedList)
			_map.Insert(instance.value, instance.key);
	}
}
