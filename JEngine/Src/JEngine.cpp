#include "pch.h"

#include <iostream>

#include "Engine.h"
#include "Window.h"
#include "Jlb/JVector.h"
#include "Jlb/Stack.h"
#include "Jlb/StringArray.h"

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

	std::cout << je::StringView("Hello") << std::endl;
	je::StringArray sr{arena, "Hello" };
	std::cout << sr.GetData() << std::endl;

	MyEngine engine{};
	return static_cast<int>(engine.Run());
}
