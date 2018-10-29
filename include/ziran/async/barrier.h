#pragma once
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace ziran
{
	namespace async
	{
		//屏障
		class barrier
		{
		public:
			//默认构造函数
			barrier()
				:mutex(std::make_shared<std::mutex>())
				,notify_count(0)
				,cv(std::make_shared<std::condition_variable>())
			{}

			//构造函数
			barrier
			(
				//初始可通过次数
				const int &pass_count
			)
				:mutex(std::make_shared<std::mutex>())
				,notify_count(pass_count)
				,cv(std::make_shared<std::condition_variable>())
			{}
			//析构函数
			~barrier()
			{
			}

			//等待通过
			void wait()
			{
				std::unique_lock<std::mutex> lock(*mutex);
				auto &n = notify_count;
				cv->wait(lock, [&n]() { return (int)n; });
				notify_count -= 1;
			}
			//通过
			void pass()
			{
				cv->notify_one();
				notify_count += 1;
			}
				
		private:
			std::shared_ptr<std::mutex> mutex;
			std::atomic_int notify_count;
			std::shared_ptr<std::condition_variable> cv;
		};
		using shared_barrier = std::shared_ptr<ziran::async::barrier>;
		using unique_barrier = std::unique_ptr<ziran::async::barrier>;
	}
}