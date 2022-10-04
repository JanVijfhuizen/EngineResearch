#include "pch.h"
#include "Engine.h"
#include "Window.h"
#include "Jlb/JVector.h"
#include "Jlb/Stack.h"

class SomeModule final : public je::Module
{
	
};

class MyEngine final : public je::Engine
{
protected:
	void DefineAdditionalModules(const Initializer& initializer) override
	{
		initializer.AddModule<SomeModule>();
	}
};

int main()
{
	void* mem = malloc(4096);
	je::Arena arena{mem, 4096};
	je::Array<char> arr{arena, 12};
	je::Vector<char> arr2{arena, 16};
	je::Vector<char> c = Move(arr2);
	je::Stack<char> st{arena, 16};
	je::Stack<char> st2 = Move(st);

	MyEngine engine{};
	return static_cast<int>(engine.Run());
}
