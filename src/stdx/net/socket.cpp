#include <stdx/net/socket.h>

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

#define _ThrowWSAError 	auto _ERROR_CODE = WSAGetLastError(); \
						if(_ERROR_CODE != WSA_IO_PENDING)\
						{\
							std::string _ERROR_STR("windows WSA error:");\
							_ERROR_STR.append(std::to_string(_ERROR_CODE));\
							throw std::system_error(std::error_code(_ERROR_CODE,std::system_category()),_ERROR_STR.c_str());\
						}\

stdx::_WSAStarter stdx::_wsastarter;

stdx::_NetworkIOService::_NetworkIOService()
	:m_iocp()
	, m_alive(std::make_shared<bool>(true))
{
	init_threadpoll();
}

stdx::_NetworkIOService::~_NetworkIOService()
{
	*m_alive = false;
	for (size_t i = 0,size = cpu_cores()*2; i < size; i++)
	{
		m_iocp.post(0, nullptr, nullptr);
	}
}

SOCKET stdx::_NetworkIOService::create_socket(const int &addr_family, const int &sock_type, const int &protocol)
{
	SOCKET sock = ::socket(addr_family, sock_type, protocol);
	if (sock == INVALID_SOCKET)
	{
		_ThrowWSAError
	}
	m_iocp.bind(sock);
	return sock;
}
SOCKET stdx::_NetworkIOService::create_wsasocket(const int &addr_family, const int &sock_type, const int &protocol)
{
	SOCKET sock = WSASocket(addr_family, sock_type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET)
	{
		_ThrowWSAError
	}
	m_iocp.bind(sock);
	return sock;
}

void stdx::_NetworkIOService::send(SOCKET sock, const char* data, const size_t &size, std::function<void(network_send_event, std::exception_ptr)> &&callback)
{
	auto *context_ptr = new network_io_context;
	context_ptr->this_socket = sock;
	char *buffer = (char*)std::calloc(sizeof(char), size);
	std::memcpy(buffer, data, size);
	context_ptr->buffer.buf = buffer;
	context_ptr->buffer.len = size;
	auto *call = new std::function <void(network_io_context*, std::exception_ptr)>;
	*call = [callback](network_io_context *context_ptr, std::exception_ptr error)
	{
		if (error)
		{
			std::free(context_ptr->buffer.buf);
			delete context_ptr;
			callback(network_send_event(), error);
			return;
		}
		network_send_event context(context_ptr);
		std::free(context_ptr->buffer.buf);
		delete context_ptr;
		callback(context, nullptr);
	};
	context_ptr->callback = call;
	if (WSASend(sock, &(context_ptr->buffer), 1, &(context_ptr->size), NULL, &(context_ptr->m_ol), NULL) == SOCKET_ERROR)
	{
		try
		{
			_ThrowWSAError
		}
		catch (const std::exception&)
		{
			delete call;
			delete context_ptr;
			callback(stdx::network_send_event(), std::current_exception());
			return;
		}
	}
}

void stdx::_NetworkIOService::recv(SOCKET sock, const size_t &size, std::function<void(network_recv_event, std::exception_ptr)> &&callback)
{
	auto *context_ptr = new network_io_context;
	context_ptr->this_socket = sock;
	char *buf = (char*)std::calloc(sizeof(char), size);
	//char *buf = stdx::calloc<char>(size);
	context_ptr->buffer.buf = buf;
	context_ptr->buffer.len = size;
	auto *call = new std::function <void(network_io_context*, std::exception_ptr)>;
	*call = [callback](network_io_context *context_ptr, std::exception_ptr error)
	{
		if (error)
		{
			std::free(context_ptr->buffer.buf);
			delete context_ptr;
			callback(network_recv_event(), error);
			return;
		}
		network_recv_event context(context_ptr);
		delete context_ptr;
		callback(context, std::exception_ptr(nullptr));
	};
	context_ptr->callback = call;
	if (WSARecv(sock, &(context_ptr->buffer), 1, &(context_ptr->size), &(_NetworkIOService::recv_flag), &(context_ptr->m_ol), NULL) == SOCKET_ERROR)
	{
		try
		{
			_ThrowWSAError
		}
		catch (const std::exception&)
		{
			delete call;
			delete context_ptr;
			callback(stdx::network_recv_event(), std::current_exception());
			return;
		}
	}
}

void stdx::_NetworkIOService::connect(SOCKET sock, stdx::network_addr &addr)
{
	if (WSAConnect(sock, addr, network_addr::addr_len, NULL, NULL, NULL, NULL) == SOCKET_ERROR)
	{
		_ThrowWSAError
	}
}
SOCKET stdx::_NetworkIOService::accept(SOCKET sock)
{
	SOCKET s = WSAAccept(sock, NULL, 0, NULL, NULL);
	if (s == INVALID_SOCKET)
	{
		_ThrowWSAError
	}
	m_iocp.bind(s);
	return s;
}
SOCKET stdx::_NetworkIOService::accept(SOCKET sock, network_addr & addr)
{
	int size = network_addr::addr_len;
	SOCKET s = WSAAccept(sock, addr, &size, NULL, NULL);
	if (s == INVALID_SOCKET)
	{
		_ThrowWSAError
	}
	m_iocp.bind(s);
	return s;
}

void stdx::_NetworkIOService::listen(SOCKET sock, int backlog)
{
	if (::listen(sock, backlog) == SOCKET_ERROR)
	{
		_ThrowWSAError
	}
}

void stdx::_NetworkIOService::bind(SOCKET sock, network_addr & addr)
{
	if (::bind(sock, addr, network_addr::addr_len) == SOCKET_ERROR)
	{
		_ThrowWSAError
	}
}

void stdx::_NetworkIOService::send_to(SOCKET sock, const network_addr & addr, const char * data, const size_t & size, std::function<void(stdx::network_send_event, std::exception_ptr)>&& callback)
{
	stdx::network_io_context *context_ptr = new stdx::network_io_context;
	context_ptr->addr = addr;
	context_ptr->this_socket = sock;
	char *buf = (char*)std::calloc(sizeof(char), size);
	std::memcpy(buf, data, size);
	context_ptr->buffer.buf = buf;
	context_ptr->buffer.len = size;
	auto *call = new std::function <void(network_io_context*, std::exception_ptr)>;
	*call = [callback](network_io_context *context_ptr, std::exception_ptr error)
	{
		if (error)
		{
			std::free(context_ptr->buffer.buf);
			delete context_ptr;
			callback(network_send_event(), error);
			return;
		}
		network_send_event context(context_ptr);
		std::free(context_ptr->buffer.buf);
		delete context_ptr;
		callback(context, nullptr);
	};
	context_ptr->callback = call;
	if (WSASendTo(sock, &(context_ptr->buffer), 1, &(context_ptr->size), NULL, (context_ptr->addr), network_addr::addr_len, &(context_ptr->m_ol), NULL) == SOCKET_ERROR)
	{
		try
		{
			_ThrowWSAError
		}
		catch (const std::exception&)
		{
			delete call;
			delete context_ptr;
			callback(stdx::network_send_event(), std::current_exception());
			return;
		}
	}
}

void stdx::_NetworkIOService::recv_from(SOCKET sock, const network_addr & addr, const size_t & size, std::function<void(network_recv_event, std::exception_ptr)>&& callback)
{
	auto *context_ptr = new network_io_context;
	context_ptr->this_socket = sock;
	char *buf = (char*)std::calloc(sizeof(char), size);
	context_ptr->buffer.buf = buf;
	context_ptr->buffer.len = size;
	auto *call = new std::function <void(network_io_context*, std::exception_ptr)>;
	*call = [callback](network_io_context *context_ptr, std::exception_ptr error)
	{
		if (error)
		{
			std::free(context_ptr->buffer.buf);
			delete context_ptr;
			callback(network_recv_event(), error);
			return;
		}
		network_recv_event context(context_ptr);
		delete context_ptr;
		callback(context, std::exception_ptr(nullptr));
	};
	context_ptr->callback = call;
	if (WSARecvFrom(sock, &(context_ptr->buffer), 1, &(context_ptr->size), &(_NetworkIOService::recv_flag), context_ptr->addr, (LPINT)&(network_addr::addr_len), &(context_ptr->m_ol), NULL) == SOCKET_ERROR)
	{
		try
		{
			_ThrowWSAError
		}
		catch (const std::exception&)
		{
			delete call;
			delete context_ptr;
			callback(stdx::network_recv_event(), std::current_exception());
			return;
		}
	}
}

void stdx::_NetworkIOService::close(SOCKET sock)
{
	if (::closesocket(sock) == SOCKET_ERROR)
	{
		_ThrowWSAError
	}
}

 stdx::network_addr stdx::_NetworkIOService::get_local_addr(SOCKET sock) const
{
	network_addr addr;
	int len = network_addr::addr_len;
	if (getsockname(sock, addr, &len) == SOCKET_ERROR)
	{
		_ThrowWSAError
	}
	return addr;
}

 stdx::network_addr stdx::_NetworkIOService::get_remote_addr(SOCKET sock) const
 {
	 network_addr addr;
	 int len = network_addr::addr_len;
	 if (getpeername(sock, addr, &len) == SOCKET_ERROR)
	 {
		 _ThrowWSAError
	 }
	 return addr;
 }

 bool stdx::_NetworkIOService::poll(SOCKET sock, int16 mode, int32 timeout) const
 {
	 WSAPOLLFD fd;
	 fd.events = mode;
	 int r = WSAPoll(&fd, 1, timeout);
	 if (r == 0)
	 {
		 return false;
	 }
	 if (r > 0)
	 {
		 if (fd.revents == mode)
		 {
			 return true;
		 }
	 }
	 if (r == SOCKET_ERROR)
	 {
		 _ThrowWSAError
	 }
	 return false;
 }

 //static LPFN_ACCEPTEX accept_ex;
 //static LPFN_GETACCEPTEXSOCKADDRS get_addr_ex;

 void stdx::_NetworkIOService::init_threadpoll() noexcept
 {
	 for (size_t i = 0, cores = cpu_cores() * 2; i < cores; i++)
	 {
		 stdx::threadpool::run([](iocp_t iocp, std::shared_ptr<bool> alive)
		 {
			 while (*alive)
			 {
				 auto *context_ptr = iocp.get();
				 if (context_ptr == nullptr)
				 {
					 continue;
				 }
				 std::exception_ptr error(nullptr);
				 try
				 {
					 DWORD flag = 0;
					 if (!WSAGetOverlappedResult(context_ptr->this_socket, &(context_ptr->m_ol), &(context_ptr->size), false, &flag))
					 {
						 //在这里出错
						 _ThrowWSAError
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
 DWORD stdx::_NetworkIOService::recv_flag = 0;

 stdx::_Socket::_Socket(const io_service_t & io_service, SOCKET s)
	 :m_io_service(io_service)
	 , m_handle(s)
 {}

 stdx::_Socket::_Socket(const io_service_t & io_service)
	 : m_io_service(io_service)
	 , m_handle(INVALID_SOCKET)
 {}

 stdx::_Socket::~_Socket()
 {
	 if (m_handle != INVALID_SOCKET)
	 {
		 m_io_service.close(m_handle);
		 m_handle = INVALID_SOCKET;
	 }
 }

 stdx::task<stdx::network_send_event> &stdx::_Socket::send(const char * data, const size_t & size,stdx::task_complete_event<stdx::network_send_event> ce)
 {
	 if (!m_io_service)
	 {
		 throw std::logic_error("this io service has been free");
	 }
	 m_io_service.send(m_handle, data, size, [ce](stdx::network_send_event context, std::exception_ptr error) mutable
	 {
		 if (error)
		 {
			 ce.set_exception(error);
		 }
		 else
		 {
			 ce.set_value(context);
		 }
		 ce.run_on_this_thread();
	 });
	 return ce.get_task();
 }

 stdx::task<stdx::network_send_event> &stdx::_Socket::send_to(const network_addr & addr, const char * data, const size_t & size,stdx::task_complete_event<stdx::network_send_event> ce)
 {
	 if (!m_io_service)
	 {
		 throw std::logic_error("this io service has been free");
	 }
	 m_io_service.send_to(m_handle, addr, data, size, [ce](stdx::network_send_event context, std::exception_ptr error) mutable
	 {
		 if (error)
		 {
			 ce.set_exception(error);
		 }
		 else
		 {
			 ce.set_value(context);
		 }
		 ce.run_on_this_thread();
	 });
	 return ce.get_task();
 }

 stdx::task<stdx::network_recv_event> &stdx::_Socket::recv(const size_t & size, stdx::task_complete_event<stdx::network_recv_event> ce)
 {
	 if (!m_io_service)
	 {
		 throw std::logic_error("this io service has been free");
	 }
	 m_io_service.recv(m_handle, size, [ce](stdx::network_recv_event context, std::exception_ptr error) mutable
	 {
		 if (error)
		 {
			 ce.set_exception(error);
		 }
		 else
		 {
			 ce.set_value(context);
		 }
		 ce.run_on_this_thread();
	 });
	 return ce.get_task();
 }

 stdx::task<stdx::network_recv_event> &stdx::_Socket::recv_from(const network_addr & addr, const size_t & size,stdx::task_complete_event<stdx::network_recv_event> ce)
 {
	 if (!m_io_service)
	 {
		 throw std::logic_error("this io service has been free");
	 }
	 m_io_service.recv_from(m_handle, addr, size, [ce](stdx::network_recv_event context, std::exception_ptr error) mutable
	 {
		 if (error)
		 {
			 ce.set_exception(error);
		 }
		 else
		 {
			 ce.set_value(context);
		 }
		 ce.run_on_this_thread();
	 });
	 return ce.get_task();
 }

 void stdx::_Socket::close()
 {
	 if (m_handle != INVALID_SOCKET)
	 {
		 m_io_service.close(m_handle);
		 m_handle = INVALID_SOCKET;
	 }
 }
 stdx::socket stdx::open_socket(const stdx::network_io_service & io_service, const int & addr_family, const int & sock_type, const int & protocol)
 {
	 stdx::socket sock(io_service);
	 sock.init(addr_family, sock_type, protocol);
	 return sock;
 }
#undef _ThrowWinError
#undef _ThrowWSAError
#endif

#ifdef LINUX

#endif // LINUX