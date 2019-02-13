#include <stdx/io.h>
#include <iostream>
int main()
{
	stdx::file_io_service io_service;
	stdx::async_fstream stream(io_service,"e://test.txt",stdx::file_access_type::all,stdx::file_open_type::open,stdx::file_shared_model::shared_read);
	try
	{
		std::string str = "原来WriteFile会覆盖";
		stream.write(str.c_str(),str.size()).wait();
		stream.read(1024, 0)
			.then([](stdx::file_read_event context)
		{
			std::cout << context.buffer;
		});
	}
	catch (const std::exception&e)
	{
		std::cerr << e.what();
	}
	std::cin.get();
	return 0;
}