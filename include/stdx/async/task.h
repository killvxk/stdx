#pragma once
#include <stdx/async/threadpool.h>
#include <stdx/async/spin_lock.h>
#include <memory>
#include <future>
#include <stdx/traits/ref_type.h>
#include <stdx/function.h>
#include <stdx/tuple.h>
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
		explicit task(_Fn &fn, _Args &...args)
			:m_impl(std::make_shared<_Task<_R>>(fn, args...))
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

		template<typename _Fn, typename ..._Args>
		static task<_R> start(_Fn &fn, _Args &...args)
		{
			auto t = task<_R>(fn, args...);
			t.run();
			return t;
		}

		template<typename _Fn, typename __R = stdx::function_info<_Fn>::result>
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
			return m_impl;
		}
	private:
		impl_t m_impl;
	};

	//_TaskCompleter模板
	template<typename _t>
	struct _TaskCompleter
	{
		static void call(stdx::runable_ptr<_t> &call, std::shared_ptr<std::promise<_t>> &promise, std::shared_ptr<std::shared_ptr<stdx::_BasicRunable<void>>> next, stdx::spin_lock lock, std::shared_ptr<int> state)
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
				//如果有callback
				if (*next)
				{
					//解锁
					lock.unlock();
					//运行callback
					(*next)->run();
				}
				//设置状态为错误
				*state = task_state::error;
				promise->set_exception(std::current_exception());
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
				(*next)->run();
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
		static void call(stdx::runable_ptr<void> &call, std::shared_ptr<std::promise<void>> &promise, std::shared_ptr<std::shared_ptr<stdx::_BasicRunable<void>>> next, stdx::spin_lock lock, std::shared_ptr<int> state)
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
				//如果有callback
				if (*next)
				{
					//解锁
					lock.unlock();
					//运行callback
					(*next)->run();
				}
				//设置状态为错误
				*state = task_state::error;
				promise->set_exception(std::current_exception());
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
				(*next)->run();
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
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<Input> &future, std::shared_ptr<int> state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::_BasicRunable<void>>> next)
		{
			static_assert(sizeof(char) == sizeof(double), "the input function is not be allowed");
			return nullptr;
		}
	};

	template<typename Input, typename Result>
	struct _TaskNextBuilder<Input,Result,void>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<Input> &future, std::shared_ptr<int> state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::_BasicRunable<void>>> next)
		{
			auto t = _Task<Result>::make([](Fn &&fn, std::shared_future<Input> &future)
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
			lock.unlock();
			return t;
		}
	};
	template<typename Input, typename Result>
	struct _TaskNextBuilder<Input, Result, stdx::task_result<Input>>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<Input> &future, std::shared_ptr<int> state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::_BasicRunable<void>>> next)
		{
			auto t = _Task<Result>::make([](Fn &&fn, std::shared_future<Input> &future)
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
			lock.unlock();

		}
	};

	template<typename Input, typename Result>
	struct _TaskNextBuilder<Input, Result,Input>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<Input> &future, std::shared_ptr<int> state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::_BasicRunable<void>>> next)
		{
			auto t = _Task<Result>::make([](Fn &&fn, std::shared_future<Input> &future)
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
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<stdx::task<Input>> &future, ...)
		{
			auto t = future.get();
			return t.then<Fn>(fn);
		}
	};

	template<typename Input, typename Result>
	struct _TaskNextBuilder<stdx::task<Input>, Result, stdx::task_result<Input>>
	{
		template<typename Fn>
		static std::shared_ptr<_Task<Result>> build(Fn &&fn, std::shared_future<stdx::task<Input>> &future, ...)
		{
			return future.get().then(std::move(fn));
		}
	};

	//Task模板的实现
	template<typename R>
	class _Task :public stdx::_BasicRunable<void>
	{
	public:
		//构造函数
		template<typename _Fn, typename ..._Args>
		explicit _Task(_Fn &&f, _Args &&...args)
			:m_action(stdx::make_runable<R>(std::move(f), args...))
			, m_promise(std::make_shared<std::promise<R>>())
			, m_future(m_promise->get_future())
			, m_next(std::make_shared<std::shared_ptr<stdx::_BasicRunable<void>>>(nullptr))
			, m_state(std::make_shared<int>(stdx::task_state::ready))
			, m_lock()

		{
		}

		template<typename _Fn>
		explicit _Task(_Fn &&f)
			:m_action(stdx::make_runable<R>(std::move(f)))
			, m_promise(std::make_shared<std::promise<R>>())
			, m_future(m_promise->get_future())
			, m_next(std::make_shared<std::shared_ptr<stdx::_BasicRunable<void>>>(nullptr))
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
				, std::shared_ptr<std::promise<R>> promise
				, std::shared_ptr<std::shared_ptr<stdx::_BasicRunable<void>>>  next
				, stdx::spin_lock lock
				, std::shared_ptr<int> state)
			{
				stdx::_TaskCompleter<R>::call(r, promise, next, lock, state);
			};
			//放入线程池
			stdx::threadpool::run(f, m_action, m_promise, m_next, m_lock, m_state);
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

		template<typename _Fn, typename ..._Args>
		static std::shared_ptr<_Task<R>> make(_Fn &fn, _Args &...args)
		{
			return std::make_shared<_Task<R>>(fn, args...);
		}

		//延续Task
		template<typename _Fn,typename _R = stdx::function_info<_Fn>::result>
		std::shared_ptr<_Task<_R>> then(_Fn &&fn)
		{
			using args_tl = stdx::function_info<_Fn>::arguments;
			std::shared_ptr<_Task<_R>> t = _TaskNextBuilder<R,_R,stdx::type_at<0,args_tl>>::build(fn,m_future,m_state,m_lock,m_next);
			return t;
		}

		//合并Task
		template<typename _R>
		std::shared_ptr<_Task<void>> with(std::shared_ptr<_Task<_R>> other)
		{
			return then([other](stdx::task_result<void>&) 
			{
				other->wait();
			});
		}

	protected:
		stdx::runable_ptr<R> m_action;
		std::shared_ptr<std::promise<R>> m_promise;
		std::shared_future<R> m_future;
		std::shared_ptr<std::shared_ptr<stdx::_BasicRunable<void>>> m_next;
		std::shared_ptr<int> m_state;
		stdx::spin_lock m_lock;
	};	

	//启动一个Task
	template<typename _Fn, typename ..._Args,typename _R = stdx::function_info<_Fn>::result>
	stdx::task<_R> async(_Fn &fn, _Args &...args)
	{
		return task<_R>::start(fn,args...);
	}

	template<typename R>
	class _SyncTask:public _Task<R>
	{
	public:
		template<typename _Fn,typename ..._Args>
		_SyncTask(_Fn &&fn, _Args &&...args)
			:_Task<R>(std::move(fn),args...)
		{}
		void run() override
		{
			m_promise->set_value(m_action->run());
		}

	};

	template<>
	class _SyncTask<void> :public _Task<void>
	{
	public:
		template<typename _Fn, typename ..._Args>
		_SyncTask(_Fn &&fn, _Args &&...args)
			:_Task<void>(std::move(fn), args...)
		{}
		void run() override
		{
			m_action->run();
			m_promise->set_value();
		}
	};

	template<typename _Result>
	class _TaskCompleteEvent
	{
	public:
		_TaskCompleteEvent()
			:m_promise()
			,m_future(m_promise.get_future())
		{}
		~_TaskCompleteEvent()=default;
		void set_value(const _Result &value)
		{
			m_promise.set_value(value);
		}
		void set_exception(std::exception_ptr exception_ptr)
		{
			m_promise.set_exception(exception_ptr);
		}

		stdx::task<_Result> get_task()
		{
			return task<_Result>(nullptr);
		}
	private:
		std::promise<_Result> m_promise;
		std::shared_future<_Result> m_future;
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
		task_complete_event(task_complete_event<_R> &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~task_complete_event() = default;
		task_complete_event<_R> &operator=(const task_complete_event<_R> &other)
		{
			m_impl = other.m_impl;
		}
		void set_value(const _R &value)
		{
			m_impl->set_value(value);
		}
		void set_exception(std::exception_ptr error)
		{
			m_impl->set_exception(error);
		}
		stdx::task<_R> get_task()
		{
			return m_impl->get_task();
		}
	private:
		impl_t m_impl;
	};
}