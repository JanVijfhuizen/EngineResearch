#include "pch.h"
#include "Resource.h"

namespace je::engine
{
	Resource::Resource(Info& info)
	{
	}

	void Resource::Load(Info& info)
	{
		assert(!_loaded);
		_loaded = true;

		OnLoad(info);
	}

	void Resource::Unload(Info& info)
	{
		assert(_loaded);
		_loaded = false;

		OnUnload(info);
	}

	const char* Resource::GetPath() const
	{
		return _path;
	}
}
