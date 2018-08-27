#include "ziran/net/ip_header.h"
#include "ziran/net/tcp_header.h"
#include "ziran/net/udp_header.h"
#include"ziran/net/pseudo_header.h"
#include <iostream>

int main()
{
	std::cout << sizeof(ziran::net::ip_header) << std::endl
		<< sizeof(ziran::net::tcp_header) << std::endl
		<< sizeof(ziran::net::udp_header) << std::endl
		<< sizeof(ziran::net::pseudo_header) << std::endl;
	system("pause");
	return 0;
}