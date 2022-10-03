#include "pch.h"

#include "Engine.h"
#include "Window.h"

class MyEngine final : public je::Engine
{
protected:
	void DefineAdditionalModules(const Initializer& initializer) override
	{
		
	}
};

int main()
{
	je::engine::Window window{"hi", {800, 600}};

	MyEngine engine{};

	bool quit = false;
	while(!quit)
		window.BeginFrame(quit);
}
