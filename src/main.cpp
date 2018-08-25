#include "ziran/mysql_tool.h"
#include "ziran/web_tool.h"
#include "ziran/memory/object_pool.h"
#include "ziran/memory/pool_ptr.h"
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
	ziran::memory::object_pool pool;
	auto ptr = pool.make_pool_ptr<double>(pool.make_object<double>(1.23));
	std::cout << *ptr <<std::endl;
	system("pause");
	return 0;
}