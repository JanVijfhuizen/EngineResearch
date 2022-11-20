#pragma once

namespace je
{
	class Engine;

	namespace engine
	{
		struct Info;
	}

	// Modules are dynamic parts of the engine that can communicate with each other.
	// Examples can be: windowing, rendering, collision system, etc.
	class Module
	{
	public:
		virtual ~Module() = default;

	private:
		friend Engine;

	protected:
		virtual void OnInitialize(engine::Info& info);
		virtual void OnBegin(engine::Info& info);
		virtual void OnUpdate(engine::Info& info);
		virtual void OnExit(engine::Info& info);
	};
}