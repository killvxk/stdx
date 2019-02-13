#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <stdx/async/task.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define _ThrowWinError auto _ERROR_CODE = GetLastError(); \
						LPVOID _MSG;\
						if(_ERROR_CODE != ERROR_IO_PENDING) \
						{ \
							if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,_ERROR_CODE,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &_MSG,0,NULL))\
							{ \
								throw std::runtime_error((char*)_MSG);\
							}else \
							{ \
								std::string _ERROR_MSG("windows system error:");\
								_ERROR_MSG.append(std::to_string(_ERROR_CODE));\
								throw std::runtime_error(_ERROR_MSG.c_str()); \
							} \
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
		_Buffer(size_t size, char* data)
			:m_size(size)
			,m_data(data)
		{}
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
		buffer(size_t size, char* data)
			:m_impl(std::make_shared<_Buffer>(size,data))
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

	struct file_io_context
	{
		file_io_context()
		{
			std::memset(&m_ol, 0, sizeof(OVERLAPPED));
		}
		~file_io_context() = default;
		OVERLAPPED m_ol;
		HANDLE file;
		char *buffer;
		DWORD size;
		DWORD offset;
		bool eof;
	};

	struct file_read_event
	{
		file_read_event() = default;
		~file_read_event() = default;
		file_read_event(const file_read_event &other)
			:file(other.file)
			,buffer(other.buffer)
			,offset(other.offset)
			,eof(other.eof)
		{}
		file_read_event(file_read_event &&other)
			:file(std::move(other.file))
			,buffer(std::move(other.buffer))
			,offset(std::move(other.offset))
			,eof(std::move(other.eof))
		{}
		file_read_event &operator=(const file_read_event &other)
		{
			file = other.file;
			buffer = other.buffer;
			offset = other.offset;
			eof = other.eof;
			return *this;
		}
		file_read_event(file_io_context *ptr)
			:file(ptr->file)
			,buffer(ptr->size,ptr->buffer)
			,offset(ptr->offset)
			,eof(ptr->eof)
		{
		}
		HANDLE file;
		buffer buffer;
		size_t offset;
		bool eof;
	};

	struct file_write_event
	{
		file_write_event() = default;
		~file_write_event() = default;
		file_write_event(const file_write_event &other)
			:file(other.file)
			,size(other.size)
		{}
		file_write_event(file_write_event &&other)
			:file(std::move(other.file))
			,size(std::move(other.size))
		{}
		file_write_event &operator=(const file_write_event &other)
		{
			file = other.file;
			size = other.size;
			return *this;
		}
		file_write_event(file_io_context *ptr)
			:file(ptr->file)
			,size(ptr->size)
		{}
		HANDLE file;
		size_t size;
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
			CreateIoCompletionPort(file_handle, m_iocp,(ULONG_PTR)file_handle, 0);
		}

		template<typename _HandleType>
		void bind(const _HandleType &file_handle)
		{
			CreateIoCompletionPort((HANDLE)file_handle, m_iocp, file_handle, 0);
		}

		_IOContext *get()
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
			return CONTAINING_RECORD(ol,_IOContext, m_ol);
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
		_IOContext *get()
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
	struct file_open_type
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
		using iocp_t = stdx::iocp<file_io_context>;
		_FileIOService()
			:m_iocp()
		{}
		_FileIOService(const iocp_t &iocp)
			:m_iocp(iocp)
		{}
		~_FileIOService() = default;
		HANDLE create_file(const std::string &path,DWORD access_type,DWORD file_open_type,DWORD shared_model)
		{
			HANDLE file = CreateFile(path.c_str(), access_type, shared_model, 0,file_open_type,FILE_FLAG_OVERLAPPED,0);
			if (file == INVALID_HANDLE_VALUE)
			{
				_ThrowWinError
			}
			m_iocp.bind(file);
			return file;
		}
		void read_file(HANDLE file, size_t size, size_t offset,std::function<void(file_read_event)> &&callback)
		{
			file_io_context *context = new file_io_context;
			context->eof = false;
			context->file = file;
			context->offset = offset;
			context->buffer = (char*)std::calloc(size,sizeof(char));
			context->size = size;
			if (!ReadFile(file,context->buffer, context->size, NULL, &(context->m_ol)))
			{
				//处理错误
				DWORD code = GetLastError();
				if (code != ERROR_IO_PENDING)
				{
					if (code == ERROR_HANDLE_EOF)
					{
						context->eof = true;
					}
					else
					{
						LPVOID msg;
						if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL))\
						{ 
							throw std::runtime_error((char*)msg); 
						}
						else 
						{ 
							std::string _ERROR_MSG("windows system error:"); 
							_ERROR_MSG.append(std::to_string(code)); 
							throw std::runtime_error(_ERROR_MSG.c_str()); 
						} 
					}
				}
			}
			stdx::threadpool::run([](iocp_t &iocp, std::function<void(file_read_event)> &callback)
			{
				auto *context_ptr = iocp.get();
				if (!GetOverlappedResult(context_ptr->file, &(context_ptr->m_ol), &(context_ptr->size), false))
				{
					DWORD code = GetLastError();
					if (code != ERROR_IO_PENDING)
					{
						if (code == ERROR_HANDLE_EOF)
						{
							context_ptr->eof = true;
						}
						else
						{
							LPVOID msg;
							if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL))\
							{
								throw std::runtime_error((char*)msg);
							}
							else
							{
								std::string _ERROR_MSG("windows system error:");
								_ERROR_MSG.append(std::to_string(code));
								throw std::runtime_error(_ERROR_MSG.c_str());
							}
						}
					}
				}
				auto context = file_read_event(context_ptr);
				delete context_ptr;
				callback(context);
			}, m_iocp,callback);
			return;
		}
		void write_file(HANDLE file,const char *buffer,size_t size,std::function<void(file_write_event)> &&callback)
		{
			file_io_context *context_ptr = new file_io_context;
			context_ptr->buffer =(char*) buffer;
			context_ptr->size = 0;
			context_ptr->eof = false;
			context_ptr->offset = 0;
			if (!WriteFile(file, buffer, size, &(context_ptr->size), &(context_ptr->m_ol)))
			{
				_ThrowWinError
			}
			stdx::threadpool::run([](iocp_t &iocp, std::function<void(file_write_event)> &callback)
			{
				auto *context_ptr = iocp.get();
				if (!GetOverlappedResult(context_ptr->file, &(context_ptr->m_ol), &(context_ptr->size), false))
				{
					_ThrowWinError
				}
				auto context = file_write_event(context_ptr);
				delete context_ptr;
				callback(context);
			},m_iocp,callback);
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
		file_io_service(const file_io_service &other)
			:m_impl(other.m_impl)
		{}
		file_io_service(file_io_service &&other)
			:m_impl(std::move(other.m_impl))
		{}
		file_io_service &operator=(const file_io_service &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		operator bool() const
		{
			return (bool)m_impl;
		}
		HANDLE create_file(const std::string &path, DWORD access_type, DWORD file_open_type, DWORD shared_model)
		{
			return m_impl->create_file(path, access_type, file_open_type, shared_model);
		}
		void read_file(HANDLE file, size_t size, size_t offset, std::function<void(file_read_event)> &&callback)
		{
			return m_impl->read_file(file, size, offset,std::move(callback));
		}
		void write_file(HANDLE file,const char *buffer,size_t size,std::function<void(file_write_event)> &&callback)
		{
			return m_impl->write_file(file, buffer, size, std::move(callback));
		}
		
	private:
		impl_t m_impl;
	};

	class async_fstream
	{
		using io_service_t = file_io_service;
	public:
		async_fstream(const io_service_t &io_service,const std::string &path,DWORD access_type,DWORD file_open_type,DWORD shared_model)
			:m_io_service(io_service)
			,m_file(m_io_service.create_file(path,access_type,file_open_type,shared_model))
		{}
		async_fstream(const async_fstream &other)
			:m_io_service(other.m_io_service)
			,m_file(other.m_file)
		{}
		async_fstream(async_fstream &&other)
			:m_io_service(std::move(other.m_io_service))
			,m_file(std::move(other.m_file))
		{}
		async_fstream &operator=(const async_fstream &other)
		{
			m_file = other.m_file;
			m_io_service = other.m_io_service;
			return *this;
		}
		stdx::task<file_read_event> read(size_t size,size_t offset)
		{
			if (!m_io_service)
			{
				throw std::logic_error("this io service has been free");
			}
			std::shared_ptr<std::promise<file_read_event>> promise = std::make_shared<std::promise<file_read_event>>();
			stdx::task<file_read_event> task([promise]()
			{
				return promise->get_future().get();
			});
			m_io_service.read_file(m_file, size, offset, [promise,task](file_read_event context) mutable
			{
				promise->set_value(context);
				task.run();
			});
			return task;
		}
		stdx::task<file_write_event> write(const char* buffer, size_t size)
		{
			if (!m_io_service)
			{
				throw std::logic_error("this io service has been free");
			}
			std::shared_ptr<std::promise<file_write_event>> promise = std::make_shared<std::promise<file_write_event>>();
			stdx::task<file_write_event> task([promise]()
			{
				return promise->get_future().get();
			});
			m_io_service.write_file(m_file, buffer, size, [promise,task](file_write_event context) mutable 
			{
				promise->set_value(context);
				task.run();
			});
			return task;
		}
	private:
		io_service_t m_io_service;
		HANDLE m_file;
	};
}
#endif