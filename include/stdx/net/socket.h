#pragma once
#include <stdx/async/task.h>
#include <stdx/buffer.h>
#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib ")
#endif
namespace stdx
{
#ifdef WIN32
#include <WinSock2.h>
struct _WSAStarter
{
	WSAData wsa;
	_WSAStarter()
		: wsa()
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

class ip_endpoint
{
  public:
	ip_endpoint(unsigned long ip, unsigned short port)
	{
		m_handle.sin_family = addr_family::ip;
		m_handle.sin_addr.S_un.S_addr = ip;
		m_handle.sin_port = port;
	}
	ip_endpoint(const ip_endpoint &other)
	{
		m_handle = other.m_handle;
	}
	~ip_endpoint() = default;
	operator SOCKADDR_IN *()
	{
		return &m_handle;
	}
	ip_endpoint &operator=(const ip_endpoint &other)
	{
		m_handle = other.m_handle;
	}
	template <typename _String>
	ip_endpoint make_addr(const _String &ip, unsigned short port)
	{
		return ip_endpoint(inet_addr(ip.c_str()), htonl(port));
	}

	ip_endpoint make_addr(char *ip, unsigned short port)
	{
		return ip_endpoint(inet_addr(ip), htonl(port));
	}

	ip_endpoint make_addr(unsigned short port)
	{
		return ip_endpoint(0, htonl(port));
	}

  private:
	SOCKADDR_IN m_handle;
};
class socket
{
  public:
	socket(int addr_family, int sock_type, int protocl)
	{
		m_handle = WSASocket(addr_family, sock_type, protocl, NULL, 0, WSAOVERLAPPED);
	}
	virtual ~socket()
	{
		closesocket(handle);
	}
	int write(const char *buffer,unsigned int len) virtual
	{
		
	}
  protected:
	WSASocket m_handle;
};
#endif //Win32
#ifdef UNIX

#endif // UNIX

} // namespace stdx