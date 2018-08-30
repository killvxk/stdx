#pragma once
#include <functional>
#include <vector>
#include <memory>
#include <ziran/async/parallel.h>

namespace ziran
{
	namespace event
	{
		//ÊÂ¼şÄ£°å
		template<
			typename ...TArgs
		>
		class event
		{
		public:
			event()=default;
			~event()=default;
			event(event &&other)
				:listeners(std::move(other.listeners))
			{
			}

			event &operator=(event &&other)
			{
				listeners = std::move(other.listeners);
			}

			void register_listener(std::function<void(TArgs...)> &&listener)
			{
				listeners.push_back(listener);
			}
			void pulish(const TArgs &...args)
			{
				ziran::async::parallel::for_each<std::function<void(TArgs...)>>(listeners, [&args...](const std::function<void(TArgs...)> &listener)
				{
					listener(args...);
				});
			}
		private:
			std::vector<std::function<void(TArgs...)>> listeners;
		};
	}
}