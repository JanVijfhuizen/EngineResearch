#pragma once
#include "Array.h"

namespace je
{
	class Arena;

	template <typename T>
	class Pool final
	{
		struct Node;

	public:
		struct Instance final
		{
			friend Pool;

			operator T*() const;

		private:
			Node* _node;
		};

		Pool(Arena& arena, size_t size);

		[[nodiscard]] Instance Get();
		void Put(const Instance& instance);

		[[nodiscard]] size_t GetLength() const;
		[[nodiscard]] View<Node> GetView() const;

	private:
		struct Node final
		{
			friend Pool;

			T value{};
		private:
			Node* _next = nullptr;
		};
		
		Array<Node> _nodes{};
		Node* _open = nullptr;
	};

	template <typename T>
	Pool<T>::Instance::operator T*() const
	{
		return &_node->value;
	}

	template <typename T>
	Pool<T>::Pool(Arena& arena, size_t size) : _nodes(arena, size)
	{
		for (size_t i = size - 1; i > 0; --i)
			_nodes[i - 1]._next = &_nodes[i];
		_open = &_nodes[0];
	}

	template <typename T>
	typename Pool<T>::Instance Pool<T>::Get()
	{
		Node* current = _open;
		_open = _open->_next;
		current->_next = nullptr;
		return { current };
	}

	template <typename T>
	void Pool<T>::Put(const Instance& instance)
	{
		instance._node->_next = _open;
		_open = instance._node;
	}

	template <typename T>
	size_t Pool<T>::GetLength() const
	{
		return _nodes.GetLength();
	}

	template <typename T>
	View<typename Pool<T>::Node> Pool<T>::GetView() const
	{
		return _nodes.GetView();
	}
}
