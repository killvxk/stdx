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

		template<typename __R = void, typename _Fn>
		task<__R> then(_Fn &&fn)
		{
			return task<__R>(m_impl->then<__R>(fn));
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

	private:
		impl_t m_impl;
	};

	//_TaskCompleter模板
	template<typename _t>
	struct _TaskCompleter
	{
		static void set_value(stdx::action<_t> &call, std::shared_ptr<std::promise<_t>> &promise, std::shared_ptr<std::shared_ptr<stdx::_BasicAction<void>>> next, stdx::spin_lock lock, std::shared_ptr<int> state)
		{
			try
			{
				//调用方法
				//设置promise
				promise->set_value(call());
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
		static void set_value(stdx::action<void> &call, std::shared_ptr<std::promise<void>> &promise, std::shared_ptr<std::shared_ptr<stdx::_BasicAction<void>>> next, stdx::spin_lock lock, std::shared_ptr<int> state)
		{
			try
			{
				//调用方法
				call();
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
	//_TaskNextBuilder模板
	template<typename _t, typename  _r>
	struct _TaskNextBuilder
	{
		template<typename _fn>
		static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<_t> &future, std::shared_ptr<int> state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::_BasicAction<void>>> next)
		{
			//创建回调Task
			auto t = _Task<_r>::make([](_fn &&fn, std::shared_future<_t> &future)
			{
				return std::bind(fn, task_result<_t>(future))();
			}, fn, future);
			//加锁
			lock.lock();
			//如果已经完成
			if ((*state == task_state::complete) || (*state == task_state::error))
			{
				//解锁
				lock.unlock();
				//运行
				t->run();
				return t;
			}
			//未完成则设置回调
			*next = t;
			//解锁
			lock.unlock();
			return t;

		}
	};

	//用于实现返回Task的Task回调
	template<typename _t, typename _r>
	struct _TaskNextBuilder<std::shared_ptr<_Task<_t>>, _r>
	{
		template<typename _fn>
		static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<_Task<_t>> &future, std::shared_ptr<int> state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::_BasicAction<void>>> next)
		{
			//获取Task
			std::shared_ptr<_Task<_t>> t = future.get();
			//调用then
			return	t->then(fn);
		}
	};

	template<typename _t, typename _r>
	struct _TaskNextBuilder<task<_t>, _r>
	{
		template<typename _fn>
		static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<task<_t>> &future, std::shared_ptr<int> state, stdx::spin_lock lock, std::shared_ptr<std::shared_ptr<stdx::_BasicAction<void>>> next)
		{
			//获取Task
			auto t = future.get();
			//调用then
			return	t.then(fn);
		}
	};


	//Task模板的实现
	template<typename R>
	class _Task :public stdx::_BasicAction<void>
	{
	public:
		//构造函数
		template<typename _Fn, typename ..._Args>
		explicit _Task(_Fn &&f, _Args ...args)
			:m_action(stdx::make_action<R>(std::bind(f, args...)))
			, m_promise(std::make_shared<std::promise<R>>())
			, m_future(m_promise->get_future())
			, m_next(std::make_shared<std::shared_ptr<stdx::_BasicAction<void>>>(nullptr))
			, m_state(std::make_shared<int>(stdx::task_state::ready))
			, m_lock()

		{
		}

		template<typename _Fn>
		explicit _Task(_Fn &&f)
			:m_action(stdx::make_action<R>(f))
			, m_promise(std::make_shared<std::promise<R>>())
			, m_future(m_promise->get_future())
			, m_next(std::make_shared<std::shared_ptr<stdx::_BasicAction<void>>>(nullptr))
			, m_state(std::make_shared<int>(stdx::task_state::ready))
			, m_lock()
		{
		}

		//析构函数
		~_Task() = default;

		//启动一个Task
		void run() override
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
			auto f = [](stdx::action<R> r
				, std::shared_ptr<std::promise<R>> promise
				, std::shared_ptr<std::shared_ptr<stdx::_BasicAction<void>>>  next
				, stdx::spin_lock lock
				, std::shared_ptr<int> state)
			{
				stdx::_TaskCompleter<R>::set_value(r, promise, next, lock, state);
			};
			//放入线程池
			stdx::threadpool::run(std::bind(f, m_action, m_promise, m_next, m_lock, m_state));
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
		template<typename _R = void, typename _Fn>
		std::shared_ptr<_Task<_R>> then(_Fn &&fn)
		{
			std::shared_ptr<_Task<_R>> t = stdx::_TaskNextBuilder<R, _R>::build(fn, m_future, m_state, m_lock, m_next);
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

	private:
		stdx::action<R> m_action;
		std::shared_ptr<std::promise<R>> m_promise;
		std::shared_future<R> m_future;
		std::shared_ptr<std::shared_ptr<stdx::_BasicAction<void>>> m_next;
		std::shared_ptr<int> m_state;
		stdx::spin_lock m_lock;
	};
	
	//启动一个Task
	template<typename _R = void, typename _Fn, typename ..._Args>
	task<_R> async(_Fn &fn, _Args &...args)
	{
		return task<_R>::start(fn, args...);
	}
}