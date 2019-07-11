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
			bool exp = false;
			while (!m_locked.compare_exchange_strong(exp,true))
			{
				exp = false;
			}
		}

		void unlock()
		{
			m_locked.store(false);
		}
	private:
		volatile std::atomic_bool m_locked;
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
