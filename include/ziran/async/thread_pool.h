#pragma once
#include <thread>
#include <functional>
#include <queue>
#include <ziran/async/barrier.h>
#include <vector>
#include <initializer_list>
namespace ziran
{
	namespace async
	{
		//线程状态
		enum thread_status
		{
			//空闲
			free = 0,
			//创建中
			creating,
			//运行或关闭中
			working,
			//关闭
			close,
		};
		//线程
		class thread
		{
		public:
			//默认构造函数
			thread()
				:task_queue()
				,status(ziran::async::thread_status::creating)
				,barrier()
				,keep_alive(true)
				, thread_ptr(std::make_shared<std::thread>([this]() {private_run(); }))
				,id(thread_ptr->get_id())
			{
				if (thread_ptr->joinable())
				{
					thread_ptr->detach();
				}
			}
			//构造函数
			thread(std::initializer_list<std::function<void()>> &&tasks)
				:task_queue()
				,status(ziran::async::thread_status::creating)
				,barrier()
				,keep_alive(true)
				, thread_ptr(std::make_shared<std::thread>([this]() {private_run(); }))
				, id(thread_ptr->get_id())
			{
				if (thread_ptr->joinable())
				{
					thread_ptr->detach();
				}
				if (tasks.size())
				{
					for (auto begin = tasks.begin(), end = tasks.end(); begin != end; begin++)
					{
						task_queue.push(*begin);
					}
					barrier.pass();
				}
			}
			//析构函数
			~thread()
			{
				if (!thread_ptr)
				{
					return;
				}
				if (keep_alive)
				{
					shutdown();
				}	
			}

			//不要调用
			void private_run()
			{
				while (keep_alive)
				{
					status = ziran::async::thread_status::free;
					barrier.wait();
					status = ziran::async::thread_status::working;
					bool i = task_queue.size();
					while (i)
					{
						if (!keep_alive)
						{
							status = ziran::async::thread_status::close;
							return;
						}
						std::function<void()> task = task_queue.front();
						task_queue.pop();
						task();
						if (!keep_alive)
						{
							status = ziran::async::thread_status::close;
							return;
						}
						i = task_queue.size();
					}	
				}
				status = ziran::async::thread_status::close;
			}
			//运行任务
			void run(std::function<void()> task)
			{
				if (!keep_alive)
				{
					return;
				}
				task_queue.push(task);
				barrier.pass();
			}
			//关闭
			void shutdown()
			{
				keep_alive = false;
				barrier.pass();
			}
			//获取状态
			const ziran::async::thread_status &get_status() const
			{
				return status;
			}

			const bool &is_free() const
			{
				return status == ziran::async::thread_status::free;
			}

			const bool &is_alive() const
			{
				return keep_alive;
			}
		private:
			std::queue<std::function<void()>> task_queue;
			ziran::async::thread_status status;
			ziran::async::barrier barrier;
			std::atomic_bool keep_alive;
			std::shared_ptr<std::thread> thread_ptr;
			std::thread::id id;
		};

		using shared_thread = std::shared_ptr<ziran::async::thread>;
		using unique_thread = std::unique_ptr<ziran::async::thread>;



		//线程池
		class thread_pool
		{
		public:
			thread_pool(const int& initializ_thread_count = std::thread::hardware_concurrency())
				:initializ_thread_count(initializ_thread_count)
				,barrier(std::move(std::make_shared<ziran::async::barrier>()))
			{

			}
			~thread_pool()
			{

			}
			thread_pool(const thread_pool&) = delete;
			void run_task(std::function<void()> &&task,const bool &long_running=false)
			{
				if (long_running)
				{
					add_thread();
				}
				if (threads.size() < initializ_thread_count)
				{
					add_thread();
				}
				task_queue.push(task);
				barrier->pass();
			}

			const bool has_task() const
			{
				return !task_queue.empty();
			}

			std::function<void()> pop_task()
			{
				std::function<void()> task = task_queue.front();
				task_queue.pop();
				return task;
			}

			void wait_for_task()
			{
				return barrier->wait();
			}

			void add_thread()
			{
				auto ptr = std::make_shared<ziran::async::thread>();
				ptr->run([this,ptr]() 
				{
					while (ptr->is_alive())
					{
						wait_for_task();
						if (has_task())
						{
							auto task = pop_task();
							task();
						}
					}
				});
				threads.push_back(ptr);
			}
		private:
			int initializ_thread_count;
			ziran::async::shared_barrier barrier;
			std::vector<ziran::async::shared_thread> threads;
			std::queue<std::function<void()>> task_queue;
		};
	}
}