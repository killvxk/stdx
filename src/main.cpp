#include <iostream>
#include <stdx/cmder.h>
#include <stdx/sys/device.h>
#include <stdx/async/task.h>
#include <stdx/traits/is_same.h>
#include <stdx/traits/type_list.h>
#include <stdx/tuple.h>

int main()
{
	stdx::task<void> t = stdx::async([]() 
	{
		using namespace std;
		cout << "1" << endl;
	});
	std::cin.get();
	return 0;
}