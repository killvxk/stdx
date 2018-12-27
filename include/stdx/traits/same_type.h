#pragma once

namespace stdx
{
	template<typename _T,typename _U>
	struct same_type
	{
		enum
		{
			value = false;
		};
	};
	template<typename _T>
	struct same_type<_T,_T>
	{
		enum
		{
			value = true;
		};
	};
#define is_same(t,u) stdx::same_type<t,u>::value

}