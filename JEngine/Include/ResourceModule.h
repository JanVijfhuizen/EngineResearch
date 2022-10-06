#pragma once
#include "Module.h"
#include "Resource.h"
#include "Jlb/LinkedList.h"

namespace je::engine
{
	class ResourceModule final : public Module
	{
	public:
		struct Initializer final
		{
			friend ResourceModule;

		private:
			LinkedList<Resource>& _linkedResources;

			explicit Initializer(LinkedList<Resource>& linkedResources);
		};

		class User
		{
			friend ResourceModule;

		protected:
			virtual void DefineResourceUsage(const Initializer& initializer) = 0;
		};

	private:
		LinkedList<Resource>* _linkedResources = nullptr;

		void OnBegin(EngineInfo& info) override;
	};
}
