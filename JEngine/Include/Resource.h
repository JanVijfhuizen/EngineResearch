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

	private:
		const char* _path = nullptr;
		size_t _usages = 0;
		float _inactiveDuration = 0;
	};
}
