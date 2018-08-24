#pragma once
#include <memory>

namespace ziran
{
	namespace tools
	{
		class object_pool
		{
		public:
			object_pool() =default;
			~object_pool()=default;
			template<typename T,typename ...TArgs>
			T *make_object(TArgs &&...args)
			{
				auto size = sizeof(T);
				auto *byte_ptr = allocator.allocate(size);
				T *ptr = new(byte_ptr) T(args...);
				return ptr;
			}
			template <typename T>
			void destroy_object(T* ptr)
			{
				auto size = sizeof(T);
				ptr->~T();
				allocator.deallocate(reinterpret_cast<char*>(ptr), size);
			}
		private:
			std::allocator<char> allocator;
		};
	}
}