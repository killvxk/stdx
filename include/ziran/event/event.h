#pragma once
#include <functional>
#include <vector>
#include <memory>
#include <ziran/async/parallel.h>

namespace ziran
{
	namespace event
	{
		//事件模板
		template<
			//参数类型
			typename ...TArgs
		>
		class event
		{
		public:
			//默认构造函数
			event()=default;
			//析构函数
			~event()=default;
			//move构造函数
			event(event &&other)
				:listeners(std::move(other.listeners))
			{
			}
			//move赋值函数
			event &operator=(event &&other)
			{
				listeners = std::move(other.listeners);
			}
			//注册监听器
			void register_listener(std::function<void(TArgs...)> &&listener)
			{
				listeners.push_back(listener);
			}
			//发布事件
			void pulish(const TArgs &...args)
			{
				ziran::async::parallel::for_each<std::function<void(TArgs...)>>(listeners, [&args...](const std::function<void(TArgs...)> &listener)
				{
					listener(args...);
				});
			}
		private:
			//监听器容器
			std::vector<std::function<void(TArgs...)>> listeners;
		};
	}
}