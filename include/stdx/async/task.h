﻿#pragma once
#include <stdx/async/threadpool.h>
#include <stdx/async/spin_lock.h>
#include <memory>
#include <future>
#include <stdx/traits/ref_type.h>
#include <stdx/traits/value_type.h>
#include <stdx/function.h>
#include <stdx/env.h>

namespace stdx
{
	//Task状态
	struct task_state
	{
		enum
		{
			//就绪
			ready = 0,
			//完成
			complete = 1,
			//运行中
			running = 2,
			//错误
			error = 3
		};
	};

	template<typename _T>
	using promise_ptr = std::shared_ptr<std::promise<_T>>;

	using state_ptr = std::shared_ptr<int>;

	template<typename _T>
	using shared_future_ptr = std::shared_ptr<std::shared_future<_T>>;

	template<typename _T>
	promise_ptr<_T> make_promise_ptr()
	{
		return std::make_shared<std::promise<_T>>();
	}

	//task_result模板
	template<typename _T>
	class task_result
	{
		using result_t = stdx::ref_t<const _T>;
	public:
		task_result() = default;
		task_result(std::shared_future<_T> future)
			:m_future(future)
		{}

		~task_result() = default;
		task_result(const task_result<_T> &other)
			:m_future(other.m_future)
		{}

		task_result(task_result<_T> &&other)
			:m_future(std::move(other.m_future))
		{}

		task_result<_T> &operator=(const task_result<_T> &other)
		{
			m_future = other.m_future;
			return *this;
		}

		result_t get()
		{
			
			return m_future.get();
		}
	private:
		std::shared_future<_T> m_future;
	};

	template<>
	class task_result<void>
	{
	public:
		task_result() = default;
		task_result(std::shared_future<void> future)
			:m_future(future)
		{}

		~task_result() = default;
		task_result(const task_result<void> &other)
			:m_future(other.m_future)
		{}

		task_result(task_result<void> &&other)
			:m_future(std::move(other.m_future))
		{}

		task_result<void> &operator=(const task_result<void> &other)
		{
			m_future = other.m_future;
			return *this;
		}

		void get()
		{
			m_future.get();
		}
	private:
		std::shared_future<void> m_future;
	};

	template<typename R>
	class _Task;

	//Task模板
	template<typename _R>
	class task
	{
		using impl_t = std::shared_ptr<stdx::_Task<_R>>;
	public:
		task() = default;
		template<typename _Fn, typename ..._Args>
		task(_Fn &&fn, _Args &&...args)
			:m_impl(std::make_shared<_Task<_R>>(std::move(fn), args...))
		{}

		explicit task(impl_t impl)
			:m_impl(impl)
		{}
		task(const task<_R> &other)
			:m_impl(other.m_impl)
		{}

		task(task<_R> &&other)
			:m_impl(std::move(other.m_impl))
		{}

		task<_R> &operator=(const task<_R> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		operator impl_t()
		{
			return m_impl;
		}

		~task() = default;

		void run()
		{
			m_impl->run();
		}

		void run_on_this_thread()
		{
			m_impl->run_on_this_thread();
		}

		template<typename _Fn, typename ..._Args>
		static task<_R> start(_Fn &&fn, _Args &&...args)
		{
			auto t = task<_R>(fn, args...);
			t.run();
			return t;
		}

		template<typename _Fn, typename __R = typename stdx::function_info<_Fn>::result>
		task<__R> then(_Fn &&fn)
		{
			return task<__R>(m_impl->then<_Fn>(std::move(fn)));
		}

		void wait()
		{
			return m_impl->wait();
		}

		task_result<_R> get()
		{
			return m_impl->get();
		}

		bool is_complete() const
		{
			return m_impl->is_complete();
		}

		template<typename __R>
		task<void> with(task<__R> other)
		{
			return task<void>(m_impl->with(other.m_impl));
		}

		operator bool() const
		{
			return (bool)m_impl;
		}

		bool operator==(const stdx::task<_R> &other) const
		{
			return m_impl == other.m_impl;
		}
	private:
		impl_t m_impl;
	};
        //BasicTask
    interface_class basic_task:public stdx::basic_runable<void>
	{
	public: 
		virtual ~basic_task() = default;
		virtual void run_on_this_thread()=0;
	};
	template<typename _T>
	using task_ptr = std::shared_ptr<stdx::_Task<_T>>;

	template<typename _T,typename _Fn,typename ..._Args>
	inline task_ptr<_T> make_task_ptr(_Fn &&fn, _Args &&...args)
	{
		return std::make_shared<_Task<_T>>(fn, args...);
	}
	//_TaskCompleter模板
	template<typename _t>
	struct _TaskCompleter
	{
		static void call(stdx::runable_ptr<_t> call, promise_ptr<_t> promise, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next, stdx::spin_lock lock, state_ptr state)
		{
			try
			{
				//调用方法
				//设置promise
				promise->set_value(call->run());
			}
			catch (const std::exception&)
			{
				//加锁
				lock.lock();
				//设置状态为错误
				*state = task_state::error;
				promise->set_exception(std::current_exception());
				//如果有callback
				if (*next)
				{
					//解锁
					lock.unlock();
					//运行callback
					(*next)->run_on_this_thread();
				}
				//解锁
				lock.unlock();
				return;
			}
			//加锁
			lock.lock();
			//如果有callback
			if (*next)
			{
				*state = task_state::complete;
				//解锁
				lock.unlock();
				//运行callback
				(*next)->run_on_this_thread();
				return;
			}
			//设置状态为完成
			*state = task_state::complete;
			//解锁
			lock.unlock();
			return;
		}
	};

	template<>
	struct _TaskCompleter<void>
	{
		static void call(stdx::runable_ptr<void> call, promise_ptr<void> promise, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next, stdx::spin_lock lock, state_ptr state)
		{
			try
			{
				//调用方法
				call->run();
				//设置promise
				promise->set_value();
			}
			catch (const std::exception&)
			{
				//加锁
				lock.lock();
				//设置状态为错误
				*state = task_state::error;
				promise->set_exception(std::current_exception());
				//如果有callback
				if (*next)
				{
					//解锁
					lock.unlock();
					//运行callback
					(*next)->run_on_this_thread();
				}
				//解锁
				lock.unlock();
				return;
			}
			//加锁
			lock.lock();
			//如果有callback
			if (*next)
			{
				*state = task_state::complete;
				//解锁
				lock.unlock();
				//运行callback
				(*next)->run_on_this_thread();
				return;
			}
			//设置状态为完成
			*state = task_state::complete;
			//解锁
			lock.unlock();
			return;
		}
	};

	template<typename Input, typename Result, typename Arg>
	struct _TaskNextBuilder
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<Input> &future, state_ptr state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next)
		{
			using arg_t = typename stdx::function_info<Fn>::arguments;
			static_assert( is_arguments_type(Fn, stdx::task_result<Result> )||is_arguments_type(Fn,Result)||is_arguments_type(Fn,void), "the input function not be allowed");
			return nullptr;
		}
	};

	template<typename Input, typename Result>
	struct _TaskNextBuilder<Input,Result,void>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<Input> &future,state_ptr state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next)
		{
			auto t = stdx::make_task_ptr<Result>([](Fn &&fn, std::shared_future<Input> &future)
			{
				future.wait();
				return fn();
			}, fn, future);
			lock.lock();
			if ((*state == task_state::complete) || (*state == task_state::error))
			{
				//解锁
				lock.unlock();
				//运行
				t->run();
				return t;
			}
			*next = t;
			lock.unlock();
			return t;
		}
	};
	template<typename Input, typename Result>
	struct _TaskNextBuilder<Input, Result, stdx::task_result<Input>>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<Input> &future, state_ptr state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next)
		{
			auto t = stdx::make_task_ptr<Result>([](Fn &&fn, std::shared_future<Input> &future)
			{
				return std::bind(fn, task_result<Input>(future))();
			}, fn, future);
			lock.lock();
			if ((*state == task_state::complete) || (*state == task_state::error))
			{
				//解锁
				lock.unlock();
				//运行
				t->run();
				return t;
			}
			*next = t;
			lock.unlock();
			return t;
		}
	};

	template<typename Result>
	struct _TaskNextBuilder<void, Result, void>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<void> &future, state_ptr state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next)
		{
			auto t = stdx::make_task_ptr<Result>([](Fn &&fn, std::shared_future<void> &future)
			{
				future.wait();
				return fn();
			}, fn, future);
			lock.lock();
			if ((*state == task_state::complete) || (*state == task_state::error))
			{
				//解锁
				lock.unlock();
				//运行
				t->run();
				return t;
			}
			*next = t;
			lock.unlock();
			return t;
		}
	};

	template<typename Input, typename Result>
	struct _TaskNextBuilder<Input, Result,Input>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<Input> &future, state_ptr state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next)
		{
			auto t = stdx::make_task_ptr<Result>([](Fn &&fn, std::shared_future<Input> &future)
			{
				return std::bind(fn,future.get())();
			}, fn, future);
			lock.lock();
			if ((*state == task_state::complete) || (*state == task_state::error))
			{
				//解锁
				lock.unlock();
				//运行
				t->run();
				return t;
			}
			*next = t;
			lock.unlock();
			return t;
		}
	};

	template<typename Input, typename Result>
	struct _TaskNextBuilder<stdx::task<Input>,Result,Input>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<stdx::task<Input>> &future, state_ptr state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next)
		{
			promise_ptr<Input> promise = stdx::make_promise_ptr<Input>();
			auto t = stdx::make_task_ptr<Result>([](Fn &&fn,std::shared_future<Input> result)
			{
				fn(result.get());
			},fn,(std::shared_future<Input>)promise->get_future());
			auto start = stdx::make_task_ptr<void>([](std::shared_ptr<_Task<Result>> t,std::shared_future<stdx::task<Input>> &future, promise_ptr<Input> input_promise)
			{
				auto task = future.get();
				task.then([input_promise,t](Input &r)
				{
					input_promise->set_value(r);
					t->run_on_this_thread();
				});
			},t,future,promise);
			lock.lock();
			if ((*state == task_state::complete) || (*state == task_state::error))
			{
				//解锁
				lock.unlock();
				//运行
				start->run();
				return t;
			}
			*next = start;
			lock.unlock();
			return t;
		}
	};

	template<typename Input, typename Result>
	struct _TaskNextBuilder<stdx::task<Input>, Result, stdx::task_result<Input>>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<stdx::task<Input>> &future, state_ptr state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::basic_task>> next)
		{
			promise_ptr<stdx::task_result<Input>> promise = stdx::make_promise_ptr<stdx::task_result<Input>>();
			auto t = stdx::make_task_ptr<Result>([](Fn &&fn, std::shared_future<stdx::task_result<Input>> result)
			{
				fn(result.get());
			}, fn, (std::shared_future<stdx::task_result<Input>>)promise->get_future());
			auto start = stdx::make_task_ptr<void>([](std::shared_ptr<_Task<Result>> t, std::shared_future<stdx::task<Input>> &future, promise_ptr<stdx::task_result<Input>> input_promise)
			{
				auto task = future.get();
				task.then([input_promise, t](stdx::task_result<Input> &r)
				{
					input_promise->set_value(r);
					t->run_on_this_thread();
				});
			}, t, future, promise);
			lock.lock();
			if ((*state == task_state::complete) || (*state == task_state::error))
			{
				//解锁
				lock.unlock();
				//运行
				start->run();
				return t;
			}
			*next = start;
			lock.unlock();
			return t;
		}
	};



	//Task模板的实现
	template<typename R>
	class _Task :public stdx::basic_task
	{
	public:
		//构造函数
		template<typename _Fn, typename ..._Args>
		explicit _Task(_Fn &&f, _Args &&...args)
			:m_action(stdx::make_runable<R>(std::move(f), args...))
			, m_promise(std::make_shared<std::promise<R>>())
			, m_future(m_promise->get_future())
			, m_next(std::make_shared<std::shared_ptr<stdx::basic_task>>(nullptr))
			, m_state(std::make_shared<int>(stdx::task_state::ready))
			, m_lock()
		{
		}

		template<typename _Fn>
		explicit _Task(_Fn &&f)
			:m_action(stdx::make_runable<R>(std::move(f)))
			, m_promise(std::make_shared<std::promise<R>>())
			, m_future(m_promise->get_future())
			, m_next(std::make_shared<std::shared_ptr<stdx::basic_task>>(nullptr))
			, m_state(std::make_shared<int>(stdx::task_state::ready))
			, m_lock()
		{
		}

		//析构函数
		virtual ~_Task() = default;

		//启动一个Task
		virtual void run() override
		{
			//加锁
			m_lock.lock();
			//如果不在运行
			if (*m_state != stdx::task_state::running)
			{
				//设置状态运行中
				*m_state = stdx::task_state::running;
			}
			else
			{
				//如果正在运行
				//解锁返回
				m_lock.unlock();
				return;
			}
			//解锁
			m_lock.unlock();
			//创建方法
			auto f = [](stdx::runable_ptr<R> r
				, promise_ptr<R> promise
				, std::shared_ptr<std::shared_ptr<stdx::basic_task>>  next
				, stdx::spin_lock lock
				, state_ptr state)
			{
				stdx::_TaskCompleter<R>::call(r, promise, next, lock, state);
			};
			//放入线程池
			stdx::threadpool::run(f, m_action, m_promise, m_next, m_lock, m_state);
		}
		void run_on_this_thread() override 
		{
		    m_lock.lock();
			if (!m_state)
			{
				m_lock.unlock();
				return;
			}
		    if((*m_state) != stdx::task_state::running)
		    {
		        *m_state = stdx::task_state::running;
		    }
		    else
		    {
		        m_lock.unlock();
				return;
		    }
		    m_lock.unlock();
		    stdx::_TaskCompleter<R>::call(m_action,m_promise,m_next,m_lock,m_state);
		}

		//等待当前Task(不包括后续)完成
		void wait()
		{
			m_future.wait();
		}

		//等待当前Task(不包括后续)完成并获得结果
		//发生异常则抛出异常
		task_result<R> get()
		{
			return m_future;
		}

		//询问Task是否完成
		bool is_complete() const
		{
			auto c = (*m_state == task_state::complete) || (*m_state == task_state::error);
			return c;
		}

		//template<typename _Fn, typename ..._Args>
		//static std::shared_ptr<_Task<R>> make(_Fn &fn, _Args &...args)
		//{
		//	return std::make_shared<_Task<R>>(std::move(fn), std::move(args)...);
		//}

		template<typename _Fn, typename ..._Args>
		static std::shared_ptr<_Task<R>> make(_Fn &&fn, _Args &&...args)
		{
			return std::make_shared<_Task<R>>(fn, args...);
		}

		//延续Task
		template<typename _Fn,typename _R = typename stdx::function_info<_Fn>::result >
		std::shared_ptr<_Task<_R>> then(_Fn &&fn)
		{
			using args_tl = typename stdx::function_info<_Fn>::arguments;
			std::shared_ptr<_Task<_R>> t = _TaskNextBuilder<R,_R,stdx::value_type<stdx::type_at<0,args_tl>>>::build(fn,m_future,m_state,m_lock,m_next);
			return t;
		}

		//合并Task
		template<typename _R>
		std::shared_ptr<_Task<void>> with(std::shared_ptr<_Task<_R>> other)
		{
			return then([other](stdx::task_result<void>&) 
			{
				other->lock();
			});
		}

	protected:
		stdx::runable_ptr<R> m_action;
		promise_ptr<R> m_promise;
		std::shared_future<R> m_future;
		std::shared_ptr<std::shared_ptr<stdx::basic_task>> m_next;
		state_ptr m_state;
		stdx::spin_lock m_lock;
	};	

	//启动一个Task
	template<typename _Fn, typename ..._Args,typename _R = typename stdx::function_info<_Fn>::result>
	inline stdx::task<_R> async(_Fn &&fn, _Args &&...args)
	{
		return task<_R>::start(fn,args...);
	}

	template<typename _R>
	class _TaskCompleteEvent
	{
	public:
		_TaskCompleteEvent()
			:m_promise(stdx::make_promise_ptr<_R>())
			, m_task([](promise_ptr<_R> promise) 
			{
				return promise->get_future().get();
			},m_promise)
		{}
		~_TaskCompleteEvent()=default;
		void set_value(const _R &value)
		{
			m_promise->set_value(value);
		}
		void set_exception(const std::exception_ptr &error)
		{
			m_promise->set_exception(error);
		}
		stdx::task<_R> &get_task()
		{
			return m_task;
		}
		void run()
		{
			m_task.run();
		}
		void run_on_this_thread()
		{
			m_task.run_on_this_thread();
		}
	private:
		promise_ptr<_R> m_promise;
		stdx::task<_R> m_task;
	};

	template<>
	class _TaskCompleteEvent<void>
	{
	public:
		_TaskCompleteEvent()
			:m_promise(stdx::make_promise_ptr<void>())
			, m_task([](promise_ptr<void> promise)
		{
			return promise->get_future().get();
		}, m_promise)
		{}
		~_TaskCompleteEvent() = default;
		void set_value()
		{
			m_promise->set_value();
		}
		void set_exception(const std::exception_ptr &error)
		{
			m_promise->set_exception(error);
		}
		stdx::task<void> &get_task()
		{
			return m_task;
		}
		void run()
		{
			m_task.run();
		}
		void run_on_this_thread()
		{
			m_task.run_on_this_thread();
		}
	private:
		promise_ptr<void> m_promise;
		stdx::task<void> m_task;
	};

	template<typename _R>
	class task_complete_event
	{
		using impl_t = std::shared_ptr<_TaskCompleteEvent<_R>>;
	public:
		task_complete_event()
			:m_impl(std::make_shared<_TaskCompleteEvent<_R>>())
		{}
		task_complete_event(const task_complete_event<_R> &other)
			:m_impl(other.m_impl)
		{}
		~task_complete_event()=default;
		task_complete_event<_R> &operator=(const task_complete_event<_R> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		bool operator==(const task_complete_event<_R> &other) const
		{
			return other.m_impl == m_impl;
		}

		void set_value(const _R &value)
		{
			m_impl->set_value(value);
		}
		void set_exception(const std::exception_ptr &error)
		{
			m_impl->set_exception(error);
		}
		stdx::task<_R> &get_task()
		{
			return m_impl->get_task();
		}
		void run()
		{
			m_impl->run();
		}
		void run_on_this_thread()
		{
			m_impl->run_on_this_thread();
		}
	private:
		impl_t m_impl;
	};

	template<>
	class task_complete_event<void>
	{
		using impl_t = std::shared_ptr<_TaskCompleteEvent<void>>;
	public:
		task_complete_event()
			:m_impl(std::make_shared<_TaskCompleteEvent<void>>())
		{}
		task_complete_event(const task_complete_event<void> &other)
			:m_impl(other.m_impl)
		{}
		~task_complete_event() = default;
		task_complete_event<void> &operator=(const task_complete_event<void> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		bool operator==(const task_complete_event<void> &other) const
		{
			return other.m_impl == m_impl;
		}
		void set_value()
		{
			m_impl->set_value();
		}
		void set_exception(const std::exception_ptr &error)
		{
			m_impl->set_exception(error);
		}
		stdx::task<void> &get_task()
		{
			return m_impl->get_task();
		}
		void run()
		{
			m_impl->run();
		}
		void run_on_this_thread()
		{
			m_impl->run_on_this_thread();
		}
	private:
		impl_t m_impl;
	};
}