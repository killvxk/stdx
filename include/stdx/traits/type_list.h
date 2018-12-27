#pragma once
#include <stdx/traits/same_type.h>

namespace stdx
{
	template<typename ..._T>
	struct type_list;
	template<typename _First>
	struct type_list<_First>
	{
		using First = _First;
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
		using More = type_list<type_list<_More...>>;
		template<typename _T>
		struct include
		{
			enum
			{
				value = is_same(First, _T) || is_same(Secound, _T) || type_list<type_list<_More...>>::include<_T>::value;
			};
		};
	};
}