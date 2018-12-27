#pragma once
#include <thread>
#include <functional>
#include <queue>
#include <stdx/async/barrier.h>
#include <vector>
#include <initializer_list>
#include <stdx/function.h>
#include <memory>

namespace stdx
{
	//线程池
	class thread_pool
	{
		using runable_ptr = std::shared_ptr<stdx::runable<void>>;
	public:
		thread_pool()
			:free_count(0)
			, task_queue(std::make_shared<std::queue<runable_ptr>>())
			, m_barrier()
		{

		}

		~thread_pool()
		{
		}

		thread_pool(const thread_pool&) = delete;

		template<typename _Fn, typename ..._Args>
		void run_task(_Fn &task, _Args &...args)
		{
			if (free_count == 0)
			{
				add_thread();
			}
			runable_ptr c = stdx::make_action<void>(task, args...);
			auto f = std::bind([this](runable_ptr call)
			{
				deduct_free();
				if (call)
				{
					call->run();
				}
				add_free();
			}, c);
			task_queue->push(stdx::make_action(f));
			m_barrier.pass();
			if ((free_count > (std::thread::hardware_concurrency())) && (task_queue->empty()))
			{
				deduct_thread();
			}
		}


		void add_thread()
		{
			add_free();
			std::thread t([](std::shared_ptr<std::queue<runable_ptr>> tasks, stdx::barrier barrier)
			{
				try
				{
					while (1)
					{
						barrier.wait();
						if (!(tasks->empty()))
						{
							runable_ptr t = tasks->front();
							tasks->pop();
							t->run();
						}
						else
						{
							return;
						}
					}
				}
				catch (const std::exception&)
				{

				}
			}, task_queue, m_barrier);
			t.detach();
		}

		void deduct_thread()
		{
			deduct_free();
			m_barrier.pass();
		}

		const static std::shared_ptr<stdx::thread_pool> get()
		{
			return default;
		}

		void add_free()
		{
			free_count++;
		}

		void deduct_free()
		{
			free_count--;
		}
	private:
		std::atomic_int free_count;
		std::shared_ptr<std::queue<runable_ptr>> task_queue;
		stdx::barrier m_barrier;
		static std::shared_ptr<stdx::thread_pool> default;
	};
	std::shared_ptr<stdx::thread_pool> stdx::thread_pool::default = std::make_shared<stdx::thread_pool>();
}
#define THREAD_POOL std::shared_ptr<stdx::thread_pool>
#define GET_THREAD_POOL() stdx::thread_pool::get()
