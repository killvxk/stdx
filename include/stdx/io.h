#pragma once
#include <memory>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace stdx
{
	template<typename _IOContext>
	class io_context
	{
	public:
		io_context(const std::shared_ptr<_IOContext> &parm, const DWORD &size, HANDLE file_handle)
			:m_parm(parm)
			, m_size(size)
			, m_file_handle(file_handle)
		{}
		~io_context() = default;
		const DWORD &size() const
		{
			return m_size;
		}
		_IOContext &get() const
		{
			return *m_parm;
		}
		_IOContext *operator->()
		{
			return get();
		}
		HANDLE get_file_handle() const
		{
			return m_file_handle;
		}
	private:
		std::shared_ptr<_IOContext> m_parm;
		DWORD m_size;
		HANDLE m_file_handle;
	};
	template<typename _IOContext>
	class _IOCP
	{
	public:
		_IOCP()
			:m_iocp(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
		{
		}
		~_IOCP()
		{
			if (m_iocp != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_iocp);
			}
		}
		void bind(const HANDLE &file_handle)
		{
			CreateIoCompletionPort(file_handle, m_iocp, file_handle, 0);
		}

		template<typename _HandleType>
		void bind(const _HandleType &file_handle)
		{
			CreateIoCompletionPort((HANDLE)file_handle, m_iocp, file_handle, 0);
		}

		stdx::io_context<_IOContext> get()
		{
			HANDLE* handle = NULL;
			DWORD size = NULL;
			OVERLAPPED *overlapped;
			bool r = GetQueuedCompletionStatus(m_iocp, &size, (LPDWORD)handle, overlapped, INFINITE);
			std::shared_ptr<_IOContext> context = std::make_shared<_IOContext>();
			*context = CONTAINING_RECORD(overlapped, _IOContext, m_ol);
			if (!r)
			{
				//处理错误
				GetLastError();
			}
			return stdx::io_context(context, size, handle);
		}

	private:
		HANDLE m_iocp;
	};
	template<typename _IOContext>
	class iocp
	{
		using impl_t = std::shared_ptr<stdx::_IOCP<_IOContext>>;
	public:
		iocp()
			:m_impl(std::make_shared<stdx::_IOCP<_IOContext>>())
		{}
		iocp(const iocp<_IOContext> &other)
			:m_impl(other.m_impl)
		{}
		iocp(iocp<_IOContext> &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~iocp() = default;
		iocp<_IOContext> &operator=(const iocp<_IOContext> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		stdx::io_context<_IOContext> get()
		{
			return m_impl->get();
		}
		void bind(const HANDLE &file_handle)
		{
			m_impl->bind(file_handle);
		}
		template<typename _HandleType>
		void bind(const _HandleType &file_handle)
		{
			m_impl->bind<_HandleType>(file_handle);
		}
	private:
		impl_t m_impl;
	};
	template<typename _IOContext>
	using io_service = stdx::iocp<_IOContext>;
}
#else

#endif