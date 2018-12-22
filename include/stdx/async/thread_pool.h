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
	namespace async
	{
		//线程状态
		struct thread_status
		{
			enum
			{
				//空闲
				free = 0,
				//创建中
				creating,
				//运行或关闭中
				working,
				//关闭
				close
			};
		};
		//线程
		class loop_thread
		{
			using runable = std::shared_ptr<stdx::runable<void>>;
		public:
			//默认构造函数
			loop_thread()
				:task_queue(std::make_shared<std::queue<runable>>())
				,status(stdx::async::thread_status::creating)
				,barrier(std::make_shared<stdx::async::barrier>())
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
			loop_thread(const std::shared_ptr<std::queue<runable>> &task_queue_ptr,const stdx::async::barrier_ptr &barrier_ptr)
				:task_queue(task_queue_ptr)
				, status(stdx::async::thread_status::creating)
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
					status = stdx::async::thread_status::free;
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
					catch (...)
					{
						return;
					}
					if(!i)
					{
						if (!keep_alive)
						{
							status = stdx::async::thread_status::close;
							return;
						}
						status = stdx::async::thread_status::working;
						runable task = task_queue->front();
						task_queue->pop();
						task->run();
					}	
				}
				status = stdx::async::thread_status::close;
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
			const int &get_status() const
			{
				return status;
			}

			const bool &is_free() const
			{
				return status == stdx::async::thread_status::free;
			}

			const bool &is_alive() const
			{
				return keep_alive;
			}

		private:
			std::shared_ptr<std::queue<runable>> task_queue;
			int status;
			stdx::async::barrier_ptr barrier;
			std::atomic_bool keep_alive;
			std::shared_ptr<std::thread> thread_ptr;
			std::thread::id id;
		};

		using loop_thread_ptr = std::shared_ptr<stdx::async::loop_thread>;

		//线程池
		class thread_pool
		{
			using runable_ptr = std::shared_ptr<stdx::runable<void>>;
		public:
			thread_pool(const int& initializ_thread_count = std::thread::hardware_concurrency())
				:free_count(0)
				,task_queue(std::make_shared<std::queue<runable_ptr>>())
				,barrier(std::move(std::make_shared<stdx::async::barrier>()))
			{

			}
			~thread_pool()
			{
				std::for_each(std::begin(threads), std::end(threads), [](loop_thread_ptr ptr) 
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
				runable_ptr c = stdx::make_action<void>(task, args...);
				auto f = std::bind([this](runable_ptr call)
				{
					deduct_free();
					if (call)
					{
						call->run();
					}
					add_free();
				},c);
				task_queue->push(stdx::make_action(f));
				barrier->pass();
			}


			void add_thread()
			{
				free_count += 1;
				auto ptr = std::make_shared<stdx::async::loop_thread>(task_queue,barrier);
				threads.push_back(ptr);
			}
			
			const static std::shared_ptr<stdx::async::thread_pool> get()
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
			stdx::async::barrier_ptr barrier;
			std::vector<stdx::async::loop_thread_ptr> threads;
			static std::shared_ptr<stdx::async::thread_pool> default;
		};
		std::shared_ptr<stdx::async::thread_pool> stdx::async::thread_pool::default = std::make_shared<stdx::async::thread_pool>();
	}
}
#define THREAD_POOL std::shared_ptr<stdx::async::thread_pool>
#define GET_THREAD_POOL() stdx::async::thread_pool::get()
