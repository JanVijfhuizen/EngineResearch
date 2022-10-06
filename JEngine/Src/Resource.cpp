#include "pch.h"
#include "Resource.h"

namespace je::engine
{
	Resource::Resource(Info& info)
	{
	}

	const char* Resource::GetPath() const
	{
		return _path;
	}
}
