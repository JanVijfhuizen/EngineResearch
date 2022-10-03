#pragma once
#include "JMove.h"

namespace je
{
	// Additional memory required for arena allocations.
	constexpr static size_t ARENA_META_DATA_SIZE = sizeof(size_t);

	// Linear/stack allocator.
	class Arena final
	{
	public:
		// Local scope for arena.
		// Allocations done within the lifetime of the scope will be automatically cleared on destruction.
		struct Scope final
		{
			friend Arena;

			~Scope();

		private:
			Scope();

			Arena* _arena = nullptr;
			size_t _current = SIZE_MAX;
		};

		explicit Arena(void* ptr, size_t size);
		~Arena();
		
		[[nodiscard]] void* Alloc(size_t size);
		// Does not call destructors.
		void Free(void* ptr);

		// Allocate N objects of type T. Calls default constructors.
		template <typename T>
		[[nodiscard]] T* New(size_t count = 1);

		[[nodiscard]] Scope CreateScope();

		void Empty();
		[[nodiscard]] bool IsEmpty() const;

	private:
		void* _ptr = nullptr;
		size_t _size = 0;
		size_t _current = 0;
		Arena* _next = nullptr;
	};

	template <typename T>
	T* Arena::New(const size_t count)
	{
		void* ptr = Alloc(sizeof(T) * count);
		T* ptrType = static_cast<T*>(ptr);
		for (size_t i = 0; i < count; ++i)
			ptrType[i] = T();
		return ptrType;
	}
}

