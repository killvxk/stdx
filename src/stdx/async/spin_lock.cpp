#include <stdx/async/spin_lock.h>

stdx::_SpinLock::_SpinLock()
	:m_locked(false)
{}

void stdx::_SpinLock::lock()
{
	bool exp = false;
	while (!m_locked.compare_exchange_strong(exp, true))
	{
		exp = false;
	}
}

void stdx::_SpinLock::unlock() noexcept
{
	m_locked.store(false);
}