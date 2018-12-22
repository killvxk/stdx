#include <iostream>
#include <stdx/cmder.h>
#include <stdx/sys/device.h>
#include <stdx/async/task.h>
int main()
{/*
	TASK<void> t = RUN_TASK<void>([]() {});
	t->then([]() 
	{
		std::cout << "1";
	})->then([]() 
	{
		auto t =RUN_TASK<void>([]() {
			std::cout << "2";
		});
		return t;
	})
		->then([]() {
		std::cout << "3";
	});*/
	TASK<void> t = (RUN_TASK<void>([]() 
	{
		throw std::exception("error");
	})
		->then([]() 
	{
		printf("2");
	}));

	//std::cout << "使用前请先打开USB!"<<std::endl;
	//std::cout << "请输入要卸载的盘符(如: H: ):" << std::endl;
	//std::string str;
	//std::cin >> str;
	//try
	//{
	//	ziran::win::uninstall_usb(str);
	//})处有未经处理的异常: 0xC0000005: 读取位置 0x00000005 时发生访问冲突。

	//catch (const std::exception& e)
	//{
	//	std::cout << e.what()<<std::endl;
	//}
	try
	{
		t->get();
	}
	catch (const std::exception&)
	{
		std::cout << "error";
	}
	std::cin.get();
	return 0;
}