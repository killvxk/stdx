#pragma once
#include <atomic>
#include <chrono>
#include <thread>
#include <memory>

namespace stdx
{
	class _SpinLock
	{
	public:
		_SpinLock()
			:m_locked(false)
		{}
		~_SpinLock() = default;

		void lock()
		{
			while (m_locked.exchange(true,std::memory_order_acquire))
			{
				std::this_thread::yield();
			}
		}

		void unlock()
		{
			m_locked.store(false,std::memory_order_release);
		}
	private:
		std::atomic_bool m_locked;
	};
	class spin_lock
	{
		using impl_t = std::shared_ptr<stdx::_SpinLock>;
	public:
		spin_lock()
			:m_impl(std::make_shared<stdx::_SpinLock>())
		{}
		spin_lock(const spin_lock &other)
			:m_impl(other.m_impl)
		{}
		spin_lock(spin_lock &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~spin_lock() = default;
		spin_lock operator=(const spin_lock &other)
		{
			m_impl = other.m_impl;
		}
		void lock()
		{
			m_impl->lock();
		}
		void unlock()
		{
			m_impl->unlock();
		}
	private:
		impl_t m_impl;
	};
}
