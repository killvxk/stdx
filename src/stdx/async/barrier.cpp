#include <stdx/async/barrier.h>

stdx::_Barrier::_Barrier()
	:mutex(std::make_shared<std::mutex>())
	, notify_count(0)
	, cv(std::make_shared<std::condition_variable>())
{}

//等待通过
void stdx::_Barrier::wait()
{
	std::unique_lock<std::mutex> wait(*mutex);
	auto &n = notify_count;
	cv->wait(wait, [&n]() { return (int)n; });
	notify_count -= 1;
}

//通过
void stdx::_Barrier::pass()
{
	notify_count += 1;
	cv->notify_one();
}