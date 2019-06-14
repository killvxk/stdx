#pragma once
#include <stdx/traits/is_same.h>

namespace stdx
{
	template<typename ..._T>
	struct type_list;

	template <typename ..._Types>
	struct _TypeList;
	template <typename _First, typename _Secound, typename ..._More>
	struct _TypeList<_First,_Secound,type_list<_More...>>
	{
		using type = type_list<_First, _Secound, _More...>;
	};
	template<typename _First>
	struct _TypeList<_First>
	{
		using type = type_list<_First>;
	};
	template<typename _First,typename _Secound>
	struct _TypeList<_First,_Secound>
	{
		using type = type_list<_First, _Secound>;
	};
	template<>
	struct _TypeList<>
	{
		using type = type_list<>;
	};

	template<>
	struct type_list<>
	{
		using First = void;
	};

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

	template<typename _Type,typename _TL>
	struct _TypeAppend;

	template<typename _Type,typename ..._Types>
	struct _TypeAppend<_Type,type_list<_Types...>>
	{
		using type = type_list<_Type,_Types...>;
	};

	template<typename _Type, typename _TL>
	using type_append = typename _TypeAppend<_Type,_TL>::type;

	template<typename _Type,typename _TL>
	struct _TypePush;

	template<typename _Type,typename ..._Types>
	struct _TypePush<_Type,type_list<_Types...>>
	{
		using type = type_list<_Types..., _Type>;
	};

	template<typename _Type, typename _TL>
	using type_push = typename _TypePush<_Type,_TL>::type;

	template<typename _Type, typename _TL>
	struct _TypeErase;

	template<typename _Type, typename _First>
	struct _TypeErase<_Type,type_list<_First>>
	{
		using type = type_list<_First>;
	};
	template<typename _Type>
	struct _TypeErase<_Type, type_list<_Type>>
	{
		using type = type_list<>;
	};

	template<typename _Type,typename _First,typename _Secound>
	struct _TypeErase<_Type,type_list<_First,_Secound>>
	{
		using type = type_list<_First, _Secound>;
	};

	template<typename _Type,typename _Secound>
	struct _TypeErase<_Type, type_list<_Type, _Secound>>
	{
		using type = type_list<_Secound>;
	};

	template<typename _Type, typename _First>
	struct _TypeErase<_Type, type_list<_First, _Type>>
	{
		using type = type_list<_First>;
	};

	template<typename _Type,typename _First,typename _Secound,typename ..._Types>
	struct _TypeErase<_Type,type_list<_First,_Secound,_Types...>>
	{
		using type = typename _TypeList<_First,_Secound, _TypeErase<_Type,type_list<_Types...>>>::type;
	};

	template<typename _Type, typename _Secound, typename ..._Types>
	struct _TypeErase<_Type, type_list<_Type, _Secound, _Types...>>
	{
		using type = type_list<_Secound,_Types...>;
	};

	template<typename _Type, typename _First, typename ..._Types>
	struct _TypeErase<_Type, type_list<_First, _Type, _Types...>>
	{
		using type = type_list<_First,_Types...>;
	};
	template<typename _Type, typename _TL>
	using type_erase = typename _TypeErase<_Type,_TL>::type;

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
				value = is_same(_First, _T) || _IsSame(_Secound,_T);
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
