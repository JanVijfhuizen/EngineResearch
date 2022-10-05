#pragma once

namespace je
{
	class Arena;

	template <typename T>
	struct CopyStructure final
	{
		Arena& arena;
		T& structure;

		CopyStructure(Arena& arena, T& structure);
	};

	template <typename T>
	CopyStructure<T>::CopyStructure(Arena& arena, T& structure) : arena(arena), structure(structure)
	{

	}
}
