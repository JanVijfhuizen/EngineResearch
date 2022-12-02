#pragma once

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
			
			Scope(Scope&& other) noexcept;
			~Scope();

		private:
			explicit Scope(Arena& arena, size_t& scopeCount, size_t current);

			Arena* _arena = nullptr;
			size_t& _scopeCount;
			size_t _current;
		};
		
		explicit Arena(void* ptr, size_t size);
		~Arena();
		
		[[nodiscard]] void* Alloc(size_t size);
		// Does not call destructors. Returns if the free was successful.
		bool Free(void* ptr);

		// Allocate N objects of type T. Calls default constructors.
		template <typename T, typename ...Args>
		[[nodiscard]] T* New(size_t count = 1, Args&... args);
		template <typename T>
		bool Delete(T* ptr);

		[[nodiscard]] Scope CreateScope();

		// Invalidate all allocations, but keeps the allocated memory to be reused.
		void Empty();
		[[nodiscard]] bool IsEmpty() const;

	private:
		void* _ptr = nullptr;
		size_t _size = 0;
		size_t _current = 0;
		Arena* _next = nullptr;
		size_t _scopeCount = 0;
	};

	template <typename T, typename ...Args>
	T* Arena::New(const size_t count, Args&... args)
	{
		void* ptr = Alloc(sizeof(T) * count);
		T* ptrType = static_cast<T*>(ptr);
		for (size_t i = 0; i < count; ++i)
			new(&ptrType[i]) T(args...);
		return ptrType;
	}

	template <typename T>
	bool Arena::Delete(T* ptr)
	{
		ptr->~T();
		return Free(ptr);
	}
}

