#pragma once
#include <thread>
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#ifdef UNIX
#include <pthread.h>
#endif // UNIX

namespace ziran
{
	namespace async 
	{

	}
}
class thread
{
public:
	thread()=default;
	template<typename function_type,typename ...args_type>
	thread(function_type &&func,args_type &&...args)
		:t(std::make_shared<std::thread, function_type, args_type...>(std::move(func), args...))
	{

	}
	~thread()
	{
		if (t->joinable())
		{
			t->detach();
		}
	}


#ifdef UNIX
	void hang_up()
	{

	}
#endif // UNIX
	bool joinable() const
	{
		return t->joinable();
	}

	void join()
	{
		t->join();
	}

	void detach()
	{
		t->detach();
	}

	std::thread::id get_id() const
	{
		return t->get_id();
	}
private:
	using thread_ptr = std::shared_ptr<std::thread>;
	thread_ptr t;
};