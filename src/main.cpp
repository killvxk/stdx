#include <iostream>
//#include <stdx/io.h>
#include <stdx/async/task.h>
#include <WinSock2.h>
#include <Mswsock.h>
#pragma comment(lib,"Ws2_32.lib ")

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
int main()
{
	/*stdx::io_service<socket_io_context> io_service;
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = 8080;
	addr.sin_addr.S_un.S_addr = 0;
	bind(s,(sockaddr*)&addr,sizeof(sockaddr_in));
	listen(s, 100);
	LPFN_ACCEPTEX accept_ex;
	_GetAcceptEx(s,&accept_ex);
	LPFN_GETACCEPTEXSOCKADDRS get_accept_ex_sockaddr;
	_GetAcceptExSockaddr(s, &get_accept_ex_sockaddr);
	SOCKET new_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	std::shared_ptr<std::vector<char>> buf = std::make_shared<std::vector<char>>(4096);
	DWORD size;
	WSAOVERLAPPED ol;
	memset(&ol, 0, sizeof(ol));
	accept_ex(s, new_socket, buf->data(), buf->size() - ((sizeof(sockaddr_in) + 16) * 2), sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &size, &ol);
	io_service.bind((HANDLE)new_socket);
	stdx::threadpool::run([](stdx::io_service<socket_io_context> io) 
	{
		while (1)
		{
			auto context = io.get();
			std::cout << "get!";
		}
	},io_service);*/
	/*stdx::file_io_service io_service;
	stdx::async_fstream stream(io_service, "e://test.txt",stdx::file_access_type::read,stdx::open_type::open,stdx::file_shared_model::shared_read);
	stream.read().then([](stdx::task_result<stdx::file_io_info> r) 
	{
		try
		{
			auto info = r.get();
			std::cout << info.get_buffer();
		}
		catch (const std::exception&)
		{
			auto code = GetLastError(); 
			std::string str("windows system error:"); 
			str.append(std::to_string(code)); 
			std::cout << str;
		}
	});*/
	std::cin.get();
	return 0;
}