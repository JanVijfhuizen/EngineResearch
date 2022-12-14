#pragma once

namespace je
{
	template<class T>
	struct RemoveReference
	{
		typedef T Type;
	};

	template<class T>
	struct RemoveReference<T&>
	{
		typedef T Type;
	};

	template<class T>
	struct RemoveReference<T&&>
	{
		typedef T Type;
	};

	// Custom move method akin to std::move.
	template <typename T>
	typename RemoveReference<T>::Type&& Move(T&& arg)
	{
		return static_cast<typename RemoveReference<T>::Type&&>(arg);
	}
}
