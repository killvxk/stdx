#include <iostream>
#include <stdx/cmder.h>
#include <stdx/sys/device.h>
#include <stdx/async/task.h>
#include <stdx/traits/same_type.h>
#include <stdx/traits/type_list.h>
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
			std::cout << stdx::type_list<int,double,char>::include<int>::value;
		}
		catch (const std::exception&)
		{

		}

	});
	std::cin.get();
	return 0;
}