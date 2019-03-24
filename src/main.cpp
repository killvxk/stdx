#include <stdx/io.h>
#include <stdx/net/socket.h>
#include <iostream>
int main()
{
	/*stdx::file_io_service io_service;
	stdx::async_file_stream stream(io_service,"e://test.txt",stdx::file_access_type::all,stdx::file_open_type::open,stdx::file_shared_model::shared_read);
	try
	{
		std::string str = "23333333";
		stream.write(str).then([&stream](stdx::file_write_event e) 
		{
			std::cout << "实际写入字节数" << e.size <<std::endl;
			return stream.read(1024, 0);
		})
		.then([](stdx::file_read_event e) 
		{
			std::cout << "文件尾:" << e.eof <<std::endl
						<<"文件内容:" << std::endl
						<< e.buffer;
		});
	}
	catch (const std::exception&e)
	{
		std::cerr << e.what();
	}*/
	stdx::_NetworkIOService service;
	SOCKET s(service.create_wsasocket(stdx::addr_family::ip, stdx::socket_type::stream, stdx::protocol::tcp));
	stdx::network_addr addr(inet_addr("192.168.1.2"),htons(25565U));
	try
	{
		service.connect(s, addr);
	}
	catch (const std::exception&e)
	{
		std::cerr << e.what();
		return 0;
	}
	std::string str("hello world");
	try
	{
		service.send(s, str.c_str(), str.size(), [](stdx::network_send_event e, std::exception_ptr error)
		{
			if (error)
			{
				try
				{
					std::rethrow_exception(error);
				}
				catch (const std::exception&e)
				{
					std::cerr << e.what();
				}
			}
			else
			{
				std::cout << "send ok!";
			}
		});
	}
	catch (std::exception &e)
	{
		std::cerr << e.what();
	}
	std::cin.get();
	return 0;
}