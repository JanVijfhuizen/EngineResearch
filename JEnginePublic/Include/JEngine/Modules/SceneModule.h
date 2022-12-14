#pragma once
#include "JEngine/Module.h"
#include "JEngine/Cecsar.h"
#include "Jlb/Finder.h"

namespace je
{
	typedef bool(*SceneFunc)(const Finder<Module>& finder, ecs::Cecsar& cecsar, void* userPtr);

	struct SceneInfo final
	{
		SceneFunc onBegin = nullptr;
		SceneFunc onUpdate = nullptr;
		SceneFunc onExit = nullptr;
		// Initial amount of memory used. Dynamically increases based on usage.
		size_t initialCapacity = 4096;
		void* userPtr = nullptr;
	};

	// Manages the different levels/scenes of the game.
	class SceneModule final : public Module
	{
	public:
		explicit SceneModule(const Array<SceneInfo>& sceneInfos);

		void Load(size_t index) const;
		void Unload(size_t index) const;
		[[nodiscard]] bool IsLoaded(size_t index) const;

	private:
		struct Scene final
		{
			SceneInfo info;
			void* memory = nullptr;
			Arena* arena = nullptr;
			ecs::Cecsar* cecsar = nullptr;
			bool loaded = false;
		};

		// Frame arena, only valid .
		Array<SceneInfo> _sceneInfos;

		const Finder<Module>* _finder = nullptr;
		Array<Scene> _scenes{};

		void OnBegin(engine::Info& info) override;
		void OnUpdate(engine::Info& info) override;
		void OnExit(engine::Info& info) override;
	};
}
