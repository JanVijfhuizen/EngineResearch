#include "pch.h"

#include <iostream>

#include "Jlb/Arena.h"
#include "Jlb/Array.h"
#include "Jlb/JMove.h"
#include "Jlb/JString.h"
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
}
