#pragma once

namespace je
{
	template <typename T>
	struct View final
	{
		T* data = nullptr;
		size_t length = 0;
	};
}