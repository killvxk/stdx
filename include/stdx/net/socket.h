#pragma once
#include <stdx/exception.h>
#include <stdx/async/task.h>
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
		~net_exception() = default;

	private:
	};
	namespace net
	{
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
		};
		_WSAStarter _starter;
		class socket
		{
		public:
			socket();
			~socket();

		private:

			int native_handle;
		};
#endif // WIN32
#ifdef UNIX

#endif // UNIX

	}
}