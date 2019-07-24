#include <stdx/file.h>

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

stdx::_FileIOService::_FileIOService()
	:m_iocp()
	, m_alive(std::make_shared<bool>(true))
{
	init_threadpoll();
}

stdx::_FileIOService::~_FileIOService()
{
	*m_alive = false;
}

HANDLE stdx::_FileIOService::create_file(const std::string &path, DWORD access_type, DWORD file_open_type, DWORD shared_model)

{
	HANDLE file = CreateFile(path.c_str(), access_type, shared_model, 0, file_open_type, FILE_FLAG_OVERLAPPED, 0);
	if (file == INVALID_HANDLE_VALUE)
	{
		_ThrowWinError
	}
	m_iocp.bind(file);
	return file;
}

void stdx::_FileIOService::read_file(HANDLE file, const DWORD &size, const int64 &offset, std::function<void(file_read_event, std::exception_ptr)> &&callback)
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
			//��������
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
			delete call;
			delete context;
			callback(stdx::file_read_event(), std::current_exception());
			return;
		}
	}
	return;
}

void stdx::_FileIOService::write_file(HANDLE file, const char *buffer, const size_t &size, const int64 &offset, std::function<void(file_write_event, std::exception_ptr)> &&callback)
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
			delete call;
			delete context_ptr;
			callback(stdx::file_write_event(), std::current_exception());
			return;
		}
	}
}

int64 stdx::_FileIOService::get_file_size(HANDLE file) const
{
	LARGE_INTEGER li;
	::GetFileSizeEx(file, &li);
	return li.QuadPart;
}

void stdx::_FileIOService::init_threadpoll() noexcept
{
	for (size_t i = 0, cores = cpu_cores() * 2; i < cores; i++)
	{
		stdx::threadpool::run([](iocp_t &iocp, std::shared_ptr<bool> alive)
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
		}, m_iocp, m_alive);
	}
}

stdx::_FileStream::_FileStream(const io_service_t &io_service)
	:m_io_service(io_service)
	, m_file(INVALID_HANDLE_VALUE)
{}

stdx::_FileStream::~_FileStream()
{
	if (m_file != INVALID_HANDLE_VALUE)
	{
		m_io_service.close_file(m_file);
		m_file = INVALID_HANDLE_VALUE;
	}
}

stdx::task<stdx::file_read_event> stdx::_FileStream::read(const size_t &size, const int64 &offset)
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

stdx::task<stdx::file_write_event> stdx::_FileStream::write(const char* buffer, const size_t &size, const int64 &offset)
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
	m_io_service.write_file(m_file, buffer, size, offset, [promise, task](file_write_event context, std::exception_ptr error) mutable
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

void stdx::_FileStream::close()
{
	if (m_file != INVALID_HANDLE_VALUE)
	{
		m_io_service.close_file(m_file);
		m_file = INVALID_HANDLE_VALUE;
	}
}

stdx::file_stream stdx::open_file(const stdx::file_io_service &io_service, const std::string &path, const int32 &access_type, const int32 &open_type)
{
	stdx::file_stream file(io_service);
	file.init(path, access_type, open_type, stdx::file_shared_model::shared_read | stdx::file_shared_model::shared_write);
	return file;
}
#undef _ThrowWinError
#endif // WIN32
#ifdef LINUX

#endif // LINUX