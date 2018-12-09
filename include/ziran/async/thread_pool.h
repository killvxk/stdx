#pragma once
#include <thread>
#include <functional>
#include <queue>
#include <ziran/async/barrier.h>
#include <vector>
#include <initializer_list>
#include <ziran/function.h>
#include <memory>

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
			using runable = std::shared_ptr<ziran::runable<void>>;
		public:
			//默认构造函数
			loop_thread()
				:task_queue(std::make_shared<std::queue<runable>>())
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
			loop_thread(const std::shared_ptr<std::queue<runable>> &task_queue_ptr,const ziran::async::shared_barrier &barrier_ptr)
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
					if (!keep_alive)
					{
						return;
					}
					bool i(true);
					try
					{
						i = task_queue->empty();
					}
					catch (const std::exception&)
					{
						return;
					}
					if(!i)
					{
						if (!keep_alive)
						{
							status = ziran::async::thread_status::close;
							return;
						}
						status = ziran::async::thread_status::working;
						runable task = task_queue->front();
						task_queue->pop();
						task->run();
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
			std::shared_ptr<std::queue<runable>> task_queue;
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
			using runable_ptr = std::shared_ptr<ziran::runable<void>>;
		public:
			thread_pool(const int& initializ_thread_count = std::thread::hardware_concurrency())
				:free_count(0)
				,task_queue(std::make_shared<std::queue<runable_ptr>>())
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

			template<typename _Fn,typename ..._Args>
			void run_task(_Fn &task, _Args &...args)
			{
				if (free_count == 0)
				{
					add_thread();
				}
				runable_ptr c = ziran::make_action<void>(task, args...);
				auto f = std::bind([this](runable_ptr call)
				{
					deduct_free();
					if (call)
					{
						call->run();
					}
					add_free();
				},c);
				task_queue->push(ziran::make_action(f));
				barrier->pass();
			}


			void add_thread()
			{
				free_count += 1;
				auto ptr = std::make_shared<ziran::async::loop_thread>(task_queue,barrier);
				threads.push_back(ptr);
			}
			
			const static std::shared_ptr<ziran::async::thread_pool> get()
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
			ziran::async::shared_barrier barrier;
			std::vector<ziran::async::shared_loop_thread> threads;
			static std::shared_ptr<ziran::async::thread_pool> default;
		};
		std::shared_ptr<ziran::async::thread_pool> ziran::async::thread_pool::default = std::make_shared<ziran::async::thread_pool>();
	}
}
#define THREAD_POOL std::shared_ptr<ziran::async::thread_pool>
#define GET_THREAD_POOL() ziran::async::thread_pool::get()
