#pragma once

namespace je
{
	struct EngineInfo;

	class Module
	{
		friend class Engine;

	protected:
		virtual void OnBegin(EngineInfo& info);
		virtual void OnUpdate(EngineInfo& info);
		virtual void OnPostUpdate(EngineInfo& info);
		virtual void OnExit(EngineInfo& info);
	};
}