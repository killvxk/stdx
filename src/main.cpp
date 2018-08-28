#include "ziran/net/ip_header.h"
#include "ziran/net/tcp_header.h"
#include "ziran/net/udp_header.h"
#include "ziran/net/pseudo_header.h"
#include <iostream>
#include "ziran/async/parallel.h"
#include <thread>
int main()
{
	std::cout << sizeof(ziran::net::ip_header) << std::endl
		<< sizeof(ziran::net::tcp_header) << std::endl
		<< sizeof(ziran::net::udp_header) << std::endl
		<< sizeof(ziran::net::pseudo_header) << std::endl;
	auto res = ziran::async::parallel::invoke<int>({ 
		[]() { std::this_thread::sleep_for(std::chrono::seconds(2));  return 1; }
	,[]() {return 2; }
	, []() {return 3; }
	, []() {return 4; } 
		});
	system("pause");
	return 0;
}