#pragma once
#include <stdx/exception.h>
#include <stdx/async/task.h>
#include <stdx/buffer.h>
#include <stdx/async/spin_lock.h>
#include <stdx/io.h>
#ifdef WIN32
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib,"Ws2_32.lib ")
#endif 
namespace stdx
{
#ifdef WIN32
	struct _WSAStarter
	{
		WSAData wsa;
		_WSAStarter()
			:wsa()
		{
			if (WSAStartup(MAKEWORD(2, 2), &wsa))
			{

			}
		}
		~_WSAStarter()
		{
			if (WSACleanup())
			{

			}
		}
	};
	_WSAStarter _wsastarter;
	struct protocol
	{
		enum
		{
			ip = IPPROTO_IP,
			tcp = IPPROTO_TCP,
			udp = IPPROTO_UDP
		};
	};
	struct socket_type
	{
		enum
		{
			raw = SOCK_RAW,
			stream = SOCK_STREAM,
			dgram = SOCK_DGRAM
		};
	};
	struct addr_family
	{
		enum
		{
			ip = AF_INET,
			ipv6 = AF_INET6
		};
	};

	class tcp_addr
	{
	public:
		tcp_addr(unsigned long ip,unsigned short port)
		{
			m_handle.sin_family = addr_family::ip;
			m_handle.sin_addr.S_un.S_addr = ip;
			m_handle.sin_port = port;
		}
		tcp_addr(const tcp_addr &other)
		{
			m_handle = other.m_handle;
		}
		~tcp_addr() = default;
		operator SOCKADDR_IN* ()
		{
			return &m_handle;
		}
		tcp_addr &operator=(const tcp_addr &other)
		{
			m_handle = other.m_handle;
		}
		template<typename _String>
		tcp_addr make_addr(const _String &ip, unsigned short port)
		{
			return tcp_addr(inet_addr(ip.c_str()), htonl(port));
		}

		tcp_addr make_addr(char *ip, unsigned short port)
		{
			return tcp_addr(inet_addr(ip), htonl(port));
		}

		tcp_addr make_addr(unsigned short port)
		{
			return tcp_addr(0, htonl(port));
		}
	private:
		SOCKADDR_IN m_handle;
	};
	struct socket_io_context
	{
		OVERLAPPED m_ol;
		WSABUF m_wsabuf;
		SOCKET m_socket;
		SOCKADDR_IN m_addr;
		char m_buffer[4096];
		int m_io_type;
		DWORD m_send;
		DWORD m_recv;
	};
	struct socket_io_type
	{
		enum
		{
			idle = 0,
			accept = 1,
			send = 2,
			recv = 3
		};
	};
	using socket_io_service = stdx::io_service<stdx::socket_io_context>;

	struct task_ol
	{

	};
	class _Socket
	{
	public:
		_Socket(int addr_family,int sock_type,int protocl)
			:_Socket(WSASocket(addr_family, sock_type, protocl, NULL, 0, WSAOVERLAPPED))
		{}
		~_Socket()
		{
			closesocket(handle);
		}
		SOCKET accept()
		{
			if (accept_ex==NULL)
			{
				_GetAcceptEx(m_handle, &accept_ex);
			}
			SOCKET new_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			std::shared_ptr<std::vector<char>> buf = std::make_shared<std::vector<char>>(4096);
			DWORD size;
			WSAOVERLAPPED ol;
			memset(&ol, 0, sizeof(ol));
			accept_ex(m_handle, new_socket, buf->data(), buf->size() - ((sizeof(sockaddr_in) + 16) * 2), sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &size, &ol);
			return new_socket;
		}
		void use_io_service(const stdx::socket_io_service &io_service)
		{
			io_service.bind(m_handle);
		}
	private:
		SOCKET m_handle;
		_Socket(SOCKET handle)
			:m_handle(handle)
		{
		}
		static LPFN_ACCEPTEX accept_ex;
	};
	void _GetAcceptEx(const SOCKET &s, LPFN_ACCEPTEX *ptr)
	{
		GUID id = WSAID_ACCEPTEX;
		DWORD buf;
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &id, sizeof(id), &ptr, sizeof(ptr), &buf, NULL, NULL);
	}
	void _GetAcceptExSockaddr(const SOCKET &s, LPFN_GETACCEPTEXSOCKADDRS *ptr)
	{
		GUID id = WSAID_GETACCEPTEXSOCKADDRS;
		DWORD buf;
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &id, sizeof(id), &ptr, sizeof(ptr), &buf, NULL, NULL);
	}
#endif //Win32
#ifdef UNIX

#endif // UNIX


}