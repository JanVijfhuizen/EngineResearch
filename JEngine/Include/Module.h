#pragma once

namespace je
{
	struct EngineInfo;

	// Modules are dynamic parts of the engine that can communicate with each other.
	// Examples can be: windowing, rendering, collision system, etc.
	class Module
	{
	public:
		virtual ~Module() = default;

	private:
		friend class Engine;

	protected:
		virtual void OnInitialize(EngineInfo& info);
		virtual void OnBegin(EngineInfo& info);
		virtual void OnUpdate(EngineInfo& info);
		virtual void OnPostUpdate(EngineInfo& info);
		virtual void OnExit(EngineInfo& info);
	};
}