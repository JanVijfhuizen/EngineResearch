#include "pch.h"

#include <iostream>

#include "Window.h"
#include "Jlb/Heap.h"
#include "Jlb/JMap.h"
#include "Jlb/JMove.h"
#include "Jlb/JString.h"
#include "Jlb/Queue.h"
#include "Jlb/Stack.h"
#include "Jlb/StringView.h"

int main()
{
	struct alignas(32) S
	{
		float f = 2;
		bool b = true;
	};

	void* src = malloc(4096);
	je::Arena arena{src, 4096};

	S* s = arena.New<S>();
	s->f = 4096;
	void* a = arena.Alloc(32);
	{
		const auto _ = arena.CreateScope();
		void* b = arena.Alloc(32);
		void* c = arena.Alloc(32);
	}
	
	arena.Free(a);
	arena.Free(s);

	je::Array<float> f{ arena, 8 };
	je::View<float> view = f;
	for (int i = 0; i < 8; ++i)
	{
		view[i] = i;
	}
	je::Array<float> g = je::Move(f);

	const je::String string{arena, je::StringView("Hello")};
	std::cout << string.GetStringView() << std::endl;

	je::Map<float> map{arena, 12};
	map.Insert(27.2f, 2);
	assert(map.Contains(2));
	assert(!map.Contains(4));

	je::Stack<float> stack{arena, 4};
	stack.Push(2);
	stack.Push(3);
	stack.Push(4);
	for (const auto& val : stack.GetView())
	{
		std::cout << val << std::endl;
	}

	je::Stack<float> s2 = Move(stack);

	je::Queue<float> q{ arena, 24 };
	q.Enqueue(3);
	je::Queue<float> q2 = Move(q);
	q2.Enqueue(5);

	je::Heap<float> heap{arena, 7};
	heap.Insert(52.34f, 8);
	assert(heap.Peek() > 52.f);

	je::engine::Window window{"hi", {800, 600}};

	bool quit = false;
	while(!quit)
		window.BeginFrame(quit);
}
