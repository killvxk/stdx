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
	struct network_io_context
	{
		network_io_context()
		{
			std::memset(&m_ol, 0, sizeof(OVERLAPPED));
		}
		~network_io_context() = default;
		OVERLAPPED m_ol;
		SOCKET this_socket;
		sockaddr addr;
		char* buffer;
		DWORD size;
		SOCKET accept_socket;
	};

	class _NetworkIOService
	{
	public:
		using iocp_t = stdx::iocp<network_io_context>;
		_NetworkIOService()
			:m_iocp()
		{}
		_NetworkIOService(const iocp_t &iocp)
			:m_iocp(iocp)
		{}
		~_NetworkIOService() = default;
		SOCKET create_socket(int addr_family, int sock_type, int protocl)
		{
			SOCKET sock = socket(addr_family,sock_type,protocl);
			m_iocp.bind(sock);
			return sock;
		}
		SOCKET create_wsasocket(int addr_family,int sock_type,int protocl)
		{
			SOCKET sock = WSASocket(addr_family, sock_type,protocl, NULL, 0,WSA_FLAG_OVERLAPPED);
			return sock;
		}
	private:
		iocp_t m_iocp;
	};

	struct network_send_event
	{
		network_send_event() = default;
		~network_send_event() = default;
		network_send_event(const network_send_event &other)
			:sock(other.sock)
			, size(other.size)
		{}
		network_send_event(network_send_event &&other)
			:sock(std::move(other.sock))
			,size(std::move(other.size))
		{}
		network_send_event &operator=(const network_send_event &other)
		{
			sock = other.file;
			size = other.size;
			return *this;
		}
		network_send_event(network_io_context *ptr)
			:sock(ptr->this_socket)
			,size(ptr->size)
		{}
		SOCKET sock;
		size_t size;
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
		SOCKET accept_async(DWORD buffer_size)
		{
			if (accept_ex==NULL)
			{
				_GetAcceptEx(m_handle, &accept_ex);
			}
			SOCKET new_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			char *buffer = std::calloc(sizeof(char), buffer_size);
			DWORD size;
			
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