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
	class thread_pool
	{
		using runable_ptr = std::shared_ptr<stdx::_BasicAction<void>>;
	public:
		thread_pool()
			:m_free_count()
			, task_queue(std::make_shared<std::queue<runable_ptr>>())
			, m_barrier()
			, m_lock()
		{
			init_threads();
		}

		~thread_pool()
		{
		}

		thread_pool(const thread_pool&) = delete;

		template<typename _Fn, typename ..._Args>
		void run_task(_Fn &task, _Args &...args)
		{
			if (_FreeCount == 0)
			{
				add_thread();
			}
			runable_ptr c = stdx::_MakeAction<void>(task, args...);
			task_queue->push(c);
			m_barrier.pass();
		}


		void add_thread()
		{
			std::thread t([](std::shared_ptr<std::queue<runable_ptr>> tasks, stdx::barrier barrier, stdx::spin_lock lock, stdx::free_count count)
			{
				count.add();
				try
				{
					while (1)
					{
						if (!barrier.wait_for(std::chrono::minutes(10)))
						{
							count.deduct();
							return;
						}
						if (!(tasks->empty()))
						{
							count.deduct();
							lock.lock();
							runable_ptr t = tasks->front();
							tasks->pop();
							lock.unlock();
							t->run();
							count.add();
						}
						else
						{
							continue;
						}
					}
				}
				catch (const std::exception&)
				{
					count.deduct();
					return;
				}
			}, task_queue, m_barrier, m_lock, m_free_count);
			t.detach();
		}

		void deduct_thread()
		{
			m_barrier.pass();
		}

		const static std::shared_ptr<stdx::thread_pool> get()
		{
			return default;
		}

	private:
		stdx::free_count m_free_count;
		std::shared_ptr<std::queue<runable_ptr>> task_queue;
		stdx::barrier m_barrier;
		stdx::spin_lock m_lock;
		void init_threads()
		{
			for (unsigned int i = 0, cores = std::thread::hardware_concurrency() * 2; i < cores; i++)
			{
				add_thread();
			}
		}
		static std::shared_ptr<stdx::thread_pool> default;
	};
	std::shared_ptr<stdx::thread_pool> stdx::thread_pool::default = std::make_shared<stdx::thread_pool>();
}
#define THREAD_POOL std::shared_ptr<stdx::thread_pool>
#define GET_THREAD_POOL() stdx::thread_pool::get()