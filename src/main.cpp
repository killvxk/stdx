#include <iostream>
#include <stdx/cmder.h>
#include <stdx/sys/device.h>
#include <stdx/async/task.h>
#include <stdx/traits/same_type.h>
#include <stdx/traits/type_list.h>
#include <stdx/tuple.h>

class a;
int main()
{
	auto t = stdx::async<stdx::tuple<int, double, char>>([]()
	{
		stdx::tuple<int, double, char> t(10, 10, 'a');
		return t;
	}).then([](stdx::task_result<stdx::tuple<int, double, char>> r)
	{
		try
		{
			auto t = r.get();
			
			std::cout << t.get<0>() << t.get<1>()<<t.get<2>();
		}
		catch (const std::exception&)
		{
			
		}

	}).then<stdx::task<void>>([](stdx::task_result<void> t) 
	{
		return stdx::async<void>([]() {});
	}).then([](stdx::task_result<void>)
	{

	});
	//auto t = stdx::async<int>([]() 
	//{
	//	return 1;
	//}).then([](stdx::task_result<int> r) 
	//{
	//	auto i = r.get();
	//});
	std::cin.get();
	return 0;
}