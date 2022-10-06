#pragma once

namespace je::engine
{
	struct Info;

	class Resource
	{
		friend class ResourceModule;

	public:
		bool alwaysLoaded = false;
		float streamDelay = 30;

		[[nodiscard]] const char* GetPath() const;

	protected:
		explicit Resource(Info& info);

		virtual void OnLoad(Info& info) = 0;
		virtual void OnUnload(Info& info) = 0;

	private:
		const char* _path = nullptr;
		bool _loaded = false;
		size_t _usages = 0;
		float _inactiveDuration = 0;

		void Load(Info& info);
		void Unload(Info& info);
	};
}
