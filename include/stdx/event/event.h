#pragma once
#include <functional>
#include <vector>
#include <memory>
#include <ziran/async/parallel.h>
#include <thread>

namespace stdx
{
	namespace event
	{
		//�¼�ģ��
		template<
			//��������
			typename ...TArgs
		>
		class event
		{
		public:
			//Ĭ�Ϲ��캯��
			event()=default;
			//��������
			~event()=default;
			//move���캯��
			event(event &&other)
				:listeners(std::move(other.listeners))
			{
			}
			//move��ֵ����
			event &operator=(event &&other)
			{
				listeners = std::move(other.listeners);
			}
			//ע�������
			void register_listener(std::function<void(TArgs...)> &&listener)
			{
				listeners.push_back(listener);
			}
			//�����¼�
			void pulish(const TArgs &...args)
			{
				std::thread_pool thread_pool([&args]()
				{
					ziran::async::parallel::for_each<std::function<void(TArgs...)>>(listeners, [&args...](const std::function<void(TArgs...)> &listener)
					{
						listener(args...);
					});
				});
				thread_pool.detach();
			}
		private:
			//����������
			std::vector<std::function<void(TArgs...)>> listeners;
		};
	}
}