#pragma once
#include <queue>
#include <stdx/env.h>
#include <memory>
#include <stdx/async/spin_lock.h>

namespace stdx
{
	template<typename _Type>
	class atomic_queue
	{
	public:
		atomic_queue()
			:m_impl(std::make_shared<std::queue<_Type>>())
			,m_lock()
		{}
		atomic_queue(const atomic_queue<_Type> &other)
			:m_impl(other.m_impl)
			,m_lock(other.m_lock)
		{}
		atomic_queue(atomic_queue<_Type> &&other)
			:m_impl(other.m_impl)
			,m_lock(other.m_lock)
		{}
		~atomic_queue()=default;
		atomic_queue &operator=(const atomic_queue &other)
		{
			m_impl = other.m_impl;
			m_lock = other.m_lock;
			return *this;
		}
		atomic_queue &operator=(atomic_queue &&other)
		{
			m_impl = other.m_impl;
			m_lock = other.m_lock;
			return *this;
		}
		const _Type &back() const
		{
			return m_impl->back();
		}
		const _Type &front() const
		{
			return m_impl->front();
		}

		bool empty()
		{
			m_lock.lock();
			bool status = m_impl->empty();
			m_lock.unlock();
			return status;
		}

		size_t size()
		{
			m_lock.lock();
			size_t status = m_impl->size();
			m_lock.unlock();
			return status;
		}
		_Type pop()
		{
			m_lock.lock();
			_Type value = std::move(m_impl->front());
			m_impl->pop();
			m_lock.unlock();
			return value;
		}
		void push(const _Type &value)
		{
			m_lock.lock();
			m_impl->push(value);
			m_lock.unlock();
		}

		void push(_Type &&value)
		{
			m_lock.lock();
			m_impl->push(value);
			m_lock.unlock();
		}

		template<typename ..._Args>
		void empalce(_Args &&...args)
		{
			m_lock.lock();
			m_impl->emplace(std::forward(args)...);
			m_lock.unlock();
		}

		void lock()
		{
			m_lock.lock();
		}

		void unlock()
		{
			m_lock.unlock();
		}
	private:
		std::shared_ptr<std::queue<_Type>> m_impl;
		stdx::spin_lock m_lock;
	};
}