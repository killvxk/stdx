#pragma once

namespace stdx
{
	//引用类型
	template<typename _t>
	struct _Ref
	{
		using type = _t & ;
	};

	template<typename _t>
	struct _Ref<_t&>
	{
		using type = _t & ;
	};

	template<typename _t>
	struct _Ref<const _t&>
	{
		using type = const _t &;
	};

	template<>
	struct _Ref<void>
	{
		using type = void;
	};

	template<typename T>
	using ref_t = typename _Ref<T>::type;
}