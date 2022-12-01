#pragma once
#include "Arena.h"
#include "Array.h"
#include "JMap.h"
#include "KeyPair.h"
#include "LinkedList.h"
#include "vcruntime_typeinfo.h"

namespace je 
{
	template <typename T>
	class Finder final
	{
	public:
		class Initializer final 
		{
			friend Finder;

		public:
			template <typename U, typename ...Args>
			U& Add(Args&... args);

			Initializer(Arena& arena, Arena& tempArena);
			~Initializer();

		private:
			Arena& _arena;
			Arena& _tempArena;
			LinkedList<KeyPair<T*>> _linkedList{};
		};

		explicit Finder(Arena& arena);
		~Finder();

		void Compile(const Initializer& initializer);

		template <typename U>
		U* Get() const;

		[[nodiscard]] Iterator<T*> begin();
		[[nodiscard]] Iterator<T*> end();

	private:
		Arena& _arena;
		Array<T*> _array{};
		Map<T*> _map{};
	};

	template <typename T>
	template <typename U, typename ... Args>
	U& Finder<T>::Initializer::Add(Args&... args)
	{
		KeyPair<T*> pair{};
		U* value = _arena.New<U>(1, args...);
		pair.value = value;
		pair.key = typeid(U).hash_code();

#ifdef _DEBUG
		for (auto& keyPair : _linkedList)
			assert(keyPair.key != pair.key);
#endif

		LinkedListAdd(_linkedList, _tempArena, pair);
		return *value;
	}

	template <typename T>
	Finder<T>::Initializer::Initializer(Arena& arena, Arena& tempArena) : _arena(arena), _tempArena(tempArena)
	{
		_linkedList = CreateLinkedList<KeyPair<T*>>();
	}

	template <typename T>
	Finder<T>::Initializer::~Initializer()
	{
		DestroyLinkedList(_linkedList, _tempArena);
	}

	template <typename T>
	Finder<T>::Finder(Arena& arena) : _arena(arena)
	{
		
	}

	template <typename T>
	Finder<T>::~Finder()
	{
		if (!_map.data)
			return;
		DestroyMap(_map, _arena);
		DestroyArray(_array, _arena);
	}

	template <typename T>
	void Finder<T>::Compile(const Initializer& initializer)
	{
		assert(!_map.data);
		const size_t length = initializer._linkedList.GetCount();
		_array = CreateArray<T*>(_arena, length);
		_map = CreateMap<T*>(_arena, length);

		size_t i = length - 1;
		for (auto& linked : initializer._linkedList)
		{
			_array[i--] = linked.value;
			_map.Insert(linked.value, linked.key);
		}
	}

	template <typename T>
	template <typename U>
	U* Finder<T>::Get() const
	{
		Module* mod = *_map.Contains(typeid(U).hash_code());
		assert(mod);
		return static_cast<U*>(mod);
	}

	template <typename T>
	Iterator<T*> Finder<T>::begin()
	{
		return _array.begin();
	}

	template <typename T>
	Iterator<T*> Finder<T>::end()
	{
		return _array.end();
	}
}
