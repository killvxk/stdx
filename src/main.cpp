#include <iostream>
#include <stdx/cmder.h>
#include <stdx/sys/device.h>
#include <stdx/async/task.h>
#include <stdx/traits/same_type.h>
#include <stdx/traits/type_list.h>
#include <stdx/tuple.h>
int main()
{
	stdx::task<void> t = stdx::async<void>([]() 
	{
		return;
	});
	t.then([](stdx::task_result<void> r) 
	{
		try
		{
			r.get();
			stdx::tuple<int, double, char> t(1,1,1);
			t.set<0>(10);
			t.set<1>(10);
			t.set<2>('a');
			std::cout << t.get<0>() << t.get<1>()<<t.get<2>();
		}
		catch (const std::exception&)
		{
			
		}

	});
	std::cin.get();
	return 0;
}