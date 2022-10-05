#pragma once

namespace je
{
	struct EngineInfo;
	struct EngineInitializer;

	class Module
	{
		friend class Engine;

	protected:
		virtual void DefineDependencies(const EngineInitializer& initializer);
		virtual void OnInitialize(EngineInfo& info);
		virtual void OnBegin(EngineInfo& info);
		virtual void OnUpdate(EngineInfo& info);
		virtual void OnPostUpdate(EngineInfo& info);
		virtual void OnExit(EngineInfo& info);
	};
}