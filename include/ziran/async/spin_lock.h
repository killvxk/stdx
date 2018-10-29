#pragma once
#include <atomic>
#include <chrono>
#include <thread>

namespace ziran
{
	namespace async
	{
		//自旋锁模板
		template<
			//休眠时长(毫秒)
			int sleep_milliseconds
		>
		class spin_lock
		{
		public:
			//默认构造函数
			spin_lock()
				:is_using(false)
			{
			}
			//析构函数
			~spin_lock() = default;
			//复制构造函数
			spin_lock(const spin_lock &other)
				:is_using(other.is_using)
			{
			}
			//移动构造函数
			spin_lock(spin_lock &&other)
				:is_using(std::move(other.is_using))
			{
			}
			//进入锁
			void enter()
			{
				//如果被占用
				while (is_using)
				{
					//自旋并休眠
					std::this_thread::sleep_for(std::chrono::milliseconds(sleep_milliseconds));
				}
				//获得锁
				//将状态设置为被占用
				is_using = true;
			}
			//退出锁
			void exit()
			{
				//将状态设置为不被占用
				is_using = false;
			}
		private:
			//锁的状态
			std::atomic_bool is_using;
		};
		//自旋锁 不休眠
		template<>
		class spin_lock<0>
		{
		public:
			//默认构造函数
			spin_lock()
				:is_using(false)
			{
			}
			~spin_lock() = default;
			//进入锁
			void enter()
			{
				//如果被占用
				while (is_using)
				{
				}
				//获得锁
				//将状态设置为被占用
				is_using = true;
			}
			//退出锁
			void exit()
			{
				//将状态设置为不被占用
				is_using = false;
			}
		private:
			//锁的状态
			std::atomic_bool is_using;
		};
		//自旋锁 让步
		template<>
		class spin_lock<-1>
		{
		public:
			//默认构造函数
			spin_lock()
				:is_using(false)
			{
			}
			~spin_lock() = default;
			//进入锁
			void enter()
			{
				//如果被占用
				while (is_using)
				{
					std::this_thread::yield();
				}
				//获得锁
				//将状态设置为被占用
				is_using = true;
			}
			//退出锁
			void exit()
			{
				//将状态设置为不被占用
				is_using = false;
			}
		private:
			//锁的状态
			std::atomic_bool is_using;
		};
	}
}
