#include <stdx/async/task.h>
#include <iostream>
//#include <stdx/file.h>
//#include <stdx/net/socket.h>
#include <sstream>
#include <stdx/string.h>
int main()
{
#ifdef WIN32
	std::string str = "你好";
	std::wcout.imbue(std::locale(std::locale(), "", LC_CTYPE));
	std::wcout << stdx::utf8_to_unicode(stdx::ansi_to_utf8(str));
	system("pause");
//#pragma region web_test
//	stdx::network_io_service service;
//	stdx::socket s = stdx::open_socket(service, stdx::addr_family::ip, stdx::socket_type::stream, stdx::protocol::tcp);
//	try
//	{
//		stdx::network_addr addr("127.0.0.1", 8080);
//		s.bind(addr);
//	}
//	catch (std::exception &e)
//	{
//		std::cerr << e.what();
//		return -1;
//	}
//	std::cout << "已监听http://localhost:8080" << std::endl;
//	s.listen(65535);
//	stdx::file_io_service file_io_service;
//	while (true)
//	{
//		auto c = s.accept();
//		/*c.recv_utill_error(1024, [c,file_io_service](stdx::network_recv_event e) mutable
//		{
//			stdx::file_stream stream = stdx::open_file(file_io_service, "./index.html", stdx::file_access_type::read, stdx::file_open_type::open);
//			stream.read_to_end(0).then([c](stdx::file_read_event e) mutable
//			{
//				std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8;\r\nContent-Length:";
//				str.append(std::to_string(e.buffer.size()));
//				str.append("\r\n\r\n");
//				str.append(e.buffer);
//				c.send(str.c_str(), str.size()).then([](stdx::task_result<stdx::network_send_event> &e)
//				{
//					try
//					{
//						e.get();
//					}
//					catch (const std::exception&err)
//					{
//						std::cerr << err.what() <<std::endl;
//					}
//				});
//			});
//		}, [c](std::exception_ptr &err)
//		{
//			if (err)
//			{
//				try
//				{
//					std::rethrow_exception(err);
//				}
//				catch (const std::system_error &e)
//				{
//					std::cerr <<e.code().value() <<std::endl<< e.what()<<std::endl;
//				}
//			}
//		});*/
//		c.recv(1024).then([file_io_service, c]()
//		{
//			stdx::file_stream stream = stdx::open_file(file_io_service, "./index.html", stdx::file_access_type::read, stdx::file_open_type::open);
//			stream.read_to_end(0).then([c](stdx::file_read_event e) mutable
//			{
//				std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8;\r\nContent-Length:";
//				str.append(std::to_string(e.buffer.size()));
//				str.append("\r\n\r\n");
//				str.append(e.buffer);
//				c.send(str.c_str(), str.size()).then([c](stdx::task_result<stdx::network_send_event> &e) mutable
//				{
//					c.close();
//				});
//			});
//		});
//	}
//	//auto t = stdx::async([]() 
//	//{
//	//	std::cout << "hello";
//	//});
//	std::cin.get();
//#pragma endregion
#endif // WIN32
	std::string str = U("你好");
	stdx::unicode_string uni_string;
	try
	{
		uni_string = stdx::utf8_to_unicode(str);
	}
	catch (const std::exception&e)
	{
		std::cerr <<"error:"<< e.what() << std::endl;
	}
	std::cout << uni_string.size()<<std::endl;
	for (auto begin = std::begin(uni_string), end = std::end(uni_string); begin != end; ++begin)
	{
		std::cout << (uint16)*begin << std::endl;
	}
	std::cout << stdx::unicode_to_utf8(uni_string);
	return 0;
}