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
		return;
	});
	t.then([](stdx::task_result<void> r) 
	{
		try
		{
			r.get();
			using tl =stdx::type_list<int, double,char>;
			std::wcout  << u8"TypeList Size: " << tl::size <<std::endl
						<< u8"First Type: "<<typeid(stdx::type_at<0,tl>).name() <<std::endl
						<< u8"Secound Type: " << typeid(stdx::type_at<1, tl>).name() <<std::endl
						<< u8"Thrid Type: " << typeid(stdx::type_at<2, tl>).name()<<std::endl;
		}
		catch (const std::exception&)
		{

		}

	});
	std::cin.get();
	return 0;
}