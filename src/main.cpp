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
		stdx::network_addr addr("127.0.0.1", 8080);
		s.bind(addr);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what();
		return -1;
	}
	std::cout << "已监听http://localhost:8080" << std::endl;
	s.listen(65535);
	stdx::file_io_service file_io_service;
	while (true)
	{
		auto c = s.accept();
		c.recv_utill_error(1024, [c,file_io_service](stdx::network_recv_event e) mutable
		{
			stdx::file_stream stream = stdx::open_file(file_io_service, "./index.html", stdx::file_access_type::read, stdx::file_open_type::open);
			stream.read_to_end(0).then([c](stdx::file_read_event e) mutable
			{
				std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8;\r\nContent-Length:";
				str.append(std::to_string(e.buffer.size()));
				str.append("\r\n\r\n");
				str.append(e.buffer);
				c.send(str.c_str(), str.size()).then([](stdx::task_result<stdx::network_send_event> &e) 
				{
					try
					{
						e.get();
					}
					catch (const std::exception&err)
					{
						std::cerr << err.what() <<std::endl;
					}
				});
			});
		}, [c](std::exception_ptr &err)
		{
			if (err)
			{
				try
				{
					std::rethrow_exception(err);
				}
				catch (const std::system_error &e)
				{
					std::cerr <<e.code().value() <<std::endl<< e.what()<<std::endl;
				}
			}
		});
		//c.recv(1024).then([file_io_service,c]() 
		//{
		//	stdx::file_stream stream = stdx::open_file(file_io_service, "./index.html", stdx::file_access_type::read, stdx::file_open_type::open);
		//	stream.read_to_end(0).then([c](stdx::file_read_event e) mutable
		//	{
		//		std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8;\r\nContent-Length:";
		//		str.append(std::to_string(e.buffer.size()));
		//		str.append("\r\n\r\n");
		//		str.append(e.buffer);
		//		c.send(str.c_str(), str.size()).then([c](stdx::task_result<stdx::network_send_event> &e) mutable
		//		{
		//			c.close();
		//		});
		//	});
		//});
	}
#pragma endregion
	return 0;
}