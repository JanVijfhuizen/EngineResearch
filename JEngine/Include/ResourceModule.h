#pragma once
#include "EngineInfo.h"
#include "Module.h"
#include "Resource.h"
#include "Jlb/JMap.h"
#include "Jlb/LinkedList.h"

namespace je::engine
{
	class ResourceModule final : public Module
	{
	public:
		struct Initializer final
		{
			friend ResourceModule;

			template <typename T>
			void Add(const char* path);

		private:
			Info& _info;
			LinkedList<Resource*>& _linkedResources;

			explicit Initializer(Info& info, LinkedList<Resource*>& linkedResources);
		};

		class User
		{
			friend ResourceModule;

		protected:
			virtual void DefineResourceUsage(const Initializer& initializer) = 0;
		};

		template <typename T>
		[[nodiscard]] T* GetResource(Info& info, const char* path);

	private:
		LinkedList<Resource*>* _linkedResources = nullptr;
		Map<Resource*>* _mapResources = nullptr;

		void OnBegin(Info& info) override;
		void OnPostUpdate(Info& info) override;
	};

	template <typename T>
	void ResourceModule::Initializer::Add(const char* path)
	{
		for (const auto& resource : _linkedResources)
			if (resource->GetPath() == path)
				return;

		const T* ptr = _info.persistentArena.New<T>();
		_linkedResources.Add(ptr);
	}

	template <typename T>
	T* ResourceModule::GetResource(Info& info, const char* path)
	{
		Resource* resource = *_mapResources->Contains(reinterpret_cast<size_t>(path));
		++resource->_usages;
		if (!resource->_loaded)
			resource->Load(info);
		return static_cast<T*>(resource);
	}
}
