#pragma once

namespace stdx
{
	template<typename _T,typename _U>
	struct _IsSame
	{
		enum
		{
			value = false
		};
	};
	template<typename _T>
	struct _IsSame<_T,_T>
	{
		enum
		{
			value = true
		};
	};
#define is_same(t,u) stdx::_IsSame<t,u>::value

}