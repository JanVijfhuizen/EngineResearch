#pragma once
#include "JEngine/EngineInfo.h"
#include "JEngine/Module.h"
#include "Jlb/Arena.h"
#include "Jlb/JVector.h"
#include "Jlb/LinkedList.h"

namespace je
{
	// Base class for systems that execute an array of similar commands.
	// The jobs get removed at the end of every frame, after they've been iterated upon.
	template <typename T>
	class JobSystem : public Module
	{
	public:
		JobSystem(size_t capacity, size_t chunkCapacity);
		void OnBegin(engine::Info& info) override;
		void OnExit(engine::Info& info) override;

		[[nodiscard]] bool TryAdd(const T& job);

		[[nodiscard]] size_t GetCapacity() const;
		[[nodiscard]] size_t GetCount() const;

	protected:
		using Batch = Vector<T>;
		using Jobs = LinkedList<Batch>;

		virtual void OnUpdate(engine::Info& info, const Jobs& jobs);
		virtual bool Validate(const T& job);

		[[nodiscard]] T* GetData() const;

	private:
		const size_t _capacity;
		const size_t _chunkCapacity;
		Arena* _arena = nullptr;
		Arena* _frameArena = nullptr;
		Batch _mainBatch{};
		LinkedList<Batch> _additionalBatches{};

		void OnUpdate(engine::Info& info) override;
		void OnPostUpdate(engine::Info& info) override;
	};

	template <typename T>
	JobSystem<T>::JobSystem(const size_t capacity, const size_t chunkCapacity) :
		_capacity(capacity), _chunkCapacity(chunkCapacity)
	{
		
	}

	template <typename T>
	void JobSystem<T>::OnBegin(engine::Info& info)
	{
		Module::OnBegin(info);
		_arena = &info.arena;
		_frameArena = &info.frameArena;
		_mainBatch = CreateVector<T>(*_arena, _capacity);
		_additionalBatches = CreateLinkedList<Vector<T>>();
	}

	template <typename T>
	void JobSystem<T>::OnExit(engine::Info& info)
	{
		DestroyVector(_mainBatch, *_arena);
		Module::OnExit(info);
	}

	template <typename T>
	bool JobSystem<T>::TryAdd(const T& job)
	{
		if (!Validate(job))
			return false;

		if(_mainBatch.count < _mainBatch.length)
		{
			_mainBatch.Add(job);
			return true;
		}

		if(_additionalBatches.GetCount() == 0 || _additionalBatches[0].count == _additionalBatches[0].length)
		{
			if (_chunkCapacity == 0)
				return false;
			LinkedListAdd(_additionalBatches, *_frameArena, CreateVector<T>(*_frameArena, _chunkCapacity));
		}
			
		_additionalBatches[0].Add(job);
		return true;
	}

	template <typename T>
	size_t JobSystem<T>::GetCapacity() const
	{
		return _capacity;
	}

	template <typename T>
	size_t JobSystem<T>::GetCount() const
	{
		return _mainBatch.count;
	}

	template <typename T>
	void JobSystem<T>::OnUpdate(engine::Info& info, const Jobs& jobs)
	{
	}

	template <typename T>
	bool JobSystem<T>::Validate(const T& job)
	{
		return true;
	}

	template <typename T>
	T* JobSystem<T>::GetData() const
	{
		return _mainBatch.data;
	}

	template <typename T>
	void JobSystem<T>::OnUpdate(engine::Info& info)
	{
		Module::OnUpdate(info);

		// Adjust linked list to also host the original tasks.
		LinkedList<Vector<T>> node{};
		node.instance = _mainBatch;
		node.next = _additionalBatches.next;
		_additionalBatches.next = &node;
		
		OnUpdate(info, _additionalBatches);
	}

	template <typename T>
	void JobSystem<T>::OnPostUpdate(engine::Info& info)
	{
		Module::OnPostUpdate(info);

		_mainBatch.Clear();
		// It's in the frame arena so it will get de allocated correctly anyway.
		_additionalBatches = CreateLinkedList<Vector<T>>();
	}
}
