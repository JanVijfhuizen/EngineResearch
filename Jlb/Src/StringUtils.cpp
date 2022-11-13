#include "jpch.h"
#include "Jlb/StringUtils.h"

namespace je
{
	[[nodiscard]] size_t GetStringLength(const char* string)
	{
		size_t counter = 0;
		const char* c = string;
		for (; *c != 0; ++c)
			++counter;
		return counter;
	}
}
