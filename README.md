写给自己的STL扩展库
---
Task模板:
```c++
#include <stdx/async/task.h>
#include <iostream>
int main()
{
	auto task = stdx::async([]()
	{
		std::cout << "hello world" << std::endl; //使用stdx::async方法创建任务
	})
	.then([](stdx::task_result<void> r) //使用then方法延续任务
	{
		try
		{
			r.get(); //检查上个任务是否有异常抛出
			std::cout << "then" << std::endl; 
		}catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
	.then([](stdx::task_result<void>) //参数必须是stdx::task_result或者上个task的返回类型
	{
		return stdx::async<void>([](){});
	})
	.then([](stdx::task_result<void>) //延续返回task的task
	{
		return 0;
	})
	.then([](stdx::task_result<int> r) //延续有返回值的任务
	{
		try
		{
			return r.get(); //使用get获取结果
		}catch(const std::exception &)
		{
			return 1;
		}
	}); 
	return t.get(); //使用get获取结果
}
```
---
TypeList模板
```c++
#include <stdx/traits/type_list.h>
#include <iostream>
int main()
{
	using tl = stdx::type_list<int,double,char>;
	std::cout << tl::size << std::endl; // 使用size获取长度
	std::cout << tl::include<float>::value << std::endl; //使用include确定是否包含某个类型
	//使用 type_at和下标访问TypeList中的类型
	stdx::type_at<0,tl> a; //相当与 int a;
	stdx::type_at<1,tl> b; //相当与 double b;
	stdx::type_at<2,tl> c; //相当与 char c;
	return 0;
}
```
