#include "ziran/mysql_tool.h"
#include "ziran/web_tool.h"
#include "ziran/memory/object_pool.h"
#include "ziran/memory/pool_ptr.h"
#include "ziran/async/spin_lock.h"
int main()
{
	ziran::memory::object_pool pool;
	auto ptr = pool.make_pool_ptr<double>(pool.make_object<double>(1.23));
	ziran::async::spin_lock<20> lock;
	lock.enter();
	std::cout << "1" << std::endl;
	std::thread thread([ptr,&lock]() 
	{
		lock.enter();
		std::cout << "2" << std::endl;
		lock.exit();
	});
	thread.detach();
	lock.exit();
	system("pause");
	return 0;
}