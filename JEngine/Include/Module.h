#pragma once

namespace je
{
	class Module
	{
		friend class Engine;

	protected:
		virtual void OnBegin();
		virtual void OnUpdate();
		virtual void OnPostUpdate();
		virtual void OnExit();
	};
}