#include <iostream>
#include <stdx/cmder.h>
#include <stdx/sys/device.h>
#include <stdx/async/task.h>
#include <stdx/traits/is_same.h>
#include <stdx/traits/type_list.h>
#include <stdx/tuple.h>

int main()
{
	/*auto t = stdx::async<stdx::tuple<int, double, char>>([]()
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

	}).with(stdx::async([]() 
	{
	}));*/
	auto t = stdx::async<int>([]() 
	{
		printf("1");
		return 1;
	})/*.then([](stdx::task_result<int> r) 
	{
		auto i = r.get();
	}).with(stdx::async<void>([]() {}))
		.then([](stdx::task_result<void> r) 
	{
		std::cout << "1";
	})*/;
	std::cin.get();
	return 0;
}