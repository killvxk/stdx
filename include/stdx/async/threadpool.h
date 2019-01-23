#pragma once
#include <thread>
#include <queue>
#include <stdx/async/barrier.h>
#include <stdx/async/spin_lock.h>
#include <stdx/function.h>
#include <memory>

namespace stdx
{
	//自由线程计数器实现
	class _FreeCount
	{
	public:
		_FreeCount()
			:m_count(0)
		{}
		~_FreeCount() = default;
		void add()
		{
			++m_count;
		}
		void add(unsigned int i)
		{
			m_count + 1;
		}
		void deduct()
		{
			--m_count;
		}
		void deduct(unsigned int i)
		{
			m_count - i;
		}
		operator unsigned int() const
		{
			return (unsigned int)m_count;
		}
	private:
		std::atomic_uint m_count;
	};
	//自由线程计数器
	class free_count
	{
		using impl_t = std::shared_ptr<stdx::_FreeCount>;
	public:
		free_count()
			:m_impl(std::make_shared<stdx::_FreeCount>())
		{}
		free_count(const free_count &other)
			:m_impl(other.m_impl)
		{}
		free_count(free_count &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~free_count() = default;
		free_count &operator=(const free_count &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		void add()
		{
			m_impl->add();
		}
		void add(unsigned int i)
		{
			m_impl->add(i);
		}
		void deduct()
		{
			m_impl->deduct();
		}
		void deduct(unsigned int i)
		{
			m_impl->deduct(i);
		}
		operator unsigned int()
		{
			return *m_impl;
		}
		void operator++()
		{
			add();
		}
		void operator++(int)
		{
			add();
		}
		void operator--()
		{
			deduct();
		}
		void operator--(int)
		{
			deduct();
		}
		void operator+(unsigned int i)
		{
			add(i);
		}
		void operator-(unsigned int i)
		{
			deduct(i);
		}
	private:
		impl_t m_impl;
	};
	//线程池
	class _Threadpool
	{
		using runable_ptr = std::shared_ptr<stdx::_BasicAction<void>>;
	public:
		//构造函数
		_Threadpool()
			:m_free_count()
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
		void run_task(_Fn &task, _Args &...args)
		{
			if (m_free_count == 0)
			{
				add_thread();
				m_free_count.add();
			}
			runable_ptr c = stdx::_MakeAction<void>(task, args...);
			m_task_queue->push(c);
			m_barrier.pass();
		}

	private:
		stdx::free_count m_free_count;
		std::shared_ptr<bool> m_alive;
		std::shared_ptr<std::queue<runable_ptr>> m_task_queue;
		stdx::barrier m_barrier;
		stdx::spin_lock m_lock;

		//添加线程
		void add_thread()
		{
			//创建线程
			std::thread t([](std::shared_ptr<std::queue<runable_ptr>> tasks, stdx::barrier barrier, stdx::spin_lock lock, stdx::free_count count,std::shared_ptr<bool> alive)
			{
				//如果存活
				while (*alive)
				{
					//等待通知
					if (!barrier.wait_for(std::chrono::minutes(10)))
					{
						//如果10分钟后未通知
						//退出线程
						count.deduct();
						return;
					}
					if (!(tasks->empty()))
					{
						//如果任务列表不为空
						//减去一个计数
						count.deduct();
						//进入自旋锁
						lock.lock();
						//获取任务
						runable_ptr t = tasks->front();
						//从queue中pop
						tasks->pop();
						//解锁
						lock.unlock();
						//执行任务
						try
						{
							t->run();
						}
						catch (const std::exception &)
						{
							//忽略出现的错误
						}
						//完成或终止后
						//添加计数
						count.add();
					}
					else
					{
						continue;
					}
				}
			}, m_task_queue, m_barrier, m_lock, m_free_count,m_alive);
			//分离线程
			t.detach();
		}
		
		//初始化线程池
		void init_threads()
		{
			unsigned int cores = (std::thread::hardware_concurrency()+1)<<1;
			for (unsigned int i = 0; i < cores; i++)
			{
				add_thread();
			}
			m_free_count.add(cores);
		}
	};

	//线程池静态类
	class threadpool
	{
	public:
		~threadpool() = default;
		using impl_t = std::shared_ptr<stdx::_Threadpool>;
		//执行任务
		template<typename _Fn,typename ..._Args>
		static void run(_Fn &fn,_Args &...args)
		{
			m_impl->run_task(fn,args...);
		}
	private:
		threadpool() = default;
		const static impl_t m_impl;
	};
	const stdx::threadpool::impl_t stdx::threadpool::m_impl = std::make_shared <stdx::_Threadpool>();
}