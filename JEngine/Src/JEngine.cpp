#include "pch.h"
#include "Engine.h"

class SomeModule final : public je::Module
{
	
};

class MyEngine final : public je::Engine
{
protected:
	void DefineAdditionalModules(je::Finder<je::Module>::Initializer& initializer) override
	{
		initializer.Add<SomeModule>();
	}
};

int main()
{
	MyEngine engine{};
	return static_cast<int>(engine.Run());
}
