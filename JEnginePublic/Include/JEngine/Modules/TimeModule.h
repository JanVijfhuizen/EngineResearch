#pragma once
#include "JEngine/Module.h"
#include <chrono>

namespace je::engine
{
	// Manages the time/deltatime tracking for this application.
	class TimeModule final : public Module
	{
	public:
		// Returns the total runtime of the application.
		[[nodiscard]] float GetTime() const;
		// Returns the duration of the previous frame.
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
