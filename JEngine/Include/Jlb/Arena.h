#pragma once

namespace je
{
	constexpr static size_t ARENA_META_DATA_SIZE = sizeof(size_t);

	class Arena final
	{
	public:
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
		void Free(void* ptr);

		template <typename T>
		[[nodiscard]] T* New(size_t count = 1);

		[[nodiscard]] Scope CreateScope();

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
			ptrType[i] = {};
		return ptrType;
	}
}

