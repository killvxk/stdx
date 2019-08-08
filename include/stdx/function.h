#pragma once
#include <functional>
#include <stdx/traits/type_list.h>

namespace stdx
{

	template<typename _Fn>
	struct function_info;

	template<typename _Result, typename ..._Args>
	struct function_info<_Result(*)(_Args...)>
	{
		using result = _Result;
		using arguments = stdx::type_list<_Args...>;
	};

	template<typename _Class, typename _Result, typename ..._Args>
	struct function_info<_Result(_Class::*)(_Args...)>
	{
		using belong_type = _Class;
		using result = _Result;
		using arguments = stdx::type_list<_Args...>;
	};

	template<typename _Class, typename _Result, typename ..._Args>
	struct function_info<_Result(_Class::*)(_Args...) const>
	{
		using belong_type = _Class;
		using result = _Result;
		using arguments = stdx::type_list<_Args...>;
	};

	template<typename _Fn>
	struct function_info
	{
	private:
		using info = function_info<decltype(&_Fn::operator())>;
	public:
		using result = typename info::result;
		using arguments = typename info::arguments;
		using belong_type = _Fn;
	};

	template<typename _Fn>
	struct function_info<_Fn&>
	{
	private:
		using info = function_info<_Fn>;
	public:
		using result = typename info::result;
		using arguments = typename info::arguments;
		using belong_type = _Fn;
	};

	template<typename _Fn>
	struct function_info<_Fn&&>
	{
	private:
		using info = function_info<_Fn>;
	public:
		using result = typename info::result;
		using arguments = typename info::arguments;
		using belong_type = _Fn;
	};

	template<typename _Fn>
	struct function_info<const _Fn&>
	{
	private:
		using info = function_info<_Fn>;
	public:
		using result = typename info::result;
		using arguments = typename info::arguments;
		using belong_type = _Fn;
	};

	template<typename _Fn, typename ..._Args>
	struct _IsArgs
	{
	private:
		using info = stdx::function_info<_Fn>;
		using arguments = typename info::arguments;
	public:
		enum
		{
			value = is_same(arguments, stdx::type_list<_Args...>)
		};
	};
#define is_arguments_type(_Fn,_Args) stdx::_IsArgs<_Fn,_Args>::value

	template<typename _Fn, typename _Result>
	struct _IsResult
	{
	private:
		using info = stdx::function_info<_Fn>;
		using result = typename info::result;
	public:
		enum
		{
			value = is_same(result, _Result)
		};
	};
#define is_result_type(_Fn,_Result) stdx::_IsResult<_Fn, _Result>::value
	template<typename _Fn,typename ..._Args>
	inline typename stdx::function_info<_Fn>::result invoke( _Fn &&callable,_Args &&...args)
	{
		return callable(args...);
	}
	template<typename _R = void>
	class _BasicRunable
	{
	public:
		virtual ~_BasicRunable() = default;
		virtual _R run() = 0;
	};

	template<typename _t, typename _fn>
	struct _ActionRunner
	{
		static _t run(_fn &&fn)
		{
			return fn();
		}

		static _t run(_fn &fn)
		{
			return fn();
		}
	};

	template<typename _fn>
	struct _ActionRunner<void, _fn>
	{
		static void run(_fn &&fn)
		{
			fn();
			return;
		}

		static void run(_fn &fn)
		{
			fn();
			return;
		}
	};

	template<typename _R,typename _Fn>
	class _Runable:public _BasicRunable<_R>
	{
	public:
		_Runable()
			:_BasicRunable<_R>()
		{}
		_Runable(const _Fn &fn)
			:_BasicRunable<_R>()
			, m_func(fn)
		{}
		~_Runable()=default;

		virtual _R run() override
		{
			return _ActionRunner<_R,_Fn>::run(std::move(m_func));
		}

		operator bool()
		{
			return m_func;
		}
	private:
		_Fn m_func;
	};

	template<typename T>
	using runable_ptr = std::shared_ptr<_BasicRunable<T>>;
	
	template<typename T, typename _Fn>
	inline runable_ptr<T> make_runable(_Fn &&fn)
	{
		return std::make_shared<_Runable<T,_Fn>>(std::move(fn));
	}

	template<typename T,typename _Fn,typename ..._Args>
	inline runable_ptr<T> make_runable(_Fn &&fn,_Args &&...args)
	{
		return make_runable<T>(std::bind(fn, args...));
	};

	template<typename T, typename _Fn, typename ..._Args>
	inline runable_ptr<T> make_runable(_Fn &fn, _Args &...args)
	{
		return make_runable<T>(std::bind(fn, args...));
	}

	template<typename _R,typename ..._Args>
	class _BasicFunction
	{
	public:
		virtual ~_BasicFunction()=default;
		virtual _R run(const _Args&...) = 0;
	};
	template<typename _R,typename _Fn,typename ..._Args>
	class _Function:public _BasicFunction<_R,_Args...>
	{
	public:
		_Function(_Fn &&fn)
			:_BasicFunction<_R,_Args...>()
			,m_fn(fn)
		{}
		~_Function() = default;
		_R run(const _Args &...args) override
		{
			return stdx::invoke(m_fn,args...);
		}
	private:
		_Fn m_fn;
	};
	template<typename _R,typename ..._Args>
	class function
	{
		using impl_t = std::shared_ptr<_BasicFunction<_R,_Args...>>;
	public:
		function() = default;
		template<typename _Fn>
		function(_Fn &&fn)
			:m_impl(new _Function<_R,_Fn,_Args...>(std::move(fn)))
		{}
		function(const function<_R,_Args...> &other)
			:m_impl(other.m_impl)
		{}
		function(function<_R,_Args...> &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~function() = default;
		function<_R, _Args...> &operator=(const function<_R, _Args...> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		_R operator()(const _Args &...args)
		{
			return m_impl->run(args...);
		}
		operator bool()
		{
			return (bool)m_impl;
		}
	private:
		impl_t m_impl;
	};
}
