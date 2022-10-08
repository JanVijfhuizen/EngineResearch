#include "pch.h"
#include "Modules/ResourceModule.h"
#include "EngineInfo.h"
#include "ModuleFinder.h"
#include "Modules/TimeModule.h"

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

		const size_t count = _linkedResources->GetCount();
		_mapResources = info.persistentArena.New<Map<Resource*>>(1, info.persistentArena, count);
		for (auto& resource : *_linkedResources)
			_mapResources->Insert(resource, reinterpret_cast<size_t>(resource->GetPath()));
	}

	void engine::ResourceModule::OnPostUpdate(Info& info)
	{
		Module::OnPostUpdate(info);

		const auto timeModule = info.finder.Get<TimeModule>();

		for (const auto& resource : *_linkedResources)
		{
			if (resource->alwaysLoaded)
			{
				if (!resource->_loaded)
					resource->Load(info);
				continue;
			}

			if (!resource->_loaded)
				continue;
				
			const bool used = resource->_usages > 0;

			resource->_usages = 0;
			resource->_inactiveDuration = used ? 0 : resource->_inactiveDuration + timeModule->GetDeltaTime();

			if(!used && resource->_inactiveDuration > resource->streamDelay)
				resource->Unload(info);
		}
	}
}
