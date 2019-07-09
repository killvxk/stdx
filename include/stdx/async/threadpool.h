#pragma once
#include <stdx/env.h>
#include <thread>
#include <queue>
#include <stdx/async/barrier.h>
#include <stdx/async/atomic_queue.h>
#include <stdx/function.h>
#include <memory>

namespace stdx
{
	//线程池
	class _Threadpool
	{
		using runable_ptr = std::shared_ptr<stdx::_BasicRunable<void>>;
	public:
		//构造函数
		_Threadpool()
			:m_free_count(std::make_shared<uint32>())
			,m_count_lock()
			, m_alive(std::make_shared<bool>(true))
			, m_task_queue(std::make_shared<std::queue<runable_ptr>>())
			, m_barrier()
			, m_lock()
		{
			//初始化线程池
			init_threads();
		}

		//析构函数
		~_Threadpool()
		{
			//终止时设置状态
			*m_alive = false;
		}

		//删除复制构造函数
		_Threadpool(const _Threadpool&) = delete;

		//执行任务
		template<typename _Fn, typename ..._Args>
		void run(_Fn &&task, _Args &&...args)
		{
			m_count_lock.lock();
			if (*m_free_count ==0)
			{
				*m_free_count+=1;
				add_thread();
			}
			m_count_lock.unlock();
			m_task_queue->emplace(stdx::make_runable<void>(std::move(task), args...));
			m_barrier.pass();
		}

		 template<typename _Fn,typename ..._Args>
		 void run_lazy(_Fn &task,_Args &&...args)
		 {
			 m_task_queue->emplace(stdx::make_runable<void>(std::move(task), args...));
			 m_barrier.pass();
		 }


	private:
		std::shared_ptr<uint32> m_free_count;
		stdx::spin_lock m_count_lock;
		std::shared_ptr<bool> m_alive;
		std::shared_ptr<std::queue<runable_ptr>> m_task_queue;
		stdx::barrier m_barrier;
		stdx::spin_lock m_lock;

		//添加线程
		void add_thread()
		{
			//创建线程
			std::thread t([](std::shared_ptr<std::queue<runable_ptr>> tasks, stdx::barrier barrier, stdx::spin_lock lock, std::shared_ptr<uint32> count,stdx::spin_lock count_lock,std::shared_ptr<bool> alive)
			{
				//如果存活
				while (*alive)
				{
					//等待通知
					if (!barrier.wait_for(std::chrono::minutes(10)))
					{
						//如果10分钟后未通知
						//退出线程
						count_lock.lock();
						*count -=1;
						count_lock.unlock();
						return;
					}
					if (!(tasks->empty()))
					{
						//如果任务列表不为空
						//减去一个计数
						*count -= 1;
						//进入自旋锁
						lock.lock();
						//获取任务
						runable_ptr t(std::move(tasks->front()));
						//从queue中pop
						tasks->pop();
						//解锁
						lock.unlock();
						//执行任务
						try
						{
							if (t)
							{
								t->run();
							}
						}
						catch (const std::exception &)
						{
							//忽略出现的错误
						}
						//完成或终止后
						//添加计数
						count_lock.lock();
						*count += 1;
						count_lock.unlock();
					}
					else
					{
						continue;
					}
				}
			}, m_task_queue, m_barrier, m_lock, m_free_count,m_count_lock,m_alive);
			//分离线程
			t.detach();
		}
		
		//初始化线程池
		void init_threads()
		{
			unsigned int cores = (std::thread::hardware_concurrency()+1)<<1;
			*m_free_count+=cores;
			for (unsigned int i = 0; i < cores; i++)
			{
				add_thread();
			}
		}
	};
	using threadpool_ptr = std::shared_ptr<stdx::_Threadpool>;
	//线程池静态类
	class threadpool
	{
	public:
		~threadpool() = default;
		using impl_t = std::shared_ptr<stdx::_Threadpool>;
		//执行任务
		template<typename _Fn,typename ..._Args>
		static void run(_Fn &&fn,_Args &&...args)
		{
			m_impl->run(std::move(fn),args...);
		}

		template<typename _Fn, typename ..._Args>
		static void run_lazy(_Fn &&fn, _Args &&...args)
		{
			m_impl->run_lazy(std::move(fn),args...);
		}

		template<typename _Fn,typename _Cond, typename ..._Args>
		static void run_lazy_if(_Cond &cond,_Fn &&fn, _Args &&...args)
		{
			static_assert(stdx::is_result_type<_Cond, bool>, "the input function is not be allowed");
			if (std::invoke(cond))
			{
				run_lazy(std::move(fn), args...);
			}
			else
			{
				run(std::move(fn),args...);
			}
		}
	private:
		threadpool() = default;
		const static impl_t m_impl;
	};
	const stdx::threadpool::impl_t stdx::threadpool::m_impl = std::make_shared <stdx::_Threadpool>();
}