#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <stdx/async/task.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define _ThrowWinError auto _ERROR_CODE = GetLastError(); \
						if(_ERROR_CODE != 997)\
						{\
								std::string _ERROR_STRING("windows system error:"); \
								_ERROR_STRING.append(std::to_string(_ERROR_CODE)); \
								throw std::runtime_error(_ERROR_STRING.c_str()); \
						}\

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
			return *this;
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
	class file_io_info
	{
	public:
		file_io_info(size_t size=4096)
			:m_offset(0)
			,m_buffer(size)
		{}
		file_io_info(const file_io_info &other)
			:m_buffer(other.m_buffer)
			,m_offset(other.m_offset)
		{}
		~file_io_info() = default;
		file_io_info &operator=(const file_io_info &other)
		{
			m_buffer = other.m_buffer;
			m_offset = other.m_offset;
			return *this;
		}
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
		void set_buffer(const buffer &other)
		{
			m_buffer = other;
		}
	private:
		buffer m_buffer;
		unsigned int m_offset;
	};
	template<typename _Parm>
	class io_context
	{
	public:
		OVERLAPPED m_ol;
		template<typename ..._Args>
		io_context(HANDLE file_handle,const _Args &...args)
			:m_parm(std::make_shared<_Parm>(args...))
			, m_file_handle(file_handle)
			, m_promise(std::make_shared<std::promise<_Parm>>())
		{
			std::memset(&m_ol, 0, sizeof(m_ol));
		}
		io_context(const std::shared_ptr<_Parm> &parm,HANDLE file_handle)
			:m_parm(parm)
			,m_file_handle(file_handle)
			, m_promise(std::make_shared<std::promise<_Parm>>())
		{
			std::memset(&m_ol, 0, sizeof(m_ol));
		}
		io_context(const io_context<_Parm> &other)
			:m_ol(other.m_ol)
			,m_parm(other.m_parm)
			,m_file_handle(other.m_file_handle)
			, m_promise(other.m_promise)
		{
		}
		io_context(io_context<_Parm> &&other)
			:m_ol(other.m_ol)
			,m_parm(std::move(other.m_parm))
			,m_file_handle(std::move(other.m_file_handle))
			, m_promise(std::move(other.m_promise))

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
		void callback()
		{
			m_promise->set_value(*m_parm);
		}
		std::shared_ptr<std::shared_future<_Parm>> get_future()
		{
			return std::make_shared<std::shared_future<_Parm>>(m_promise->get_future());
		}
	private:
		std::shared_ptr<_Parm> m_parm;
		HANDLE m_file_handle;
		std::shared_ptr<std::promise<_Parm>> m_promise;
	};


	using file_io_context = io_context<file_io_info>;


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
			bool r = GetQueuedCompletionStatus(m_iocp, &size,&key,&ol, INFINITE);
			if (!r)
			{
				//处理错误
				_ThrowWinError
			}
			return *CONTAINING_RECORD(ol, stdx::io_context<_IOContext>, m_ol);
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

	struct file_access_type
	{
		enum
		{
			execute = FILE_GENERIC_EXECUTE,
			read = FILE_GENERIC_READ,
			write = FILE_GENERIC_WRITE,
			all = GENERIC_ALL
		};
	};

	struct file_shared_model
	{
		enum
		{
			unique = 0UL,
			shared_read = FILE_SHARE_READ,
			shared_write = FILE_SHARE_WRITE,
			shared_delete = FILE_SHARE_DELETE
		};
	};
	struct open_type
	{
		enum
		{
			open = OPEN_EXISTING,
			create = CREATE_ALWAYS,
			new_file = CREATE_NEW,
			create_open = OPEN_ALWAYS
		};
	};
	class _FileIOService
	{
	public:
		using iocp_t = stdx::iocp<file_io_info>;
		_FileIOService()
			:m_iocp()
		{}
		_FileIOService(const iocp_t &iocp)
			:m_iocp(iocp)
		{}
		~_FileIOService() = default;
		HANDLE create_file(const std::string &path,DWORD access_type,DWORD open_type,DWORD shared_model)
		{
			HANDLE file = CreateFile(path.c_str(), access_type, shared_model, 0,open_type,FILE_FLAG_OVERLAPPED,0);
			if (file == INVALID_HANDLE_VALUE)
			{
				_ThrowWinError
			}
			m_iocp.bind(file);
			return file;
		}
		std::shared_ptr<std::shared_future<file_io_info>> read_file(HANDLE file, unsigned int buffer_size = 4096U, unsigned int offset = 0)
		{
			file_io_context context(file, buffer_size);
			context.get().set_offset(offset);
			auto future = context.get_future();
			if (!ReadFile(file, context.get().get_buffer(), context.get().get_buffer().size(), NULL, &context.m_ol))
			{
				_ThrowWinError
			}
			stdx::threadpool::run([](iocp_t &iocp) 
			{
				auto context = iocp.get();
				context.callback();
			}, m_iocp);
			return future;
		}
	private:
		iocp_t m_iocp;
	};

	class file_io_service
	{
		using impl_t = std::shared_ptr<_FileIOService>;
		using iocp_t = typename _FileIOService::iocp_t;
	public:
		file_io_service()
			:m_impl(std::make_shared<_FileIOService>())
		{}
		file_io_service(const iocp_t &iocp)
			:m_impl(std::make_shared<_FileIOService>(iocp))
		{}
		HANDLE create_file(const std::string &path, DWORD access_type, DWORD open_type, DWORD shared_model)
		{
			return m_impl->create_file(path, access_type, open_type, shared_model);
		}
		std::shared_ptr<std::shared_future<file_io_info>> read_file(HANDLE file, unsigned int buffer_size=4096U, unsigned int offset=0)
		{
			return m_impl->read_file(file, buffer_size, offset);
		}
	private:
		impl_t m_impl;
	};

	class async_fstream
	{
		using io_service_t = file_io_service;
	public:
		async_fstream(const io_service_t &io_service,const std::string &path,DWORD access_type,DWORD open_type,DWORD shared_model)
			:m_io_service(io_service)
			,m_file(m_io_service.create_file(path,access_type,open_type,shared_model))
		{}
		stdx::task<file_io_info> read(unsigned int buffer_size=4096U,unsigned int offset=0)
		{
			return stdx::async([](std::shared_ptr<std::shared_future<file_io_info>> future) 
			{
				return future->get();
			},m_io_service.read_file(m_file, buffer_size, offset));
		}
	private:
		io_service_t m_io_service;
		HANDLE m_file;
	};
}
#endif