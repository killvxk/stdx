#pragma once
#include<exception>
#include <string>

namespace ziran
{
	class fail_exception:public std::exception
	{
	public:
		fail_exception()
			:exception()
		{}
		fail_exception(const std::string &str)
			:exception(str.c_str())
		{}
		~fail_exception()=default;

	private:
	};
}