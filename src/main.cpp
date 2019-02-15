#include <stdx/io.h>
#include <iostream>
int main()
{
	stdx::file_io_service io_service;
	stdx::async_file_stream stream(io_service,"e://test.txt",stdx::file_access_type::all,stdx::file_open_type::open,stdx::file_shared_model::shared_read);
	try
	{
		std::string str = "23333";
		stream.write(str)
			.then([stream](stdx::file_write_event e) mutable
			{
				std::cout << "实际写入字节数:" << e.size <<std::endl;
				return stream.read(1024, 0);
			})
			.then([](stdx::file_read_event e) 
			{
				std::cout << e.buffer;
			});
	}
	catch (const std::exception&e)
	{
		std::cerr << e.what();
	}
	std::cin.get();
	return 0;
}