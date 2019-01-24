#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace stdx
{
	class _Buffer
	{
	public:
		_Buffer(size_t size=4096)
			:m_size(size)
			,m_data((char*)std::calloc(sizeof(char),m_size))
		{
			if (m_data == nullptr)
			{
				throw std::bad_alloc();
			}
		}
		~_Buffer()
		{
			free(m_data);
		}
		char &operator[](const size_t &i) const
		{
			if (i >= m_size)
			{
				throw std::out_of_range("out of range");
			}
			return *(m_data + i);
		}
		operator char*() const
		{
			return m_data;
		}
		void realloc(size_t size)
		{
			if (size == 0)
			{
				throw std::invalid_argument("invalid argument: 0");
			}
			if (size > m_size)
			{
				if (std::realloc(m_data, m_size)==nullptr)
				{
					throw std::bad_alloc();
				}
				m_size = size;
			}
		}
		const size_t &size() const
		{
			return m_size;
		}

		void copy_from(const _Buffer &other)
		{
			auto new_size = other.size();
			if (new_size > m_size)
			{
				realloc(new_size);
			}
			std::memcpy(m_data, other, new_size);
		}
	private:
		size_t m_size;
		char *m_data;
	};
	class buffer
	{
		using impl_t = std::shared_ptr<_Buffer>;
	public:
		buffer(size_t size=4096)
			:m_impl(std::make_shared<_Buffer>(size))
		{}
		buffer(const buffer &other)
			:m_impl(other.m_impl)
		{}
		buffer(buffer &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~buffer() = default;
		operator char*()
		{
			return *m_impl;
		}
		buffer &operator=(const buffer &other)
		{
			m_impl = other.m_impl;
		}
		char &operator[](const size_t &i)
		{
			return m_impl->operator[](i);
		}
		void realloc(size_t size)
		{
			m_impl->realloc(size);
		}
		const size_t &size() const
		{
			return m_impl->size();
		}
		void copy_from(const buffer &other)
		{
			m_impl->copy_from(*other.m_impl);
		}
	private:
		impl_t m_impl;
	};
	//class _BasicIOContext
	//{
	//public:
	//	OVERLAPPED m_ol;
	//	_BasicIOContext(HANDLE file_handle)
	//		:m_file_handle(file_handle)
	//		,m_buffer()
	//	{
	//	}
	//	_BasicIOContext(HANDLE file_handle, const buffer &buffer)
	//		:m_file_handle(file_handle)
	//		,m_buffer(buffer)
	//	{
	//	}
	//	virtual ~_BasicIOContext() = default;
	//	HANDLE get_file_handle() const
	//	{
	//		return m_file_handle;
	//	}
	//	buffer get_buffer() const
	//	{
	//		return m_buffer;
	//	}
	//	operator OVERLAPPED*()
	//	{
	//		return &m_ol;
	//	}
	//private:
	//	HANDLE m_file_handle;
	//	buffer m_buffer;
	//};
	class file_iodata
	{
	public:
		file_iodata(size_t size=4096)
			:m_offset(0)
			,m_buffer(size)
		{}
		~file_iodata() = default;
		unsigned int get_offset() const
		{
			return m_offset;
		}
		void set_offset(unsigned int offset)
		{
			m_offset = offset;
		}
		buffer get_buffer() const
		{
			return m_buffer;
		}
	private:
		buffer m_buffer;
		unsigned int m_offset;
	};

	//template<typename _IOContext>
	//class io_context
	//{};
	//template<>
	//class io_context<_FileIOContext>
	//{
	//	using impl_t = std::shared_ptr<_FileIOContext>;
	//public:
	//	io_context(HANDLE file_handle)
	//		:m_impl(std::make_shared< _FileIOContext>(file_handle))
	//	{}
	//	io_context(HANDLE file_handle, const buffer &buffer)
	//		:m_impl(std::make_shared< _FileIOContext>(file_handle,buffer))
	//	{}
	//	io_context(_FileIOContext *ptr)
	//		:m_impl(ptr)
	//	{}
	//	io_context(const io_context<_FileIOContext> &other)
	//		:m_impl(other.m_impl)
	//	{}
	//	io_context(io_context<_FileIOContext> &&other)
	//		:m_impl(std::move(other.m_impl))
	//	{}
	//	~io_context()=default;
	//	io_context<_FileIOContext> &operator=(const io_context<_FileIOContext> &other)
	//	{
	//		m_impl = other.m_impl;
	//	}
	//	buffer get_buffer() const
	//	{
	//		return m_impl->get_buffer();
	//	}
	//	HANDLE get_file_handle() const
	//	{
	//		return m_impl->get_file_handle();
	//	}
	//	unsigned int get_offset()
	//	{
	//		return m_impl->get_offset();
	//	}
	//	operator OVERLAPPED*()
	//	{
	//		return *m_impl;
	//	}
	//private:
	//	impl_t m_impl;
	//};

	template<typename _Parm>
	class io_context
	{
	public:
		OVERLAPPED m_ol;
		template<typename ..._Args>
		io_context(HANDLE file_handle,const _Args &...args)
			:m_parm(std::make_shared<_Parm>(args...))
			, m_file_handle(file_handle)
		{
			std::memset(&m_ol, 0, sizeof(m_ol));
		}
		io_context(const std::shared_ptr<_Parm> &parm,HANDLE file_handle)
			:m_parm(parm)
			,m_file_handle(file_handle)
		{
			std::memset(&m_ol, 0, sizeof(m_ol));
		}
		io_context(const io_context<_Parm> &other)
			:m_ol(other.m_ol)
			,m_parm(other.m_parm)
			,m_file_handle(other.m_file_handle)
		{
		}
		io_context(io_context<_Parm> &&other)
			:m_ol(other.m_ol)
			,m_parm(std::move(other.m_parm))
			,m_file_handle(std::move(other.m_file_handle))

		{
		}
		~io_context() = default;
		_Parm &get() const
		{
			return *m_parm;
		}
		io_context<_Parm> &operator=(const io_context<_Parm> &other)
		{
			m_ol = other.m_ol;
			m_parm = other.m_parm;
			m_file_handle = other.m_file_handle;
			return *this;
		}
		HANDLE get_file_handle() const
		{
			return m_file_handle;
		}
	private:
		std::shared_ptr<_Parm> m_parm;
		HANDLE m_file_handle;
	};


	using file_iocontext = io_context<file_iodata>;


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
			CreateIoCompletionPort(file_handle, m_iocp,(ULONG_PTR)file_handle, 0);
		}

		template<typename _HandleType>
		void bind(const _HandleType &file_handle)
		{
			CreateIoCompletionPort((HANDLE)file_handle, m_iocp, file_handle, 0);
		}

		stdx::io_context<_IOContext> get()
		{
			DWORD size = 0;
			OVERLAPPED *ol= nullptr;
			ULONG_PTR key = 0;
			bool r = GetQueuedCompletionStatus(m_iocp, &size,&key,(LPOVERLAPPED*)ol, INFINITE);
			if (!r)
			{
				//处理错误
				auto code = GetLastError();
				std::string str("windows system error:");
				str.append(std::to_string(code));
				throw std::runtime_error(str.c_str());
			}
			stdx::io_context<_IOContext> context (*CONTAINING_RECORD(ol, stdx::io_context<_IOContext>, m_ol));
			return context;
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
	using file_ioservice = stdx::iocp<file_iodata>;
}
#else

#endif