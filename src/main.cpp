#include <iostream>
#include <stdx/cmder.h>
#include <stdx/sys/device.h>
#include <stdx/async/task.h>
#include <stdx/traits/is_same.h>
#include <stdx/traits/type_list.h>
#include <stdx/tuple.h>

int main()
{
	stdx::function<void,int> action([](int i) 
	{
		std::cout << 1 <<std::endl;
	});
	std::queue<stdx::function<void, int>> queue;
	queue.push(action);
	action = queue.front();
	queue.pop();
	action(1);
	std::cin.get();
	return 0;
}