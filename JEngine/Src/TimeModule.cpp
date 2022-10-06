#include "pch.h"
#include "TimeModule.h"

#include "Jlb/JMath.h"

namespace je::engine
{
	float TimeModule::GetTime() const
	{
		return _time;
	}

	float TimeModule::GetDeltaTime() const
	{
		return _deltaTime;
	}

	void TimeModule::OnBegin(Info& info)
	{
		Module::OnBegin(info);

		_point = std::chrono::high_resolution_clock::now();
	}

	void TimeModule::OnUpdate(Info& info)
	{
		Module::OnUpdate(info);

		const auto now = std::chrono::high_resolution_clock::now();
		_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - _point).count() * 0.001f;
		_deltaTime = math::Min(1.f, _deltaTime);
		_time += _deltaTime;
	}
}
