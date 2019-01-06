#pragma once
#include <functional>

namespace stdx
{
	template<typename R = void>
	class _BasicAction
	{
	public:
		virtual ~_BasicAction() = default;
		virtual R run() = 0;
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

	template<typename R,typename _Fn>
	class _Action:public _BasicAction<R>
	{
	public:
		_Action()
			:_BasicAction()
		{}
		_Action(_Fn &&fn)
			:_BasicAction()
			, m_func(fn)
		{}
		~_Action()=default;

		_Action(_Action<R,_Fn> &&other)
			:m_func(std::move(other.m_func))
		{
		}

		_Action(const _Action<R,_Fn> &other)
			:m_func(other.m_func)
		{}

		_Action<R,_Fn> &operator=(const _Action<R,_Fn> &other)
		{
			m_func = other.m_func;
			return *this;
		}

		// Í¨¹ý _BasicAction ¼Ì³Ð
		virtual R run() override
		{
			return _ActionRunner<R,_Fn>::run(m_func);
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
			:m_impl(std::make_shared<_Action<_R,_Fn>>(fn))
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

	template<typename R = void,typename _Fn>
	action<R> make_action(_Fn &&func)
	{
		return action<R>(func);
	}

	template<typename R = void, typename _Fn,typename ..._Args>
	action<R> make_action(_Fn &&func,_Args &...args)
	{
		return action<R>(std::bind(func,args...));
	}

	template<typename R = void, typename _Fn>
	std::shared_ptr<_Action<R, _Fn>> _MakeAction(_Fn &func)
	{
		return std::make_shared<_Action<R, _Fn>>(std::move(func));
	}

	//template<typename _R,typename ..._Args>
	//class function
	//{
	//public:
	//	function();
	//	~function();

	//private:

	//};
}