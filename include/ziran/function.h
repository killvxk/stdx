#pragma once
#include <functional>

namespace ziran
{
	template<typename R = void>
	class runable
	{
	public:
		virtual ~runable() = default;
		virtual R run() = 0;
	};

	template<typename R,typename _Fn>
	class action:public runable<R>
	{
	public:
		action()
			:runable()
		{}
		action(_Fn &&fn)
			:runable()
			, m_func(fn)
		{}
		~action()=default;

		action(action<R,_Fn> &&other)
			:m_func(std::move(other.m_func))
		{
		}

		action(const action<R,_Fn> &other)
			:m_func(other.m_func)
		{}

		action<R,_Fn> &operator=(const action<R,_Fn> &other)
		{
			m_func = other.m_func;
			return *this;
		}
		// Í¨¹ý runable ¼Ì³Ð
		virtual R run() override
		{
			return m_func();
		}

		operator bool()
		{
			return m_func;
		}
	private:
		_Fn m_func;
	};

	template<typename R = void,typename _Fn>
	std::shared_ptr<action<R,_Fn>> make_action(_Fn &func)
	{
		return std::make_shared<action<R,_Fn>>(std::move(func));
	}

	template<typename R = void, typename _Fn,typename ..._Args>
	std::shared_ptr<action<R, _Fn>> make_action(_Fn &func,_Args &...args)
	{
		return std::make_shared<action<R, _Fn>>(std::move(std::bind(func,args...)));
	}
}