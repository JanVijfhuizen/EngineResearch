#include "pch.h"
#include "Modules/SceneModule.h"
#include "EngineInfo.h"

namespace je
{
	SceneModule::SceneModule(const Array<SceneInfo>& sceneInfos) : _sceneInfos(sceneInfos)
	{

	}

	void SceneModule::OnBegin(engine::Info& info)
	{
		Module::OnBegin(info);
		_finder = &info.finder;
		
		_scenes = CreateArray<Scene>(info.persistentArena, _sceneInfos.length);

		for (size_t i = 0; i < _scenes.length; ++i)
		{
			auto& scene = _scenes[i];
			const auto& sceneInfo = _sceneInfos[i];
			scene.info = sceneInfo;
		}

		Load(0);
	}

	void SceneModule::OnUpdate(engine::Info& info)
	{
		Module::OnUpdate(info);

		for (const auto& scene : _scenes)
		{
			if (!scene.loaded)
				continue;
			if (!scene.info.onUpdate)
				continue;
			if (!scene.info.onUpdate(*_finder, *scene.cecsar, scene.info.userPtr))
				info.quit = true;
		}
	}

	void SceneModule::OnExit(engine::Info& info)
	{
		for (size_t i = 0; i < _scenes.length; ++i)
			Unload(i);
		DestroyArray(_scenes, info.persistentArena);
		Module::OnExit(info);
	}

	void SceneModule::Load(const size_t index) const
	{
		if (IsLoaded(index))
			return;

		auto& scene = _scenes[index];
		scene.loaded = true;
		scene.memory = malloc(scene.info.initialCapacity + sizeof(Arena));
		scene.arena = static_cast<Arena*>(scene.memory);
		new(scene.arena) Arena(static_cast<unsigned char*>(scene.memory) + sizeof(Arena), scene.info.initialCapacity);
		scene.cecsar = scene.arena->New<ecs::Cecsar>(1, *scene.arena);

		if (scene.info.onBegin)
			scene.info.onBegin(*_finder, *scene.cecsar, scene.info.userPtr);
	}

	void SceneModule::Unload(const size_t index) const
	{
		if (!IsLoaded(index))
			return;

		auto& scene = _scenes[index];
		scene.loaded = false;
		if (scene.info.onExit)
			scene.info.onExit(*_finder, *scene.cecsar, scene.info.userPtr);
		free(scene.memory);
	}

	bool SceneModule::IsLoaded(const size_t index) const
	{
		return _scenes[index].loaded;
	}
}
