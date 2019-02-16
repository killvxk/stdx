#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <stdx/async/task.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//定义抛出Windows错误宏
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
	//自动释放缓存区实现
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
		explicit _Buffer(size_t size, char* data)
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

	//自动释放缓存区
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

	//文件IO上下文
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
		std::function<void(file_io_context*,std::exception_ptr)> *callback;
	};

	//文件读取完成事件
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

	//文件写入完成事件
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

	//IOCP封装
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
			if (CreateIoCompletionPort(file_handle, m_iocp, (ULONG_PTR)file_handle, 0) == NULL)
			{
				_ThrowWinError
			}
		}

		template<typename _HandleType>
		void bind(const _HandleType &file_handle)
		{
			if (CreateIoCompletionPort((HANDLE)file_handle, m_iocp, file_handle, 0) == NULL)
			{
				_ThrowWinError
			}
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

	//IOCP引用封装
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

	//文件访问类型
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

	//文件共享类型
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

	//文件打开类型
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

	//文件IO服务实现
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
		void read_file(HANDLE file,const size_t &size,const size_t &offset,std::function<void(file_read_event,std::exception_ptr)> &&callback)
		{
			file_io_context *context = new file_io_context;
			context->eof = false;
			context->file = file;
			context->offset = offset;
			context->buffer = (char*)std::calloc(size,sizeof(char));
			context->size = size;
			std::function<void(file_io_context*, std::exception_ptr)> *call = new std::function<void(file_io_context*, std::exception_ptr)>;
			*call = [callback,size](file_io_context *context_ptr,std::exception_ptr error)
			{
				if (error)
				{
					callback(file_read_event(), error);
					delete context_ptr;
					return;
				}
				if (context_ptr->size < size)
				{
					context_ptr->eof = true;
				}
				file_read_event context(context_ptr);
				delete context_ptr;
				callback(context,nullptr);
			};
			context->callback = call;
			if (!ReadFile(file,context->buffer,size,&(context->size), &(context->m_ol)))
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
			stdx::threadpool::run([](iocp_t &iocp)
			{
				auto *context_ptr = iocp.get();
				std::exception_ptr error(nullptr);
				try
				{
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
				}
				catch (const std::exception&)
				{
					error = std::current_exception();
				}
				auto *call = context_ptr->callback;
				(*call)(context_ptr,error);
				delete call;
			}, m_iocp);
			return;
		}
		void write_file(HANDLE file,const char *buffer,const size_t &size,std::function<void(file_write_event,std::exception_ptr)> &&callback)
		{
			file_io_context *context_ptr = new file_io_context;
			context_ptr->size = 0;
			context_ptr->offset = 0;
			auto *call = new std::function<void(file_io_context*, std::exception_ptr)>;
			*call = [callback](file_io_context *context_ptr, std::exception_ptr error)
			{
				if (error)
				{
					delete context_ptr;
					callback(file_write_event(), error);
					return;
				}
				file_write_event context(context_ptr);
				delete context_ptr;
				callback(context,nullptr);
			};
			context_ptr->callback = call;
			if (!WriteFile(file, buffer, size, &(context_ptr->size), &(context_ptr->m_ol)))
			{
				_ThrowWinError
			}
			stdx::threadpool::run([](iocp_t &iocp)
			{
				auto *context_ptr = iocp.get();
				std::exception_ptr error(nullptr);
				try
				{
					if (!GetOverlappedResult(context_ptr->file, &(context_ptr->m_ol), &(context_ptr->size), false))
					{
						_ThrowWinError
					}
				}
				catch (const std::exception&)
				{
					error = std::current_exception();
				}
				auto *call = context_ptr->callback;
				(*call)(context_ptr,error);
				delete call;
			},m_iocp);
		}
	private:
		iocp_t m_iocp;
	};

	//文件IO服务
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
		void read_file(HANDLE file,const size_t &size, const size_t &offset, std::function<void(file_read_event,std::exception_ptr)> &&callback)
		{
			return m_impl->read_file(file, size, offset,std::move(callback));
		}
		void write_file(HANDLE file, const char *buffer, const size_t &size, std::function<void(file_write_event,std::exception_ptr)> &&callback)
		{
			return m_impl->write_file(file, buffer, size, std::move(callback));
		}
		
	private:
		impl_t m_impl;
	};

	//异步文件流实现
	class _AsyncFileStream
	{
		using io_service_t = file_io_service;
	public:
		_AsyncFileStream(const io_service_t &io_service,const std::string &path,DWORD access_type,DWORD open_type,DWORD shared_model)
			:m_io_service(io_service)
			,m_file(m_io_service.create_file(path,access_type,open_type,shared_model))
		{}
		~_AsyncFileStream()
		{
			CloseHandle(m_file);
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
			m_io_service.read_file(m_file, size, offset, [promise,task](file_read_event context,std::exception_ptr error) mutable
			{
				if (error)
				{
					promise->set_exception(error);
					return;
				}
				promise->set_value(context);
				task.run();
			});
			return task;
		}
		stdx::task<file_write_event> write(const char* buffer,const size_t &size)
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
			m_io_service.write_file(m_file, buffer, size, [promise,task](file_write_event context,std::exception_ptr error) mutable 
			{
				if (error)
				{
					promise->set_exception(error);
				}
				promise->set_value(context);
				task.run();
			});
			return task;
		}
	private:
		io_service_t m_io_service;
		HANDLE m_file;
	};
	class async_file_stream
	{
		using impl_t = std::shared_ptr<_AsyncFileStream>;
		using io_service_t = file_io_service;
	public:
		explicit async_file_stream(const io_service_t &io_service, const std::string &path, DWORD access_type, DWORD open_type, DWORD shared_model)
			:m_impl(std::make_shared<_AsyncFileStream>(io_service,path,access_type,open_type,shared_model))
		{}

		async_file_stream(const async_file_stream &other)
			:m_impl(other.m_impl)
		{}

		async_file_stream(async_file_stream &&other)
			:m_impl(std::move(other.m_impl))
		{}

		~async_file_stream() = default;

		async_file_stream &operator=(const async_file_stream &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		stdx::task<file_read_event> read(size_t size, size_t offset)
		{
			return m_impl->read(size, offset);
		}

		stdx::task<file_write_event> write(const char* buffer, size_t size)
		{
			return m_impl->write(buffer, size);
		}
		stdx::task<file_write_event> write(const std::string &str)
		{
			return m_impl->write(str.c_str(), str.size());
		}
	private:
		impl_t m_impl;
	};
}
#endif