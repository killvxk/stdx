#pragma once
#include <stdx/async/thread_pool.h>
#include<stdx/async/spin_lock.h>
#include <memory>
#include <future>
namespace stdx
{
	namespace async
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

		//Task模板
		//仅在指针下有意义
		template<typename R>
		class _Task:public stdx::runable<void>
		{
			using runable_ptr = std::shared_ptr<stdx::runable<R>>;
		public:
			//构造函数
			template<typename _Fn, typename ..._Args>
			explicit _Task(_Fn &&f, _Args ...args)
				:m_action(stdx::make_action<R>(std::bind(f, args...)))
				, m_pool(GET_THREAD_POOL())
				, m_promise(std::make_shared<std::promise<R>>())
				, m_future(m_promise->get_future())
				, m_next(std::make_shared<std::shared_ptr<stdx::runable<void>>>(nullptr))
				, m_state(std::make_shared<int>(task_state::ready))
				,m_lock(std::make_shared<stdx::async::spin_lock>())

			{
			}

			template<typename _Fn>
			explicit _Task(_Fn &&f)
				:m_action(stdx::make_action<R>(f))
				, m_pool(GET_THREAD_POOL())
				, m_promise(std::make_shared<std::promise<R>>())
				, m_future(m_promise->get_future())
				, m_next(std::make_shared<std::shared_ptr<stdx::runable<void>>>(nullptr))
				, m_state(std::make_shared<int>(task_state::ready))
				, m_lock(std::make_shared<stdx::async::spin_lock>())
			{
			}

			//析构函数
			~_Task()
			{
			}
			//完成者元函数
			template<typename _t>
			struct completer
			{
				static void set_value(runable_ptr &call, std::shared_ptr<std::promise<R>> &promise, std::shared_ptr<std::shared_ptr<stdx::runable<void>>> next, stdx::async::spin_lock_ptr lock,std::shared_ptr<int> state)
				{
					try 
					{
						//调用方法
						auto &value = call->run();
						//设置promise
						promise->set_value(value);
					}
					catch (const std::exception&)
					{
						//加锁
						lock->lock();
						//如果有callback
						if (*next)
						{
							//解锁
							lock->unlock();
							//运行callback
							(*next)->run();
						}
						//设置状态为错误
						*state = task_state::error;
						promise->set_exception(std::current_exception());
						//解锁
						lock->unlock();
						return;
					}
					//加锁
					lock->lock();
					//如果有callback
					if (*next)
					{
					        *stats＝ task_state::complete;
						//解锁
						lock->unlock();
						//运行callback
						(*next)->run();
						return;
					}
					//设置状态为完成
					*state = task_state::complete;
					//解锁
					lock->unlock();
					return;
				}
			};

			template<>
			struct completer<void>
			{
				static void set_value(runable_ptr &call, std::shared_ptr<std::promise<R>> &promise, std::shared_ptr<std::shared_ptr<stdx::runable<void>>> next,stdx::async::spin_lock_ptr lock, std::shared_ptr<int> state)
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
						lock->lock();
						//如果有callback
						if (*next)
						{
							//解锁
							lock->unlock();
							//运行callback
							(*next)->run();
						}
						//设置状态为错误
						*state = task_state::error;
						promise->set_exception(std::current_exception());
						//解锁
						lock->unlock();
						return;
					}
					//加锁
					lock->lock();
					//如果有callback
					if (*next)
					{
						*state = task_state::complete;
						//解锁
						lock->unlock();
						//运行callback
						(*next)->run();
						return;
					}
					//设置状态为完成
					*state = task_state::complete;
					//解锁
					lock->unlock();
					return;
				}
			};

			//启动一个Task
			void run() override
			{
				//加锁
				m_lock->lock();
				//如果不在运行
				if (*m_state != task_state::running)
				{
					//设置状态运行中
					*m_state = task_state::running;
				}
				else
				{
					//如果正在运行
					//解锁返回
					m_lock->unlock();
					return;
				}
				//解锁
				m_lock->unlock();
				//创建方法
				auto f = [](runable_ptr r
					, std::shared_ptr<std::promise<R>> promise
					, std::shared_ptr<std::shared_ptr<stdx::runable<void>>>  next
					, stdx::async::spin_lock_ptr lock
					, std::shared_ptr<int> state)
				{
					completer<R>::set_value(r, promise, next, lock, state);
				};
				//放入线程池
				m_pool->run_task(std::bind(f, m_action, m_promise, m_next,m_lock,m_state));
			}

			//等待当前Task(不包括后续)完成
			void wait()
			{
				m_future.wait();
			}

			//结果类型
			template<typename _t>
			struct result_t
			{
				using type = _t&;
			};

			template<typename _t>
			struct result_t<_t&>
			{
				using type = _t &;
			};

			template<>
			struct result_t<void>
			{
				using type =void;
			};

			//等待当前Task(不包括后续)完成并获得结果
			//发生异常则抛出异常
			typename result_t<R>::type get()
			{
				return m_future.get();
			}

			//询问Task是否完成
			bool is_complete() const
			{
				auto c = (*m_state == task_state::complete)||(*m_state == task_state::error);
				return c;
			}
			
			//创造一个task_ptr
			template<typename _Fn, typename ..._Args>
			static std::shared_ptr<_Task<R>> make(_Fn &fn,_Args &...args)
			{
				return std::make_shared<_Task<R>>(fn,args...);
			}

			//next_builder元函数
			template<typename _t,typename  _r>
			struct next_builder
			{
				template<typename _fn>
				static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<_t> &future, std::shared_ptr<int> state, stdx::async::spin_lock_ptr lock, std::shared_ptr<std::shared_ptr<stdx::runable<void>>> next)
				{
					//创建回调Task
					auto t = _Task<_r>::make([](_fn &&fn, std::shared_future<_t> &future)
					{
						return std::bind(fn, future.get())();
					}, fn,future);
					//加锁
					lock->lock();
					//如果已经完成
					if ((*state == task_state::complete)||(*state == task_state::error))
					{
						//解锁
						lock->unlock();
						//运行
						t->run();
						return t;
					}
					//未完成则设置回调
					*next = t;
					//解锁
					lock->unlock();
					return t;
	
				}
			};

			template<typename _r>
			struct next_builder<void,_r>
			{
				template<typename _fn>
				static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<void> &future, std::shared_ptr<int> state, stdx::async::spin_lock_ptr lock, std::shared_ptr<std::shared_ptr<stdx::runable<void>>> next)
				{
					//创建回调Task
					auto t= _Task<_r>::make([](_fn &&fn, std::shared_future<void> &future)
					{	
						future.get();
						return fn();
					}, fn, future);
					//加锁
					lock->lock();
					//如果已经完成
					if ((*state == task_state::complete) || (*state == task_state::error))
					{
						//解锁
						lock->unlock();
						//运行
						t->run();
						return t;
					}
					//未完成则设置回调
					*next = t;
					//解锁
					lock->unlock();
					return t;
				}
			};

			//用于实现返回Task的Task回调
			template<typename _t, typename _r>
			struct next_builder<std::shared_ptr<_Task<_t>>, _r>
			{
				template<typename _fn>
				static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<_Task<_t>> &future, std::shared_ptr<int> state, stdx::async::spin_lock_ptr lock, std::shared_ptr<std::shared_ptr<stdx::runable<void>>> next)
				{
					//获取Task
					std::shared_ptr<_Task<_t>> t = future.get();
					//调用then
					return	t->then(fn);
				}
			};

			//延续Task
			template<typename _R = void,typename _Fn>
			std::shared_ptr<_Task<_R>> then(_Fn &&fn)
			{
				std::shared_ptr<_Task<_R>> t = next_builder<R, _R>::build(fn, m_future, m_state, m_lock, m_next);
				return t;
			}

		private:
			runable_ptr m_action;
			THREAD_POOL m_pool;
			std::shared_ptr<std::promise<R>> m_promise;
			std::shared_future<R> m_future;
			std::shared_ptr<std::shared_ptr<stdx::runable<void>>> m_next;
			std::shared_ptr<int> m_state;
			stdx::async::spin_lock_ptr m_lock;
		};
		//_Task指针别名
		template<typename _T>
		using task_ptr = std::shared_ptr<stdx::async::_Task<_T>>;
		//定义宏TASK
#define TASK stdx::async::task_ptr
		//创造Task
		template<typename _R,typename _Fn,typename ..._Args>
		TASK<_R> make_task(_Fn &fn, _Args &...args)
		{
			return  _Task<_R>::make(fn, args...);
		}
		//定义宏MAKE_TASK
#define MAKE_TASK stdx::async::make_task
		template<typename _R, typename _Fn, typename ..._Args>
		//创造并运行TASK
		TASK<_R> run_task(_Fn &fn, _Args &...args)
		{
			auto t = MAKE_TASK<_R>(fn, args...);
			t->run();
			return t;
		}
		//定义宏RUN_TASK
#define RUN_TASK stdx::async::run_task
	}
}

