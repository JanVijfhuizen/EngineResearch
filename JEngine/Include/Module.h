#pragma once

namespace je
{
	struct EngineInfo;

	class Module
	{
		friend class Engine;

	protected:
		virtual void OnBegin(const EngineInfo& info);
		virtual void OnUpdate(const EngineInfo& info);
		virtual void OnPostUpdate(const EngineInfo& info);
		virtual void OnExit(const EngineInfo& info);
	};
}