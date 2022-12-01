#pragma once
#include "JEngine/Module.h"
#include <chrono>

namespace je::engine
{
	class TimeModule final : public Module
	{
	public:
		[[nodiscard]] float GetTime() const;
		[[nodiscard]] float GetDeltaTime() const;

	private:
		using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

		float _time = 0;
		float _deltaTime = 1;

		void OnBegin(Info& info) override;
		void OnUpdate(Info& info) override;

		TimePoint _point;
	};
}
