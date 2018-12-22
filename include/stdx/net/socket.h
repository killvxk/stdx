#pragma once
#include <stdx/exception.h>
#include <stdx/async/task.h>
#include <stdx/net/addr.h>
#include <stdx/buffer.h>
namespace stdx
{
	class net_exception :public std::exception
	{
	public:
		net_exception()
			:exception()
		{}
		net_exception(const std::string &str)
			:exception(str.c_str())
		{}
		net_exception(const int &code)
			:exception(std::string(std::string("error code:")+std::to_string(code)).c_str())
		{}
		~net_exception() = default;

	private:
	};
	namespace net
	{
		struct socket_type
		{
			enum
			{
				raw = 3,
				stream = 1,
				dgram = 2
			};
		};
#ifdef WIN32
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")
		struct _WSAStarter
		{
			WSADATA m_data;
			_WSAStarter()
			{
				if (WSAStartup(MAKEWORD(2, 2), &m_data))
				{
					throw stdx::net_exception("无法初始化winsocket");
				}
			}
			~_WSAStarter()
			{
				WSACleanup();
			}
		};
		_WSAStarter _starter;
		SOCKET make_socket(const int &addr_family, const int &socket_type, const int &proto_type)
		{
			SOCKET s = socket(addr_family, socket_type, proto_type);
			if (s == INVALID_SOCKET)
			{
				throw stdx::net_exception("无法创建Socket");
			}
			return s;
		}
		void make_listen(SOCKET s, int &backlog)
		{
			if (listen(s, backlog)==SOCKET_ERROR)
			{
				throw stdx::net_exception(WSAGetLastError());
			}
		}
		class socket
		{
		public:
			socket(const int &addr_family,const int &socket_type,const int &proto_type)
				:native_handle(make_socket(addr_family,socket_type,proto_type))
			{
				
			}

			~socket()
			{
			}

			void listen(const int &backlog)
			{
				make_listen(native_handle,backlog);
			}

			template<typename _Buffer = stdx::buffer<1024>>
			int write(_Buffer &buffer)
			{
				int r = send(native_handle, buffer.c_str(), buffer.size(), 0);
				if (r == SOCKET_ERROR)
				{
					throw stdx::net_exception(WSAGetLastError());
				}
				return r;
			}

			template<typename _Buffer=stdx::buffer<1024>>
			int read(_Buffer &buffer)
			{
				int r = recv(native_handle, buffer.c_str(),buffer.size() ,0);
				if (r == SOCKET_ERROR)
				{
					throw stdx::net_exception(WSAGetLastError());
				}
				return r;
			}
		private:

			SOCKET native_handle;
		};
#endif // WIN32
#ifdef UNIX
#include <sys/socket.h>
		class socket
		{
		public:
			socket(const int &addr_family, const int &socket_type, const int &proto_type)
				:native_handle(socket(addr_family,socket_type,proto_type))
			{

			}
			~socket()
			{
			}
		private:
			int native_handle;
		};
#endif // UNIX

	}
}