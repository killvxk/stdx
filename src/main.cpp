#include <stdx/io.h>
#include <stdx/net/socket.h>
#include <iostream>
#include <stdx/string.h>
#include <stdx/file.h>
int main()
{
#pragma region web_test
	stdx::network_io_service service;
	stdx::socket s = stdx::open_socket(service,stdx::addr_family::ip,stdx::socket_type::stream,stdx::protocol::tcp);
	try
	{
		stdx::network_addr addr("0.0.0.0", 5000);
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
			try
			{
				stdx::file_stream stream  = stdx::open_file(file_io_service,"./index.html", stdx::file_access_type::read, stdx::file_open_type::open);
				stream.read_to_end(0).then([c](stdx::file_read_event e) mutable
				{
					std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8;\r\nContent-Length:";
					str.append(std::to_string(e.buffer.size()));
					str.append("\r\n\r\n");
					str.append(e.buffer);
					c.send(str.c_str(), str.size());
				});
			}
			catch (const std::exception&e)
			{
				std::cerr << e.what() << std::endl;
			}
		});
	}
	std::cin.get();
#pragma endregion
	return 0;
}