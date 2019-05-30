#include <stdx/io.h>
#include <stdx/net/socket.h>
#include <stdx/sys/window.h>
#include <iostream>
#include <stdx/string.h>
#include <ppl.h>
int main()
{
#pragma region web_test
	/*stdx::network_io_service service;
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
	while (true)
	{
		auto c = s.accept();
		c.recv(1024).then([c](stdx::task_result<stdx::network_recv_event> e) mutable
		{
			std::string data = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><body><h1>OK!</h1></body></html>";
			c.send(data.c_str(), data.size()).then([c](stdx::network_send_event)mutable
			{
				c.close();
			});
		});
	}
	std::cin.get();*/
#pragma endregion
	stdx::stop_watcher watch;
	watch.begin();
	for (size_t i = 0; i < 1000; i++)
	{
		concurrency::create_task([]() 
		{
			return 1 + 1;
		});
	}
	watch.end();
	std::cout << "PPL:" << watch.time() << std::endl;
	watch.clean();
	watch.begin();
	return 0;
}