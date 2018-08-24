#include "ziran/async.h"
#include "ziran/mysql_tool.h"
#include "ziran/web_tool.h"
#include "ziran/object_pool.h"
int main()
{
	/*auto task = ziran::tools::async::make_task([]() 
	{
		return 1 + 1;
	});
	task->then([](int r) 
	{
		return r + 1;
	})->then([](int r) 
	{
		std::cout << "½á¹ûÊÇ:" << r << std::endl;
	});
	ziran::tools::async::start_task(task);
	auto fut = task->get_future();
	fut.wait();*/
	ziran::tools::object_pool pool;
	
	int *a = pool.make_object<int>(1);
	std::cout << *a << std::endl;
	pool.destroy_object(a);
	system("pause");
	return 0;
}