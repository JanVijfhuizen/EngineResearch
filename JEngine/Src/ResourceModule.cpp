#include "pch.h"
#include "ResourceModule.h"
#include "EngineInfo.h"
#include "ModuleFinder.h"

namespace je
{
	engine::ResourceModule::Initializer::Initializer(Info& info, LinkedList<Resource*>& linkedResources) :
		_info(info), _linkedResources(linkedResources)
	{

	}

	void engine::ResourceModule::OnBegin(Info& info)
	{
		Module::OnBegin(info);

		_linkedResources = info.persistentArena.New<LinkedList<Resource*>>(1, info.persistentArena);

		const Initializer initializer{info, *_linkedResources };

		const auto& finder = info.finder;
		for (const auto& [value, key] : finder)
			if (const auto subscriber = dynamic_cast<User*>(value))
				subscriber->DefineResourceUsage(initializer);

		_mapResources = info.persistentArena.New<Map<Resource*>>(1, info.persistentArena, _linkedResources->GetCount());
		for (auto& resource : *_linkedResources)
			_mapResources->Insert(resource, reinterpret_cast<size_t>(resource->GetPath()));
	}
}
