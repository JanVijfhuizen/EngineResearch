#pragma once
#include "EngineInfo.h"
#include "Module.h"
#include "Jlb/Arena.h"
#include "Jlb/JVector.h"
#include "Jlb/LinkedList.h"

namespace je
{
	template <typename T>
	class JobSystem : public Module
	{
	public:
		JobSystem(size_t constCapacity, size_t chunkCapacity);
		void OnBegin(engine::Info& info) override;
		void OnExit(engine::Info& info) override;

		[[nodiscard]] bool TryAdd(const T& job);

	protected:
		virtual void OnUpdate(engine::Info& info, const LinkedList<Vector<T>>& jobs) = 0;
		virtual bool Validate(const T& job);

	private:
		const size_t _constCapacity;
		const size_t _chunkCapacity;
		Arena* _arena = nullptr;
		Arena* _dumpArena = nullptr;
		Vector<T> _tasks{};
		LinkedList<Vector<T>> _overflowingTasks{};

		void OnUpdate(engine::Info& info) override;
	};

	template <typename T>
	JobSystem<T>::JobSystem(const size_t constCapacity, const size_t chunkCapacity) :
		_constCapacity(constCapacity), _chunkCapacity(chunkCapacity)
	{
		
	}

	template <typename T>
	void JobSystem<T>::OnBegin(engine::Info& info)
	{
		Module::OnBegin(info);
		_arena = &info.persistentArena;
		_dumpArena = &info.dumpArena;
		_tasks = CreateVector<T>(*_arena, _constCapacity);
		_overflowingTasks = CreateLinkedList<Vector<T>>();
	}

	template <typename T>
	void JobSystem<T>::OnExit(engine::Info& info)
	{
		DestroyVector(_tasks, *_arena);
		Module::OnExit(info);
	}

	template <typename T>
	bool JobSystem<T>::TryAdd(const T& job)
	{
		if (!Validate(job))
			return false;

		if(_tasks.count < _tasks.length)
		{
			_tasks.Add(job);
			return true;
		}

		if(_overflowingTasks.GetCount() == 0 || _overflowingTasks[0].count == _overflowingTasks[0].length)
		{
			if (_chunkCapacity == 0)
				return false;
			LinkedListAdd(_overflowingTasks, *_dumpArena, CreateVector<T>(*_dumpArena, _chunkCapacity));
		}
			
		_overflowingTasks[0].Add(job);
		return true;
	}

	template <typename T>
	bool JobSystem<T>::Validate(const T& job)
	{
		return true;
	}

	template <typename T>
	void JobSystem<T>::OnUpdate(engine::Info& info)
	{
		Module::OnUpdate(info);

		// Adjust linked list to also host the original tasks.
		LinkedList<Vector<T>> node{};
		node.instance = _tasks;
		node.next = _overflowingTasks.next;
		_overflowingTasks.next = &node;
		
		OnUpdate(info, _overflowingTasks);
		_tasks.Clear();
		// It's in the dump arena so it will get deallocated correctly anyway.
		_overflowingTasks = CreateLinkedList<Vector<T>>();
	}
}
