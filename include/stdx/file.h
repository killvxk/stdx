#pragma once
#include <stdx/env.h>
#include <stdx/io.h>
#include <stdx/async/task.h>
#ifdef WIN32
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
								throw std::system_error(std::error_code(_ERROR_CODE,std::system_category()),_ERROR_MSG.c_str()); \
							} \
						}\

namespace stdx
{
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
		int64 offset;
		bool eof;
		std::function<void(file_io_context*, std::exception_ptr)> *callback;
	};
	//文件读取完成事件
	struct file_read_event
	{
		file_read_event() = default;
		~file_read_event() = default;
		file_read_event(const file_read_event &other)
			:file(other.file)
			, buffer(other.buffer)
			, offset(other.offset)
			, eof(other.eof)
		{}
		file_read_event(file_read_event &&other)
			:file(std::move(other.file))
			, buffer(std::move(other.buffer))
			, offset(std::move(other.offset))
			, eof(std::move(other.eof))
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
			, buffer(ptr->size, ptr->buffer)
			, offset(ptr->offset)
			, eof(ptr->eof)
		{
		}
		HANDLE file;
		stdx::buffer buffer;
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
			, size(other.size)
		{}
		file_write_event(file_write_event &&other)
			:file(std::move(other.file))
			, size(std::move(other.size))
		{}
		file_write_event &operator=(const file_write_event &other)
		{
			file = other.file;
			size = other.size;
			return *this;
		}
		file_write_event(file_io_context *ptr)
			:file(ptr->file)
			, size(ptr->size)
		{}
		HANDLE file;
		size_t size;
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

	struct file_seek_method
	{
		enum
		{
			begin = FILE_BEGIN,
			end = FILE_END,
			current = FILE_CURRENT
		};
	};
	//文件IO服务实现
	class _FileIOService
	{
	public:
		using iocp_t = stdx::iocp<file_io_context>;
		_FileIOService()
			:m_iocp()
			,m_alive(std::make_shared<bool>(true))
		{
			init_threadpoll();
		}
		//_FileIOService(const iocp_t &iocp)
		//	:m_iocp(iocp)
		//{}
		delete_copy(_FileIOService);
		~_FileIOService() 
		{
			*m_alive = false;
		}
		HANDLE create_file(const std::string &path, DWORD access_type, DWORD file_open_type, DWORD shared_model)
		{
			HANDLE file = CreateFile(path.c_str(), access_type, shared_model, 0, file_open_type, FILE_FLAG_OVERLAPPED, 0);
			if (file == INVALID_HANDLE_VALUE)
			{
				_ThrowWinError
			}
			m_iocp.bind(file);
			return file;
		}

		void read_file(HANDLE file, const DWORD &size, const int64 &offset, std::function<void(file_read_event, std::exception_ptr)> &&callback)
		{
			file_io_context *context = new file_io_context;
			int64_union li;
			li.value = offset;
			context->m_ol.Offset = li.low;
			context->m_ol.OffsetHigh = li.height;
			context->eof = false;
			context->file = file;
			context->offset = offset;
			context->buffer = (char*)std::calloc(size, sizeof(char));
			context->size = size;
			std::function<void(file_io_context*, std::exception_ptr)> *call = new std::function<void(file_io_context*, std::exception_ptr)>;
			*call = [callback, size](file_io_context *context_ptr, std::exception_ptr error)
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
				callback(context, nullptr);
			};
			context->callback = call;
			if (!ReadFile(file, context->buffer, size, &(context->size), &(context->m_ol)))
			{
				try
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
				catch (const std::exception&)
				{
					callback(stdx::file_read_event(), std::current_exception());
					return;
				}
			}
			
			return;
		}
		void write_file(HANDLE file, const char *buffer, const size_t &size,const int64 &offset, std::function<void(file_write_event, std::exception_ptr)> &&callback)
		{
			file_io_context *context_ptr = new file_io_context;
			int64_union li;
			li.value = offset;
			context_ptr->m_ol.Offset = li.low;
			context_ptr->m_ol.OffsetHigh = li.height;
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
				callback(context, nullptr);
			};
			context_ptr->callback = call;
			if (!WriteFile(file, buffer, size, &(context_ptr->size), &(context_ptr->m_ol)))
			{
				try
				{
					_ThrowWinError
				}
				catch (const std::exception&)
				{
					callback(stdx::file_write_event(), std::current_exception());
					return;
				}
			}
		}


		int64 get_file_size(HANDLE file) const
		{
			LARGE_INTEGER li;
			::GetFileSizeEx(file, &li);
			return li.QuadPart;
		}

		void close_file(HANDLE file)
		{
			CloseHandle(file);
		}
	private:
		iocp_t m_iocp;
		std::shared_ptr<bool> m_alive;

		void init_threadpoll() noexcept
		{
			for (size_t i = 0,cores = cpu_cores(); i < cores; i++)
			{
				stdx::threadpool::run([](iocp_t &iocp,std::shared_ptr<bool> alive)
				{
					while (*alive)
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
						try
						{
							(*call)(context_ptr, error);
						}
						catch (const std::exception&)
						{
						}
						delete call;
					}
				}, m_iocp,m_alive);
			}
			//stdx::threadpool::run([](iocp_t &iocp)
			//{
			//	auto *context_ptr = iocp.get();
			//	std::exception_ptr error(nullptr);
			//	try
			//	{
			//		if (!GetOverlappedResult(context_ptr->file, &(context_ptr->m_ol), &(context_ptr->size), false))
			//		{
			//			_ThrowWinError
			//		}
			//	}
			//	catch (const std::exception&)
			//	{
			//		error = std::current_exception();
			//	}
			//	auto *call = context_ptr->callback;
			//	try
			//	{
			//		(*call)(context_ptr, error);
			//	}
			//	catch (const std::exception&)
			//	{
			//	}
			//	delete call;
			//}, m_iocp);
		}
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
		//file_io_service(const iocp_t &iocp)
		//	:m_impl(std::make_shared<_FileIOService>(iocp))
		//{}
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
		void read_file(HANDLE file, const size_t &size, const int64 &offset, std::function<void(file_read_event, std::exception_ptr)> &&callback)
		{
			return m_impl->read_file(file, size, offset, std::move(callback));
		}
		void write_file(HANDLE file, const char *buffer, const size_t &size,const int64 &offset, std::function<void(file_write_event, std::exception_ptr)> &&callback)
		{
			return m_impl->write_file(file, buffer, size,offset, std::move(callback));
		}
		void close_file(HANDLE file)
		{
			return m_impl->close_file(file);
		}
		int64 get_file_size(HANDLE file) const
		{
			return m_impl->get_file_size(file);
		}
	private:
		impl_t m_impl;
	};

	//异步文件流实现
	class _FileStream
	{
		using io_service_t = file_io_service;
	public:
		_FileStream(const io_service_t &io_service)
			:m_io_service(io_service)
			,m_file(INVALID_HANDLE_VALUE)
		{}

		~_FileStream()
		{
			if (m_file != INVALID_HANDLE_VALUE)
			{
				m_io_service.close_file(m_file);
				m_file = INVALID_HANDLE_VALUE;
			}
		}

		void init(const std::string &path, const DWORD &access_type, const DWORD &open_type, const DWORD &shared_model)
		{
			m_file = m_io_service.create_file(path, access_type, open_type, shared_model);
		}

		stdx::task<file_read_event> read(const size_t &size, const int64 &offset)
		{
			if (!m_io_service)
			{
				throw std::logic_error("this io service has been free");
			}
			stdx::promise_ptr<file_read_event> promise = stdx::make_promise_ptr<file_read_event>();
			stdx::task<file_read_event> task([promise]()
			{
				return promise->get_future().get();
			});
			m_io_service.read_file(m_file, size, offset, [promise, task](file_read_event context, std::exception_ptr error) mutable
			{
				if (error)
				{
					promise->set_exception(error);
				}
				else
				{
					promise->set_value(context);
				}
				task.run_on_this_thread();
			});
			return task;
		}
		//返回true则继续
		template<typename _Fn>
		void read_utill(const size_t &size, const int64 &offset, const _Fn &call)
		{
			using args_t = typename stdx::function_info<_Fn>::arguments;
			static_assert(std::is_same<args_t::First, stdx::task_result<stdx::file_read_event>>::value, "the input function not be allowed");
			this->read(size, offset).then([call, offset, size, this](stdx::task_result<stdx::file_read_event> r) mutable
			{
				auto ex = call;
				if (std::invoke(ex,r))
				{
					auto e = r.get();
					read_utill(size, e.buffer.size() + offset, ex);
				}
			});
		}

		template<typename _Fn, typename _ErrHandler>
		void read_utill_eof(const size_t &size, const int64 &offset, const _Fn &call,const _ErrHandler &err_handler)
		{
			using args_t = typename stdx::function_info<_Fn>::arguments;
			static_assert(std::is_same<args_t::First, stdx::file_read_event>::value, "the input function not be allowed");
			return read_utill(size, offset, [call](stdx::task_result<stdx::file_read_event> r) 
			{
				try
				{
					auto e = r.get();
					auto ex = call;
					std::invoke(ex, e);
					if (e.eof)
					{
						return false;
					}
					else
					{
						return true;
					}
				}
				catch (const std::exception&)
				{
					std::invoke(err_handler, std::current_exception());
					return false;
				}
			});
		}

		stdx::task<stdx::file_read_event> read_to_end(const int64 &offset)
		{
			return read(size()-offset,offset);
		}
		

		stdx::task<file_write_event> write(const char* buffer, const size_t &size,const int64 &offset)
		{
			if (!m_io_service)
			{
				throw std::logic_error("this io service has been free");
			}
			stdx::promise_ptr<file_write_event> promise = stdx::make_promise_ptr<file_write_event>();
			stdx::task<file_write_event> task([promise]()
			{
				return promise->get_future().get();
			});
			m_io_service.write_file(m_file, buffer, size,offset, [promise, task](file_write_event context, std::exception_ptr error) mutable
			{
				if (error)
				{
					promise->set_exception(error);
				}
				else
				{
					promise->set_value(context);
				}
				task.run_on_this_thread();
			});
			return task;
		}

		void close()
		{
			if (m_file != INVALID_HANDLE_VALUE)
			{
				m_io_service.close_file(m_file);
				m_file = INVALID_HANDLE_VALUE;
			}
		}

		int64 size() const
		{
			return m_io_service.get_file_size(m_file);
		}
	private:
		io_service_t m_io_service;
		HANDLE m_file;
	};
	class file_stream
	{
		using impl_t = std::shared_ptr<_FileStream>;
		using io_service_t = file_io_service;
	public:

		explicit file_stream(const io_service_t &io_service)
			:m_impl(std::make_shared<_FileStream>(io_service))
		{}


		file_stream(const file_stream &other)
			:m_impl(other.m_impl)
		{}

		file_stream(file_stream &&other)
			:m_impl(std::move(other.m_impl))
		{}

		~file_stream() = default;

		void init(const std::string &path, DWORD access_type, DWORD open_type, DWORD shared_model)
		{
			return m_impl->init(path, access_type, open_type, shared_model);
		}

		file_stream &operator=(const file_stream &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		stdx::task<file_read_event> read(const size_t &size,const int64 &offset)
		{
			return m_impl->read(size, offset);
		}

		stdx::task<file_write_event> write(const char* buffer,const size_t &size,const int64 &offset)
		{
			return m_impl->write(buffer, size,offset);
		}
		stdx::task<file_write_event> write(const std::string &str,const int64 &offset)
		{
			return m_impl->write(str.c_str(), str.size(),offset);
		}

		template<typename _Fn>
		void read_utill(const size_t &size, const int64 &offset, const _Fn &call)
		{
			return m_impl->read_utill(size, offset, call);
		}

		template<typename _Fn, typename _ErrHandler>
		void read_utill_eof(const size_t &size, const int64 &offset, const _Fn &call, const _ErrHandler &err_handler)
		{
			return m_impl->read_utill_eof(size, offset,call,err_handler);
		}

		stdx::task<stdx::file_read_event> read_to_end(const int64 &offset)
		{
			return m_impl->read_to_end(offset);
		}

		int64 size() const
		{
			return m_impl->size();
		}

		void close()
		{
			return m_impl->close();
		}
	private:
		impl_t m_impl;
	};
}

namespace stdx
{
	inline stdx::file_stream open_file(const stdx::file_io_service &io_service, const std::string &path, const int32 &access_type, const int32 &open_type)
	{
		stdx::file_stream file(io_service);
		file.init(path, access_type, open_type, stdx::file_shared_model::shared_read|stdx::file_shared_model::shared_write);
		return file;
        }
}
#undef _ThrowWinError
#endif // WIN32
#ifdef LINUX

struct file_io_context
{
	file_io_context()
	{
	}
	~file_io_context() = default;
	int file;
	char *buffer;
	size_t size;
	size_t offset;
	bool eof;
	std::function<void(file_io_context*, std::exception_ptr)> callback;
};
#endif //LINUX
