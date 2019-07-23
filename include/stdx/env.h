#pragma once
//类库遵循以下约定
//所有的Class(除实现Class外,例如:_XxYy)都是引用类型
//所有的Struct(除另外说明外)都是值类型

//在没有定义任何平台符号的情况下
//依照编译器判断平台

#ifndef WIN32
#ifndef LINUX

#ifdef __WIN32
#ifndef WIN32
#define WIN32
#endif // !WIN32
#endif // __WIN32

#ifdef __linux__
#ifndef LINUX
#define LINUX
#endif // !LINUX
#endif // __linux__

#endif // !LINUX
#endif // !WIN32

#define int32 int
#define byte unsigned char
#define uint32 unsigned int
#define int16 short
#define uint16 unsigned short

#ifdef WIN32
#define uint unsigned int
#define int64 __int64
#define uint64 unsigned __int64
#endif

#ifdef LINUX
#define int64 long long
#define uint64 unsigned long long
#endif

#define get_byte(x,ptr) *((byte*)ptr+(x))
#define delete_copy(type) type(const type &)=delete
#define delete_move(type) type(type&&)=delete
#define cstring const char*
#define empty_cstring ""
#define CRLF "\r\n"

#ifdef WIN32
#define next_line CRLF
#endif

#ifdef LINUX
#define next_line "\n"
#endif

namespace stdx
{
	union int64_union
	{
		struct 
		{
			uint32 low;
			int32 height;
		};
		int64 value;
	};

	union int32_union
	{
		struct
		{
			uint16 low;
			int16 height;
		};
		int32 value;
	};

	union uint64_union
	{
		struct
		{
			uint32 low;
			uint32 height;
		};
		uint64 value;
	};

	union uint32_union
	{
		struct
		{
			uint16 low;
			uint16 height;
		};
		uint32 value;
	};
}

#include <stdexcept>
#include <system_error>
#include <time.h>
namespace stdx
{
	struct stop_watcher
	{
	public:
		stop_watcher()
			:m_begin(0)
			, m_end(0)
			, m_time(0)
		{}
		~stop_watcher() = default;
		stop_watcher(const stop_watcher &other)
			:m_begin(other.m_begin)
			, m_end(other.m_end)
			, m_time(other.m_time)
		{}
		void begin()
		{
			m_begin = clock();
		}
		void end()
		{
			m_end = clock();
		}
		clock_t time()
		{
			if (!m_time)
			{
				m_time = m_end - m_begin;
			}
			return m_time;
		}
		stop_watcher &operator=(const stop_watcher &other)
		{
			m_begin = other.m_begin;
			m_end = other.m_end;
			m_time = other.m_time;
			return *this;
		}
		void clean()
		{
			m_begin = 0;
			m_end = 0;
			m_time = 0;
		}
	private:
		clock_t m_begin;
		clock_t m_end;
		clock_t m_time;
	};
}

//#include <new>
//#include <memory>
//namespace stdx
//{
//	using allocator = std::allocator<char>;
//	struct mem_alloc
//	{
//		static void *alloc(const size_t &size)
//		{
//			void *ptr = m_allocator.allocate(size);
//			if (!ptr)
//			{
//				throw std::bad_alloc();
//			}
//			return ptr;
//		}
//		static void dealloc(void *ptr,const size_t &size)
//		{
//			m_allocator.deallocate((char*)ptr,size);
//		}
//		static void *calloc(const size_t &size, const size_t &count)
//		{
//			void *ptr = m_allocator.allocate(size*count);
//			if (!ptr)
//			{
//				throw std::bad_alloc();
//			}
//			return ptr;
//		}
//		static void cdealloc(void *ptr, const size_t &size, const size_t &count)
//		{
//			m_allocator.deallocate((char*)ptr, size*count);
//		}
//		static void *realloc(void *ptr,const size_t &size,const size_t &into_size)
//		{
//			if (size == into_size)
//			{
//				return ptr;
//			}
//			if (size < into_size)
//			{
//				void *new_ptr = m_allocator.allocate(into_size);
//				if (!new_ptr)
//				{
//					throw std::bad_alloc();
//				}
//				::memmove(new_ptr, ptr, size);
//				m_allocator.deallocate((char*)ptr, size);
//				return new_ptr;
//			}
//			else
//			{
//				size_t oversize = size - into_size;
//				m_allocator.deallocate(((char*)(ptr)+(into_size)), oversize);
//				return ptr;
//			}
//		}
//		static allocator m_allocator;
//	};
//
//	void *malloc(const size_t &size)
//	{
//		return stdx::mem_alloc::alloc(size);
//	}
//
//	template<typename _Type>
//	_Type *malloc()
//	{
//		return (_Type*)stdx::malloc(sizeof(_Type));
//	}
//
//	void free(void *ptr,const size_t &size)
//	{
//		return stdx::mem_alloc::dealloc(ptr, size);
//	}
//
//	template<typename _Type>
//	void free(_Type *ptr)
//	{
//		stdx::free(ptr,sizeof(_Type));
//	}
//
//	void *calloc(const size_t &size,const size_t &count)
//	{
//		return stdx::mem_alloc::calloc(size, count);
//	}
//
//	template<typename _Type>
//	_Type *calloc(const size_t &count)
//	{
//		return (_Type*)stdx::mem_alloc::calloc(sizeof(_Type),count);
//	}
//
//	void cfree(void *ptr,const size_t &size,const size_t &count)
//	{
//		stdx::mem_alloc::cdealloc(ptr, size, count);
//	}
//
//	template<typename _Type>
//	void cfree(_Type *ptr, const size_t &count)
//	{
//		stdx::mem_alloc::cdealloc(ptr, sizeof(_Type),count);
//	}
//
//	void *realloc(void *ptr, const size_t &size, const size_t &into_size)
//	{
//		return stdx::mem_alloc::realloc(ptr, size, into_size);
//	}
//
//	template<typename _Type>
//	_Type *realloc(_Type *ptr, const size_t &into_size)
//	{
//		return (_Type*)stdx::mem_alloc::realloc(ptr, sizeof(_Type), into_size);
//	}
//}