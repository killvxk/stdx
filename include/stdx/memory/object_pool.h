#pragma once
#include <memory>
#include <ziran/memory/pool_ptr.h>
namespace ziran
{
	namespace memory
	{
		//对象池
		class object_pool
		{
		public:
			//默认构造函数
			object_pool() = default;
			//析构函数
			~object_pool() = default;
			//创建对象
			template<
				//类型
				typename T
				//参数类型
				, typename ...TArgs
			>
			T *make_object(TArgs &&...args)
			{
				//获取对象大小
				auto size = sizeof(T);
				//分配内存
				auto *byte_ptr = allocator.allocate(size);
				//执行构造函数
				T *ptr = new(byte_ptr) T(args...);
				//返回指针
				return ptr;
			}
			//创建池指针
			template<
				//类型
				typename T
				//池指针类型
				,typename TPtr=ziran::memory::pool_ptr<T>
			>
			TPtr make_pool_ptr(T *ptr)
			{
				return TPtr(ptr, *this);
			}
			//创建池指针
			template<
				//类型
				typename T
				//参数类型
				typename ...TArgs
			>
			ziran::memory::pool_ptr<T> make_object_ptr(TArgs &&...args)
			{
				return make_pool_ptr<T>(make_object<T>(args...));
			}

			//销毁对象
			template <typename T>
			void destroy_object(T* ptr)
			{
				//获取大小
				auto size = sizeof(T);
				//执行析构函数
				ptr->~T();
				//回收内存
				allocator.deallocate(reinterpret_cast<char*>(ptr), size);
			}
		private:
			std::allocator<char> allocator;
		};
	}
}