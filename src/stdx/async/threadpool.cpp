#include <stdx/async/threadpool.h>

const stdx::threadpool::impl_t stdx::threadpool::m_impl = std::make_shared <stdx::_Threadpool>();

//���캯��

stdx::_Threadpool::_Threadpool() noexcept
	:m_free_count(std::make_shared<uint32>())
	, m_count_lock()
	, m_alive(std::make_shared<bool>(true))
	, m_task_queue(std::make_shared<std::queue<runable_ptr>>())
	, m_barrier()
	, m_lock()
{
	//��ʼ���̳߳�
	init_threads();
}

//��������

stdx::_Threadpool::~_Threadpool() noexcept
{
	//��ֹʱ����״̬
	*m_alive = false;
}

//����߳�

void stdx::_Threadpool::add_thread() noexcept
{
	//�����߳�
	std::thread t([](std::shared_ptr<std::queue<runable_ptr>> tasks, stdx::barrier barrier, stdx::spin_lock lock, std::shared_ptr<uint32> count, stdx::spin_lock count_lock, std::shared_ptr<bool> alive)
	{
		//������
		while (*alive)
		{
			//�ȴ�֪ͨ
			if (!barrier.wait_for(std::chrono::minutes(10)))
			{
				//���10���Ӻ�δ֪ͨ
				//�˳��߳�
				count_lock.lock();
				*count -= 1;
				count_lock.unlock();
				return;
			}
			if (!(tasks->empty()))
			{
				//��������б�Ϊ��
				//��ȥһ������
				*count -= 1;
				//����������
				lock.lock();
				//��ȡ����
				runable_ptr t(std::move(tasks->front()));
				//��queue��pop
				tasks->pop();
				//����
				lock.unlock();
				//ִ������
				try
				{
					if (t)
					{
						t->run();
					}
				}
				catch (const std::exception &e)
				{
					//���Գ��ֵĴ���
				}
				//��ɻ���ֹ��
				//��Ӽ���
				count_lock.lock();
				*count += 1;
				count_lock.unlock();
			}
			else
			{
				continue;
			}
		}
	}, m_task_queue, m_barrier, m_lock, m_free_count, m_count_lock, m_alive);
	//�����߳�
	t.detach();
}

//��ʼ���̳߳�

void stdx::_Threadpool::init_threads() noexcept
{
	unsigned int cores = (cpu_cores()) << 2;
	*m_free_count += cores;
	for (unsigned int i = 0; i < cores; i++)
	{
		add_thread();
	}
}
