#include "ziran/net/ip_header.h"
#include "ziran/net/tcp_header.h"
#include "ziran/net/udp_header.h"
#include "ziran/net/pseudo_header.h"
#include <iostream>
#include "ziran/async/task.h"
#include "ziran/inject/service.h"
#include "ziran/event/event.h"
#include <thread>
#include "ziran/quicksort.h"
#include "ziran/function.h"
#include "ziran/async/thread_pool.h"
int main()
{
	ziran::async::thread_pool pool;
	pool.run_task([]() {std::cout << std::this_thread::get_id()<<std::endl; });
	pool.run_task([]() {std::cout << std::this_thread::get_id() << std::endl; });
	pool.run_task([]() {std::cout << std::this_thread::get_id() << std::endl; });
	pool.run_task([]() {std::cout << std::this_thread::get_id() << std::endl; });
	pool.run_task([]() {std::cout << std::this_thread::get_id() << std::endl; });
	pool.run_task([]() {std::cout << std::this_thread::get_id() << std::endl; });
	std::cin.get();
	return 0;
}