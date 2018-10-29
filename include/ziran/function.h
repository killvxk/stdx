#pragma once
#include <functional>

namespace ziran
{
	template<typename TResult,typename ...TArgs>
	struct function
	{
		using function_type = std::function<TResult(TArgs...)>;
	};
	template<typename TResult>
	struct function<TResult,void>
	{
		using function_type = std::function<TResult()>;
	};
}