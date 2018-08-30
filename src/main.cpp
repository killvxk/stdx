#include "ziran/net/ip_header.h"
#include "ziran/net/tcp_header.h"
#include "ziran/net/udp_header.h"
#include "ziran/net/pseudo_header.h"
#include <iostream>
#include "ziran/async/parallel.h"
#include "ziran/inject/service.h"
#include "ziran/event/event.h"
#include <thread>

int main()
{
	ziran::inject::service_collection services;
	services.register_type<int>()
		.register_type<std::string, ziran::inject::singleton<std::string>>([]() 
		{
			return "abc";
		});
	int v1 = services.resolve_type<int>();
	auto v2 = services.resolve_type<std::string>();
	ziran::event::event<int, std::string> event;
	event.register_listener([](int i,std::string str) 
	{
		std::cout << i << str << std::endl;
	});
	event.pulish(v1, v2);
	//ziran::inject::service<int, ziran::inject::life_time::singleton>::value = 10;
	//std::cout << sizeof(ziran::net::ip_header) << std::endl
	//	<< sizeof(ziran::net::tcp_header) << std::endl
	//	<< sizeof(ziran::net::udp_header) << std::endl
	//	<< sizeof(ziran::net::pseudo_header) << std::endl;
	//auto res = ziran::async::parallel::invoke<int>({ 
	//	[]() { std::this_thread::sleep_for(std::chrono::seconds(2));  return 1; }
	//,[]() {return 2; }
	//, []() {return 3; }
	//, []() {return 4; } 
	//	});
	system("pause");
	return 0;
}