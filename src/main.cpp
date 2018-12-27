#include <iostream>
#include <stdx/cmder.h>
#include <stdx/sys/device.h>
#include <stdx/async/task.h>
int main()
{
	stdx::task<void> t = stdx::async<void>([]() 
	{
		std::cout << "1";
		return;
	});
	t.then([](stdx::task_result<void>) 
	{
		std::cout << "2";
	});
	try
	{
		t.get();
	}
	catch (const std::exception&)
	{
		std::cout << "error";
	}
	std::cin.get();
	return 0;
}