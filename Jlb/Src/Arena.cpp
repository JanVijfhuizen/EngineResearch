#include "jpch.h"
#include "Jlb/Arena.h"
#include "Jlb/JMath.h"

namespace je
{
	Arena::Scope::~Scope()
	{
		Arena* next = _arena->_next;
		while(next)
		{
			next->_current = 0;
			next = next->_next;
		}
		_arena->_current = _current;
	}

	Arena::Scope::Scope() = default;

	Arena::Arena(void* ptr, const size_t size) : _ptr(ptr), _size(size)
	{
	}

	Arena::~Arena()
	{
		// Only free the cascaded arenas.
		const Arena* next = _next;
		while(next)
		{
			void* ptr = next->_ptr;
			free(ptr);

			const Arena* current = next;
			next = next->_next;

			delete current;
		}
	}

	void* Arena::Alloc(size_t size)
	{
		size = math::Max(size, sizeof(size_t)) + sizeof(size_t);

		// If the arena doesn't have enough space, create a cascaded arena OR use an already available cascaded one.
		if(size > _size - _current || _next && _next->_current > 0)
		{
			if (!_next)
			{
				const size_t newSize = math::Max(_size, size);
				void* ptr = malloc(newSize);
				_next = new Arena(ptr, newSize);
			}

			return _next->Alloc(size);
		}

		// Allocate new chunk of memory and change current pointer.
		void* ptrSize = static_cast<unsigned char*>(_ptr) + _current;
		*static_cast<size_t*>(ptrSize) = size;
		void* ptr = static_cast<unsigned char*>(ptrSize) + sizeof(size_t);
		_current += size;
		return ptr;
	}

	void Arena::Free(void* ptr)
	{
		if(_next && _next->_current > 0)
		{
			_next->Free(ptr);
			return;
		}

		void* ptrSize = static_cast<unsigned char*>(ptr) - sizeof(size_t);
		const size_t size = *static_cast<size_t*>(ptrSize);

#ifdef _DEBUG
		const void* ptrCurrent = static_cast<unsigned char*>(_ptr) + _current - size + sizeof(size_t);
		assert(ptrCurrent == ptr);
#endif
		
		_current -= size;
	}

	Arena::Scope Arena::CreateScope()
	{
		Arena* next = this;
		while (next->_next && next->_next->_current > 0)
			next = next->_next;

		Scope scope{};
		scope._arena = next;
		scope._current = _current;
		return scope;
	}

	void Arena::Empty()
	{
		Arena* next = this;
		while (next)
		{
			next->_current = 0;
			next = next->_next;
		}
	}

	bool Arena::IsEmpty() const
	{
		const Arena* next = this;
		while(next)
		{
			if (next->_current > 0)
				return false;
			next = next->_next;
		}
		return true;
	}
}
