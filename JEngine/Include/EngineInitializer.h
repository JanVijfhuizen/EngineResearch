#pragma once
#include "Engine.h"

namespace je
{
	struct EngineInitializer final
	{
		friend Engine;

		template <typename T, typename ...Args>
		void AddModule(Args... args) const;

	private:
		Engine& _engine;

		explicit EngineInitializer(Engine& engine);
	};

	template <typename T, typename ...Args>
	void EngineInitializer::AddModule(Args... args) const
	{
		_engine.AddModule<T>(args...);
	}
}