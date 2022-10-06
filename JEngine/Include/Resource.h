#pragma once

namespace je
{
	struct EngineInfo;

	class Resource
	{
		friend class ResourceModule;

	public:
		[[nodiscard]] const char* GetPath() const;

	protected:
		explicit Resource(EngineInfo& info);

	private:
		const char* _path = nullptr;
	};
}
