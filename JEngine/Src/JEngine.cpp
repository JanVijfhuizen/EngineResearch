#include "pch.h"
#include "Arena.h"

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
}
