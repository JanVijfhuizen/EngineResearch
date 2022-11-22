#pragma once

namespace je
{
	template <size_t I, typename T>
	struct TupleLeaf
	{
		T value{};

		TupleLeaf() = default;
		explicit TupleLeaf(const T& value);
	};

	template <size_t I, typename ...Args>
	struct TupleImpl;

	template <size_t I>
	struct TupleImpl<I>
	{
		
	};

	template <size_t I, typename Head, typename ...Tail>
	struct TupleImpl<I, Head, Tail...> : TupleLeaf<I, Head>, TupleImpl<I + 1, Tail...>
	{
		TupleImpl() = default;
		explicit TupleImpl(const Head& head, const Tail&... tail);
	};

	template <size_t I, typename T>
	TupleLeaf<I, T>::TupleLeaf(const T& value) : value(value)
	{

	}

	template <size_t I, typename Head, typename ... Tail>
	TupleImpl<I, Head, Tail...>::TupleImpl(const Head& head, const Tail&... tail) : TupleLeaf<I, Head>(head), TupleImpl<I + 1, Tail...>(tail...)
	{

	}

	template<typename ...Args>
	using Tuple = TupleImpl<0, Args...>;

	template <size_t I, typename Head, typename ...Tail>
	[[nodiscard]] auto& Get(TupleImpl<I, Head, Tail...>& tuple)
	{
		return tuple.template TupleLeaf<I, Head>::value;
	}

	template <typename Head, typename ...Tail>
	[[nodiscard]] constexpr size_t Size(const Tuple<Head, Tail...>& tuple)
	{
		return sizeof...(Tail) + 1;
	}
}