#pragma once
#include <functional>

namespace stdx
{
	template<typename _R = void>
	class _BasicAction
	{
	public:
		virtual ~_BasicAction() = default;
		virtual _R run() = 0;
	};

	template<typename _t, typename _fn>
	struct _ActionRunner
	{
		static _t run(_fn &fn)
		{
			return fn();
		}
	};

	template<typename _fn>
	struct _ActionRunner<void, _fn>
	{
		static void run(_fn &fn)
		{
			fn();
			return;
		}
	};

	template<typename _R,typename _Fn>
	class _Action:public _BasicAction<_R>
	{
	public:
		_Action()
			:_BasicAction<_R>()
		{}
		_Action(_Fn &&fn)
			:_BasicAction()
			, m_func(fn)
		{}
		~_Action()=default;

		_Action(_Action<_R,_Fn> &&other)
			:m_func(std::move(other.m_func))
		{
		}

		_Action(const _Action<_R,_Fn> &other)
			:m_func(other.m_func)
		{}

		_Action<_R,_Fn> &operator=(const _Action<_R,_Fn> &other)
		{
			m_func = other.m_func;
			return *this;
		}

		// Í¨¹ý _BasicAction ¼Ì³Ð
		virtual _R run() override
		{
			return _ActionRunner<_R,_Fn>::run(m_func);
		}

		operator bool()
		{
			return m_func;
		}
	private:
		_Fn m_func;
	};

	template<typename _R>
	class action
	{
		using impl_t = std::shared_ptr<_BasicAction<_R>>;
	public:
		template<typename _Fn>
		action(_Fn &&fn)
			:m_impl(new _Action<_R,_Fn>(std::move(fn)))
		{}
		action(const action<_R> &other)
			:m_impl(other.m_impl)
		{}
		action(action<_R> &&other)
			:m_impl(other.m_impl)
		{}
		~action() = default;
		action<_R> &operator=(const action<_R> &other)
		{
			printf("1");
			m_impl = other.m_impl;
			return *this;
		}
		_R operator()()
		{
			return m_impl->run();
		}
		operator bool()
		{
			return (bool)m_impl;
		}
	private:
		impl_t m_impl;
	};

	template<typename _R = void,typename _Fn>
	action<_R> make_action(_Fn &&func)
	{
		return action<_R>(func);
	}

	template<typename _R = void, typename _Fn,typename ..._Args>
	action<_R> make_action(_Fn &&func,_Args &...args)
	{
		return action<_R>(std::bind(func,args...));
	}

	template<typename _R = void, typename _Fn>
	std::shared_ptr<_Action<_R, _Fn>> _MakeAction(_Fn &func)
	{
		return std::make_shared<_Action<_R, _Fn>>(std::move(func));
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
			return std::invoke(m_fn,args...);
		}
	private:
		_Fn m_fn;
	};
	template<typename _R,typename ..._Args>
	class function
	{
		using impl_t = std::shared_ptr<_BasicFunction<_R,_Args...>>;
	public:
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
	private:
		impl_t m_impl;
	};
}