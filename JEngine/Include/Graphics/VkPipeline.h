#pragma once

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		class Pipeline
		{
		public:
			Pipeline(Arena& tempArena, const App& app);
			Pipeline(Pipeline&& other) noexcept;
			Pipeline& operator=(Pipeline&& other) noexcept;
			~Pipeline();
		};
	}
}
