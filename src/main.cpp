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
	t.then([](stdx::task_result<void> r) 
	{
		try
		{
			r.get();
			std::cout << "2";
		}
		catch (const std::exception&)
		{

		}

	});
	std::cin.get();
	return 0;
}