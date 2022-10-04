#include "pch.h"
#include "Engine.h"
#include "Window.h"

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
	MyEngine engine{};
	return static_cast<int>(engine.Run());
}
