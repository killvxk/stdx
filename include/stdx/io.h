#pragma once
#include <stdx/env.h>
#include <memory>
#include <string>
//#include <allocators>
namespace stdx
{
	//自动释放缓存区实现
	class _Buffer
	{
	public:
		_Buffer(size_t size = 4096)
			:m_size(size)
			, m_data((char*)std::calloc(sizeof(char), m_size))
		{
			if (m_data == nullptr)
			{
				throw std::bad_alloc();
			}
		}
		explicit _Buffer(size_t size, char* data)
			:m_size(size)
			, m_data(data)
		{}
		~_Buffer()
		{
			free(m_data);
		}
		char &operator[](const size_t &i) const
		{
			if (i >= m_size)
			{
				throw std::out_of_range("out of range");
			}
			return *(m_data + i);
		}
		operator char*() const
		{
			return m_data;
		}
		void realloc(size_t size)
		{
			if (size == 0)
			{
				throw std::invalid_argument("invalid argument: 0");
			}
			if (size > m_size)
			{
				if (std::realloc(m_data, m_size) == nullptr)
				{
					throw std::bad_alloc();
				}
				m_size = size;
			}
		}
		const size_t &size() const
		{
			return m_size;
		}

		void copy_from(const _Buffer &other)
		{
			auto new_size = other.size();
			if (new_size > m_size)
			{
				realloc(new_size);
			}
			std::memcpy(m_data, other, new_size);
		}
	private:
		size_t m_size;
		char *m_data;
	};

	//自动释放缓存区
	class buffer
	{
		using impl_t = std::shared_ptr<stdx::_Buffer>;
	public:
		buffer(size_t size = 4096)
			:m_impl(std::make_shared<_Buffer>(size))
		{}
		buffer(size_t size, char* data)
			:m_impl(std::make_shared<_Buffer>(size, data))
		{}
		buffer(const buffer &other)
			:m_impl(other.m_impl)
		{}
		buffer(buffer &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~buffer() = default;
		operator char*()
		{
			return *m_impl;
		}
		buffer &operator=(const buffer &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		char &operator[](const size_t &i)
		{
			return m_impl->operator[](i);
		}
		void realloc(size_t size)
		{
			m_impl->realloc(size);
		}
		const size_t &size() const
		{
			return m_impl->size();
		}
		void copy_from(const buffer &other)
		{
			m_impl->copy_from(*other.m_impl);
		}
	private:
		impl_t m_impl;
	};
}
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//定义抛出Windows错误宏
#define _ThrowWinError auto _ERROR_CODE = GetLastError(); \
						LPVOID _MSG;\
						if(_ERROR_CODE != ERROR_IO_PENDING) \
						{ \
							if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,_ERROR_CODE,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &_MSG,0,NULL))\
							{ \
								throw std::runtime_error((char*)_MSG);\
							}else \
							{ \
								std::string _ERROR_MSG("windows system error:");\
								_ERROR_MSG.append(std::to_string(_ERROR_CODE));\
								throw std::system_error(std::error_code(_ERROR_CODE,std::system_category()),_ERROR_MSG.c_str()); \
							} \
						}\
						

namespace stdx
{
	//IOCP封装
	template<typename _IOContext>
	class _IOCP
	{
	public:
		_IOCP()
			:m_iocp(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
		{
		}
		~_IOCP()
		{
			if (m_iocp != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_iocp);
			}
		}
		delete_copy(_IOCP<_IOContext>);
		void bind(const HANDLE &file_handle)
		{
			if (CreateIoCompletionPort(file_handle, m_iocp, (ULONG_PTR)file_handle, 0) == NULL)
			{
				_ThrowWinError
			}
		}

		template<typename _HandleType>
		void bind(const _HandleType &file_handle)
		{
			if (CreateIoCompletionPort((HANDLE)file_handle, m_iocp, file_handle, 0) == NULL)
			{
				_ThrowWinError
			}
		}

		_IOContext *get()
		{
			DWORD size = 0;
			OVERLAPPED *ol= nullptr;
			ULONG_PTR key = 0;
			bool r = GetQueuedCompletionStatus(m_iocp, &size,&key,&ol, INFINITE);
			if (!r)
			{
				//处理错误
				_ThrowWinError
			}
			return CONTAINING_RECORD(ol,_IOContext, m_ol);
		}

	private:
		HANDLE m_iocp;
	};

	//IOCP引用封装
	template<typename _IOContext>
	class iocp
	{
		using impl_t = std::shared_ptr<stdx::_IOCP<_IOContext>>;
	public:
		iocp()
			:m_impl(std::make_shared<stdx::_IOCP<_IOContext>>())
		{}
		iocp(const iocp<_IOContext> &other)
			:m_impl(other.m_impl)
		{}
		iocp(iocp<_IOContext> &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~iocp() = default;
		iocp<_IOContext> &operator=(const iocp<_IOContext> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		_IOContext *get()
		{
			return m_impl->get();
		}
		void bind(const HANDLE &file_handle)
		{
			m_impl->bind(file_handle);
		}
		template<typename _HandleType>
		void bind(const _HandleType &file_handle)
		{
			m_impl->bind<_HandleType>(file_handle);
		}
	private:
		impl_t m_impl;
	};
}
#undef _ThrowWinError
#endif

#ifdef LINUX
//仍未完成
#include <memory>
#include <system_error>
#include <string>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <aio.h>
#include <queue>
#include <unordered_map>
#define _ThrowLinuxError auto _ERROR_CODE = errno;
						 throw std::system_error(std::error_code(_ERROR_CODE,std::system_category()),strerr(_ERROR_CODE)); \

namespace stdx
{
	struct epoll_events
	{
		enum
		{
			in = EPOLLIN,
			out = EPOLLOUT,
			err = EPOLLERR,
			hup = EPOLLHUP,
			et = EPOLLET,
			once = EPOLLONESHOT
		};
	};
	class _EPOLL
	{
	public:
		_EPOLL()
			:m_handle(epoll_create1(0))
		{
			if (m_handle == -1)
			{
				_ThrowLinuxError
			}
		}
		~_EPOLL() = default;
		void add_event(int fd, const uint32 &events)
		{
			epoll_event e;
			e.events = events;
			e.data.fd = fd;
			if (epoll_ctl(m_handle, EPOLL_CTL_ADD, fd, &e) == -1)
			{
				_ThrowLinuxError
			}
		}
		void del_event(int fd)
		{
			epoll_event e;
			e.data.fd = fd;
			if (epoll_ctl(m_handle, EPOLL_CTL_DEL, fd, &e) == -1)
			{
				_ThrowLinuxError
			}
		}

		void wait(epoll_event *event_ptr, int maxevents, int timeout) const
		{
			if (epoll_wait(m_handle, event_ptr, maxevents, timeout) == -1)
			{
				_ThrowLinuxError
			}
		}
	private:
		int m_handle;
	};
	class epoll
	{
		using impl_t = std::shared_ptr<_EPOLL>;
	public:
		epoll()
			:m_impl(std::make_shared<_EPOLL>())
		{}
		epoll(const epoll &other)
			:m_impl(other.m_impl)
		{}
		~epoll() = default;
		epoll &operator=(const epoll &other)
		{
			m_impl = other.m_impl;
		}
		void add_event(int fd, const uint32 &events)
		{
			m_impl->add_event(fd, events);
		}

		void del_event(int fd)
		{
			m_impl->del_event(fd);
		}

		void wait(epoll_event *event_ptr, int maxevents, int timeout) const
		{
			m_impl->wait(event_ptr, maxevents, timeout);
		}

		epoll_event wait(int timeout) const
		{
			epoll_event ev;
			this->wait(&ev, 1, timeout);
			return ev;
		}
	private:
		impl_t m_impl;
	};
	using ready_bit = std::atomic_int;

	template<typename _IOContext>
	struct context_model
	{
		context_model()
			:m_ready_bit(0)
			, m_contexts()
		{}
		context_model(context_model &&other)
			:m_ready_bit((int)other.m_ready_bit)
			, m_contexts(std::move(other.m_contexts))
		{}
		~context_model() = default;

		ready_bit m_ready_bit;
		std::queue<_IOContext*> m_contexts;

		bool empty()
		{
			return m_contexts.empty();
		}
	};

	union  key_builder
	{
		struct
		{
			int32_t fd;
			int32_t ev;
		};
		int64_t value;
	};

	template<typename _IOContext>
	class _IOCP
	{
		using key_t = int64_t;
	public:
		_IOCP()
			:m_epoll()
			, m_map()
		{}
		~_IOCP() = default;

		template<typename _Type>
		void bind(const _Type &handle)
		{
			key_builder key;
			key.fd = (int)handle;
			key.ev = stdx::epoll_events::in;
			m_map.emplace(key.value, std::move(context_model<_IOContext>()));
			m_epoll.add_event(key.fd, stdx::epoll_events::in | stdx::epoll_events::et);
		}

		_IOContext *get()
		{
			//循环wait直到context可用
			while (true)
			{
				auto ev = m_epoll.wait(-1);
				key_builder key;
				key.fd = ev.data.fd;
				if (ev.events & stdx::epoll_events::in)
				{
					key.ev = stdx::epoll_events::in;
					context_model<_IOContext> &model = m_map[key.value];
					int bit = model.m_ready_bit.load();
					if (!model.empty())
					{
						auto context(std::move(model.m_contexts.front()));
						model.m_contexts.pop();
						model.m_ready_bit.store(bit);
						return context;
					}
					else
					{
						bit += 1;
						model.m_ready_bit.store(bit);
					}
				}
				else
				{

				}
			}
		}

		void push_read(int fd, int ev, _IOContext *context)
		{
			key_builder key;
			key.fd = fd;
			key.ev = ev;
			context_model<_IOContext> &model = m_map[key.value];
			int bit = model.m_ready_bit.load();
			if (!bit)
			{
				model.m_contexts.emplace(context);
				model.m_ready_bit.store(bit);
			}
			else
			{
				//执行callback
				model.m_ready_bit -= 1;
				model.m_ready_bit.store(bit);
				(*context)();
			}
		}
	private:
		stdx::epoll m_epoll;
		std::unordered_map<key_t, context_model<_IOContext>> m_map;
	};
}
#endif