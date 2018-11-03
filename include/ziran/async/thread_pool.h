#pragma once
#include <thread>
#include <functional>
#include <queue>
#include <ziran/async/barrier.h>
#include <vector>
#include <initializer_list>
#include <ziran/async/spin_lock.h>
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
		class loop_thread
		{
		public:
			//默认构造函数
			loop_thread()
				:task_queue(std::make_shared<std::queue<std::function<void()>>>())
				,status(ziran::async::thread_status::creating)
				,barrier(std::make_shared<ziran::async::barrier>())
				,keep_alive(true)
				,thread_ptr(std::make_shared<std::thread>([this]() {private_run(); }))
				,id(thread_ptr->get_id())
			{
				if (thread_ptr->joinable())
				{
					thread_ptr->detach();
				}
			}
			//构造函数
			loop_thread(const std::shared_ptr<std::queue<std::function<void()>>> &task_queue_ptr,const ziran::async::shared_barrier &barrier_ptr)
				:task_queue(task_queue_ptr)
				, status(ziran::async::thread_status::creating)
				, barrier(barrier_ptr)
				, keep_alive(true)
				, thread_ptr(std::make_shared<std::thread>([this]() {private_run(); }))
				, id(thread_ptr->get_id())
			{
				if (thread_ptr->joinable())
				{
					thread_ptr->detach();
				}
			}
			//析构函数
			~loop_thread()
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
					barrier->wait();
					bool i = task_queue->empty();
					if(!i)
					{
						if (!keep_alive)
						{
							status = ziran::async::thread_status::close;
							return;
						}
						status = ziran::async::thread_status::working;
						std::function<void()> task = task_queue->front();
						task_queue->pop();
						task();
					}	
				}
				status = ziran::async::thread_status::close;
			}
			//运行任务
			template<typename _Fn,typename ..._Args>
			void run(_Fn task,_Args &&...args)
			{
				if (!keep_alive)
				{
					return;
				}
				task_queue->push(std::bind(task,args...));
				barrier->pass();
			}
			//关闭
			void shutdown()
			{
				keep_alive = false;
				barrier->pass();
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
			std::shared_ptr<std::queue<std::function<void()>>> task_queue;
			ziran::async::thread_status status;
			ziran::async::shared_barrier barrier;
			std::atomic_bool keep_alive;
			std::shared_ptr<std::thread> thread_ptr;
			std::thread::id id;
		};

		using shared_loop_thread = std::shared_ptr<ziran::async::loop_thread>;
		using unique_loop_thread = std::unique_ptr<ziran::async::loop_thread>;

		//线程池
		class thread_pool
		{
		public:
			thread_pool(const int& initializ_thread_count = std::thread::hardware_concurrency())
				:task_queue(std::make_shared<std::queue<std::function<void()>>>())
				,initializ_thread_count(initializ_thread_count)
				,barrier(std::move(std::make_shared<ziran::async::barrier>()))
			{

			}
			~thread_pool()
			{
				std::for_each(std::begin(threads), std::end(threads), [](shared_loop_thread ptr) 
				{
					ptr->shutdown();
				});
			}
			thread_pool(const thread_pool&) = delete;

			template<typename _Fn, typename ..._Args>
			void run_task(_Fn task, _Args &&...args)
			{
				if (threads.size() < initializ_thread_count || task_queue->size() > initializ_thread_count)
				{
					add_thread();
				}
				task_queue->push(std::bind(task, args...));
				barrier->pass();
			}

			const bool has_task() const
			{
				return !task_queue->empty();
			}

			std::function<void()> pop_task()
			{
				std::function<void()> task = task_queue->front();
				task_queue->pop();
				return task;
			}

			void wait_for_task()
			{
				return barrier->wait();
			}

			void add_thread()
			{
				auto ptr = std::make_shared<ziran::async::loop_thread>(task_queue,barrier);
				threads.push_back(ptr);
			}
			
			const static std::shared_ptr<ziran::async::thread_pool> get()
			{
				return default;
			}

		private:
			std::shared_ptr<std::queue<std::function<void()>>> task_queue;
			int initializ_thread_count;
			ziran::async::shared_barrier barrier;
			std::vector<ziran::async::shared_loop_thread> threads;
			static ziran::async::spin_lock<-1> lock;
			static std::shared_ptr<ziran::async::thread_pool> default;
		};
		std::shared_ptr<ziran::async::thread_pool> ziran::async::thread_pool::default = std::make_shared<ziran::async::thread_pool>();
	}
}