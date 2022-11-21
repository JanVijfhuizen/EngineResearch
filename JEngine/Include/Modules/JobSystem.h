#pragma once
#include "EngineInfo.h"
#include "Module.h"
#include "Jlb/Arena.h"
#include "Jlb/Array.h"
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
		virtual void OnUpdate(engine::Info& info, const Array<T>& tasks, const LinkedList<Array<T>>& overflowingTasks) = 0;
		virtual bool Validate(const T& job);

	private:
		const size_t _constCapacity;
		const size_t _chunkCapacity;
		Arena* _arena = nullptr;
		Arena* _dumpArena = nullptr;
		Array<T> _tasks{};
		LinkedList<Array<T>> _overflowingTasks{};
		size_t _count = 0;

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
		_tasks = CreateArray<T>(_arena, _constCapacity);
		_overflowingTasks = CreateLinkedList<Array<T>>();
	}

	template <typename T>
	void JobSystem<T>::OnExit(engine::Info& info)
	{
		Module::OnExit(info);
	}

	template <typename T>
	JobSystem<T>::~JobSystem()
	{
		DestroyArray(_tasks, _arena);
	}

	template <typename T>
	bool JobSystem<T>::TryAdd(const T& job)
	{
		if (!Validate(job))
			return false;

		size_t index = _count++;
		if(index < _tasks.length)
		{
			_tasks[index] = job;
			return true;
		}

		if(index - _tasks.length > _overflowingTasks.GetCount() * _chunkCapacity)
			LinkedListAdd(_overflowingTasks, _dumpArena, CreateArray<T>(_dumpArena, _chunkCapacity));
		_overflowingTasks[0][index % _chunkCapacity] = job;
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
		OnUpdate(info, _tasks, _overflowingTasks);
		_count = 0;
		_overflowingTasks = CreateLinkedList<Array<T>>();
	}
}
