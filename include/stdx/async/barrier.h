#pragma once
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace stdx
{
	//屏障
	class _Barrier
	{
	public:
		//默认构造函数
		_Barrier()
			:mutex(std::make_shared<std::mutex>())
			, notify_count(0)
			, cv(std::make_shared<std::condition_variable>())
		{}
		//析构函数
		~_Barrier() = default;

		//等待通过
		//等待通过
		void wait()
		{
			std::unique_lock<std::mutex> wait(*mutex);
			auto &n = notify_count;
			cv->wait(wait, [&n]() { return (int)n; });
			notify_count -= 1;
		}

		//通过
		//通过
		void pass()
		{
			notify_count += 1;
			this->cv->notify_one();
		}

		template<class _Rep,class _Period>
			bool wait_for(const std::chrono::duration<_Rep, _Period> &time)
		{
			std::unique_lock<std::mutex> wait(*mutex);
			auto &n = notify_count;
			if (cv->wait_for(wait, time, [&n]() { return (int)n; }))
			{
				notify_count -= 1;
				return true;
			}
			else
			{
				return false;
			}
		}
	private:
		std::shared_ptr<std::mutex> mutex;
		std::atomic_int notify_count;
		std::shared_ptr<std::condition_variable> cv;
	};
	class barrier
	{
		using impl_t = std::shared_ptr<stdx::_Barrier>;
	public:
		barrier() 
			:m_impl(std::make_shared<_Barrier>())
		{}
		barrier(const barrier &other) 
			:m_impl(other.m_impl)
		{}
		barrier(barrier && other)
			: m_impl(std::move(other.m_impl))
		{}
		~barrier() = default;
		barrier &operator=(const barrier &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		void wait()
		{
			return m_impl->wait();
		}
		void pass()
		{
			return m_impl->pass();
		}
		template<class _Rep,class _Period>
			bool wait_for(const std::chrono::duration<_Rep, _Period> &time)
		{
			return m_impl->wait_for<_Rep,_Period>(time);
		}
	private:
		impl_t m_impl;
	};
}
