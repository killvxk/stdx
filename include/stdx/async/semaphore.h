#pragma once
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace stdx
{
	//屏障
	class _Semaphore
	{
	public:
		//默认构造函数
		_Semaphore()
			:mutex(std::make_shared<std::mutex>())
			, notify_count(0)
			, cv(std::make_shared<std::condition_variable>())
		{}
		//析构函数
		~_Semaphore() = default;

		void wait()
		{
			std::unique_lock<std::mutex> lock(*mutex);
			auto &n = notify_count;
			cv->wait(lock, [&n]() { return n==0; });
			notify_count -= 1;
		}

		void notify()
		{
			notify_count += 1;
			this->cv->notify_one();
		}

		void notify_all()
		{
			cv->notify_all();
			notify_count = 0;
		}

		template<class _Rep,class _Period>
			bool wait_for(const std::chrono::duration<_Rep, _Period> &time)
		{
			std::unique_lock<std::mutex> lock(*mutex);
			auto &n = notify_count;
			if (cv->wait_for(lock, time, [&n]() { return (int)n; }))
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
	class semaphore
	{
		using impl_t = std::shared_ptr<stdx::_Semaphore>;
	public:
		semaphore() 
			:m_impl(std::make_shared<_Semaphore>())
		{}
		semaphore(const semaphore &other) 
			:m_impl(other.m_impl)
		{}
		semaphore(semaphore && other)
			: m_impl(std::move(other.m_impl))
		{}
		~semaphore() = default;
		semaphore &operator=(const semaphore &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		void wait()
		{
			return m_impl->wait();
		}

		void notify()
		{
			return m_impl->notify();
		}

		void notify_all()
		{
			return m_impl->notify_all();
		}

		template<class _Rep,class _Period>
			bool wait_for(const std::chrono::duration<_Rep, _Period> &time)
		{
			return m_impl->wait_for<_Rep,_Period>(time);
		}

		bool operator==(const semaphore &other) const
		{
			return m_impl == other.m_impl;
		}
	private:
		impl_t m_impl;
	};
}
