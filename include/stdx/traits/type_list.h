#pragma once
#include <stdx/traits/same_type.h>

namespace stdx
{
	template<typename ..._T>
	struct type_list;

	template<int index,typename _TL>
	struct _TypeAt
	{
		using type = typename _TypeAt<index - 2,typename _TL::More>::type;
	};
	template<typename _TL>
	struct _TypeAt<0,_TL>
	{
		using type = typename _TL::First;
	};
	template<typename _TL>
	struct _TypeAt<1,_TL>
	{
		using type = typename _TL::Secound;
	};
	template<int index,typename tl>
	using type_at = typename _TypeAt<index, tl>::type;

	template<typename _First>
	struct type_list<_First>
	{
		using First = _First;
		enum
		{
			size = 1
		};
		template<typename _T>
		struct include
		{
			enum
			{
				value = is_same(_First, _T);
			};
		};
	};

	template<typename _First,typename _Secound>
	struct type_list<_First,_Secound>
	{
		using First = _First;
		using Secound = _Secound;

		enum
		{
			size = 2
		};

		template<typename _T>
		struct include
		{
			enum
			{
				value = is_same(_First, _T) || is_same(_Secound,_T);
			};
		};
	};

	template<typename _First,typename _Secound,typename ..._More>
	struct type_list<_First,_Secound,_More...>
	{
		using First = _First;
		using Secound = _Secound;
		using More = type_list<_More...>;

		enum
		{
			size = 2+More::size
		};

		template<typename _T>
		struct include
		{
			enum
			{
				value = is_same(First, _T) || is_same(Secound, _T) || type_list<_More...>::include<_T>::value;
			};
		};
	};
}