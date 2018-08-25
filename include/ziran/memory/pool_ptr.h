#pragma once
#include <atomic>
namespace ziran
{
	namespace memory
	{
		//对象池前置声明
		class object_pool;
		//池指针模板
		template<
			//类型
			typename T
			//计算器类型
			,typename TCount = std::atomic_int
			//对象池类型
			,typename TPool = ziran::memory::object_pool
		>
		class pool_ptr
		{
		public:
			//构造函数
			explicit pool_ptr(T *ptr,TPool &pool)
				:ptr(ptr)
				,pool(pool)
			{
				count = new TCount;
				*count = 1;
			}
			//拷贝构造函数
			pool_ptr(const pool_ptr<T, TCount, TPool> &other)
				:ptr(other.ptr)
				,count(other.count)
				,pool(other.pool)
			{
				*count += 1;
			}
			//move构造函数
			pool_ptr(pool_ptr<T, TCount, TPool> &&other)
				:ptr(std::move(other.ptr))
				,count(std::move(other.count))
				,pool(std::move(other.pool))
			{
			}
			//拷贝赋值函数
			pool_ptr<T, TCount, TPool> &operator=(const pool_ptr<T, TCount, TPool> &other)
			{
				ptr = other.ptr;
				count = other.count;
				pool = other.pool;
			}
			//重载*操作符
			T &operator*() const
			{
				return *ptr;
			}
			//重载->操作符
			T *operator->() const
			{
				return ptr;
			}
			//重载到bool的转换
			operator bool() const
			{
				return ptr;
			}
			//析构函数
			~pool_ptr()
			{
				//如果计算器被释放
				if (!count)
				{
					//直接返回
					return;
				}
				try
				{
					//引用计算减1
					*count -= 1;
					//如果计算器等于0
					if (*count == 0)
					{
						//销毁对象
						pool.destroy_object(ptr);
						//销毁计算器
						delete count;
						//将计算器悬空
						count = nullptr;
					}
				}
				catch (const std::exception&)
				{
					return;
				}
			}
		private:
			//类型指针
			T *ptr;
			//计数器指针
			TCount *count;
			//对象池引用
			TPool &pool;
		};
	}
}