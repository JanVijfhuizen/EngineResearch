#include "pch.h"
#include "ResourceModule.h"
#include "EngineInfo.h"
#include "ModuleFinder.h"

namespace je
{
	engine::ResourceModule::Initializer::Initializer(LinkedList<Resource>& linkedResources) : _linkedResources(linkedResources)
	{

	}

	void engine::ResourceModule::OnBegin(EngineInfo& info)
	{
		Module::OnBegin(info);

		//_linkedResources = info.persistentArena.New<LinkedList<Resource>>(1, info.persistentArena);

		const auto& finder = info.finder;
		for (const auto& [value, key] : finder)
			if (auto subscriber = dynamic_cast<User*>(value))
			{
				
			}
	}
}
