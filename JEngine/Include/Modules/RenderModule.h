#pragma once
#include "Module.h"

namespace je::engine
{
	class RenderModule final : public Module
	{
	public:
		void OnInitialize(Info& info) override;
	};
}
