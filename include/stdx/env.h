#pragma once
//在没有定义任何平台符号的情况下
//依照编译器判断平台

#ifndef WIN32
#ifndef LINUX

#ifdef __WIN32
#define WIN32
#endif // __WIN32

#ifdef __linux__
#define LINUX
#endif // __linux__

#endif // !LINUX
#endif // !WIN32

#define int32 int
#define byte unsigned char
#define uint32 unsigned int
#define uint unsigned int
#define int16 short
#define uint16 unsigned short
#ifdef WIN32
#define int64 __int64
#define uint64 unsigned __int64
#else
#define int64 long long
#define uint64 unsigned long long
#endif // WIN32

#define get_byte(x,ptr) *((byte*)ptr+(x))
#define delete_copy(type) type(const type &)=delete
#define delete_move(type) type(type&&)=delete
#define empty_cstr ""

#include <stdexcept>
#include <system_error>

#include <time.h>
namespace stdx
{
	class stop_watcher
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