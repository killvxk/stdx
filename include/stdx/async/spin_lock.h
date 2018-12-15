#pragma once
#include <atomic>
#include <chrono>
#include <thread>

namespace stdx
{
	namespace async
	{
		class spin_lock
		{
		public:
			spin_lock()
				:m_locked(false)
			{}
			~spin_lock() = default;

			void lock()
			{
				while (std::atomic_exchange_explicit(&m_locked, true, std::memory_order_acquire))
				{
					std::this_thread::yield();
				}
			}

			void unlock()
			{
				std::atomic_store_explicit(&m_locked, false, std::memory_order_release);
			}
		private:
			std::atomic_bool m_locked;
		};
		using spin_lock_ptr = std::shared_ptr<spin_lock>;
	}
}
