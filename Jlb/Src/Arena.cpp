#include "jpch.h"
#include "Jlb/Arena.h"
#include "Jlb/JMath.h"
#include "Jlb/JMove.h"

namespace je
{
	Arena::Scope::~Scope()
	{
		if (!_arena)
			return;

		Arena* next = _arena->_next;
		while(next)
		{
			next->_current = 0;
			next = next->_next;
		}
		_arena->_current = _current;
		--_scopeCount;
	}

	Arena::Scope::Scope(Arena& arena, size_t& scopeCount, const size_t current) :
		_arena(&arena), _scopeCount(scopeCount), _current(current)
	{
		++_scopeCount;
	}

	Arena::Scope::Scope(Scope&& other) noexcept :
		_arena(other._arena), _scopeCount(other._scopeCount), _current(other._current)
	{
		other._arena = nullptr;
	}

	Arena::Arena(void* ptr, const size_t size) : _ptr(ptr), _size(size)
	{
	}

	Arena::~Arena()
	{
		if(_next)
		{
			free(_next->_ptr);
			delete _next;
		}
	}

	void* Arena::Alloc(size_t size)
	{
		size = math::Max(size, sizeof(size_t));

		// If the arena doesn't have enough space, create a cascaded arena OR use an already available cascaded one.
		if(size + sizeof(size_t) > _size - _current || _next && _next->_current > 0)
		{
			if (!_next)
			{
				const size_t newSize = math::Max(_size, size + sizeof(size_t));
				void* ptr = malloc(newSize);
				_next = new Arena(ptr, newSize);
			}

			return _next->Alloc(size);
		}

		// Allocate new chunk of memory and change current pointer.
		void* ptrSize = static_cast<unsigned char*>(_ptr) + _current;
		*static_cast<size_t*>(ptrSize) = size + sizeof(size_t);
		void* ptr = static_cast<unsigned char*>(ptrSize) + sizeof(size_t);
		_current += size + sizeof(size_t);
		return ptr;
	}

	bool Arena::Free(void* ptr)
	{
		if (_scopeCount > 0)
			return false;

		if(_next && _next->_current > 0)
			return _next->Free(ptr);

		void* ptrSize = static_cast<unsigned char*>(ptr) - sizeof(size_t);
		const size_t size = *static_cast<size_t*>(ptrSize);

#ifdef _DEBUG
		const void* ptrCurrent = static_cast<unsigned char*>(_ptr) + _current - size + sizeof(size_t);
		assert(ptrCurrent == ptr);
#endif
		
		_current -= size;
		return true;
	}

	Arena::Scope Arena::CreateScope()
	{
		const Arena* next = this;
		while (next->_next && next->_next->_current > 0)
			next = next->_next;

		Scope scope{*this, _scopeCount, _current};
		return Move(scope);
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
