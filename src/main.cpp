#include <stdx/io.h>
#include <stdx/net/socket.h>
#include <iostream>
#include <stdx/string.h>
#include <stdx/file.h>
int main()
{
#pragma region web_test
	stdx::network_io_service service;
	stdx::socket s(service, stdx::addr_family::ip, stdx::socket_type::stream, stdx::protocol::tcp);
	try
	{
		stdx::network_addr addr("0.0.0.0", 25565);
		s.bind(addr);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what();
		return -1;
	}
	s.listen(1024);
	stdx::file_io_service file_io_service;
	while (true)
	{
		auto c = s.accept();
		c.recv_utill_exception(1024, [c,file_io_service](stdx::task_result<stdx::network_recv_event> e) mutable
		{
			stdx::async_file_stream stream(file_io_service, "./index.html", stdx::file_access_type::read, stdx::file_open_type::open, stdx::file_shared_model::shared_read);
			stream.read_utill_eof(8192,0).then([c](std::string e)mutable 
			{
				std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8;\r\nContent-Length:";
				str.append(std::to_string(e.size()));
				str.append("\r\n\r\n");
				str.append(e);
				c.send(str.c_str(), str.size());
			});
		});
	}
	std::cin.get();
#pragma endregion
	std::cin.get();
	return 0;
}