﻿#pragma once
#include <stdx/env.h>
#include <memory>
#include <string>
namespace stdx
{
	//自动释放缓存区实现
	class _Buffer
	{
	public:
		_Buffer(size_t size = 4096);
			
		explicit _Buffer(size_t size, char* data);

		~_Buffer();

		char &operator[](const size_t &i) const;

		operator char*() const
		{
			return m_data;
		}

		void realloc(size_t size);

		const size_t &size() const
		{
			return m_size;
		}

		void copy_from(const _Buffer &other);

		char *to_raw();
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

		bool operator==(const buffer &other)
		{
			return m_impl == other.m_impl;
		}
		char *to_raw()
		{
			return m_impl->to_raw();
		}
	private:
		impl_t m_impl;
	};
}
#ifdef WIN32

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
			if (ol == nullptr)
			{
				return nullptr;
			}
			return CONTAINING_RECORD(ol,_IOContext, m_ol);
		}

		void post(DWORD size,_IOContext *context_ptr,OVERLAPPED *ol_ptr)
		{
			bool r = PostQueuedCompletionStatus(m_iocp, size, (ULONG_PTR)context_ptr, ol_ptr);
			if (!r)
			{
				//处理错误
				_ThrowWinError
			}
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
		void post(DWORD size, _IOContext *context_ptr, OVERLAPPED *ol_ptr)
		{
			m_impl->post(size, context_ptr, ol_ptr);
		}

		bool operator==(const iocp &other) const
		{
			return m_impl == other.m_impl;
		}

	private:
		impl_t m_impl;
	};
}
#undef _ThrowWinError
#endif

#ifdef LINUX
#include <memory>
#include <system_error>
#include <string>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <linux/aio_abi.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>
#include <queue>
#include <stdx/async/spin_lock.h>
#include <mutex>
#include <stdx/function.h>
#include <stdx/async/threadpool.h>
#define _ThrowLinuxError auto _ERROR_CODE = errno;\
						 throw std::system_error(std::error_code(_ERROR_CODE,std::system_category()),strerror(_ERROR_CODE)); \

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
		{}
		~_EPOLL()
		{
			close(m_handle);
		}

		void add_event(int fd, epoll_event *event_ptr);

		void del_event(int fd);

		void update_event(int fd, epoll_event *event_ptr);

		void wait(epoll_event *event_ptr, const int &maxevents, const int &timeout) const;
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
		void add_event(int fd, epoll_event *event_ptr)
		{
			return m_impl->add_event(fd, event_ptr);
		}
		void update_event(int fd, epoll_event *event_ptr)
		{
			return m_impl->update_event(fd, event_ptr);
		}
		void del_event(int fd)
		{
			return m_impl->del_event(fd);
		}

		void wait(epoll_event *event_ptr,const int &maxevents,const int &timeout) const
		{
			return m_impl->wait(event_ptr, maxevents, timeout);
		}

		epoll_event wait(const int &timeout) const
		{
			
			epoll_event ev;
			this->wait(&ev, 1, timeout);
			return ev;
		}
	private:
		impl_t m_impl;
	};

	inline int io_setup(unsigned nr_events, aio_context_t *ctx_idp)
	{
		return syscall(SYS_io_setup, nr_events, ctx_idp);
	}

	inline int io_destroy(aio_context_t ctx_id)
	{
		return syscall(SYS_io_destroy, ctx_id);
	}

	inline int io_submit(aio_context_t ctx_id, long nr, struct iocb **iocbpp)
	{
		return syscall(SYS_io_submit,ctx_id,nr,iocbpp );
	}

	inline int io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event *events, struct timespec *timeout)
	{
		return syscall(SYS_io_getevents, ctx_id, min_nr, nr, events, timeout);
	}

	inline int io_cancel(aio_context_t ctx_id, struct iocb *iocb, struct io_event *result)
	{
		return syscall(SYS_io_cancel, ctx_id, iocb, result);
	}
#define invalid_eventfd -1
	template<typename _Data>
	inline void aio_read(aio_context_t context,int fd,char *buf,size_t size,int_64 offset,int resfd,_Data *ptr)
	{
		iocb cbs[1],*p[1] = {&cbs[0]};
		memset(&(cbs[0]), 0,sizeof(iocb));
		(cbs[0]).aio_lio_opcode = IOCB_CMD_PREAD;
		(cbs[0]).aio_fildes = fd;
		(cbs[0]).aio_buf = (uint_64)buf;
		(cbs[0]).aio_nbytes = size;
		(cbs[0]).aio_offset = offset;
		(cbs[0]).aio_data =(uint_64)ptr;
		if (resfd != invalid_eventfd)
		{
			(cbs[0]).aio_flags = IOCB_FLAG_RESFD;
			(cbs[0]).aio_resfd = resfd;
		}
		if (io_submit(context, 1,p) != 1)
		{
			_ThrowLinuxError
		}
		return;
	}
	
	template<typename _Data>
	inline void aio_write(aio_context_t context, int fd, char *buf, size_t size, int_64 offset, int resfd, _Data *ptr)
	{
		iocb cbs[1], *p[1] = { &cbs[0] };
		memset(&(cbs[0]), 0, sizeof(iocb));
		(cbs[0]).aio_lio_opcode = IOCB_CMD_PWRITE;
		(cbs[0]).aio_fildes = fd;
		(cbs[0]).aio_buf = (uint_64)buf;
		(cbs[0]).aio_nbytes = size;
		(cbs[0]).aio_offset = offset;
		(cbs[0]).aio_data = (uint_64)ptr;
		if (resfd != invalid_eventfd)
		{
			(cbs[0]).aio_flags = IOCB_FLAG_RESFD;
			(cbs[0]).aio_resfd = resfd;
		}
		if (io_submit(context, 1, p) != 1)
		{
			_ThrowLinuxError
		}
		return;
	}

	template<typename _IOContext>
	class _AIOCP
	{
	public:
		_AIOCP(unsigned nr_events=2048)
			:m_ctxid(0)
		{
			memset(&m_ctxid, 0, sizeof(aio_context_t));
			io_setup(nr_events, &m_ctxid);
		}
		~_AIOCP()
		{
			io_destroy(m_ctxid);
		}

		_IOContext *get(int_64 &res)
		{
			io_event ev;
			io_getevents(m_ctxid, 1, 1,&ev,NULL);
			res = ev.res;
			return (_IOContext*)ev.data;
		}
		aio_context_t get_context() const
		{
			return m_ctxid;
		}
	private:
		aio_context_t m_ctxid;
	};
	template<typename _IOContext>
	class aiocp
	{
		using impl_t = std::shared_ptr<_AIOCP<_IOContext>>;
	public:
		aiocp(unsigned nr_events)
			:m_impl(std::make_shared<_AIOCP<_IOContext>>(nr_events))
		{}
		aiocp(const aiocp<_IOContext> &other)
			:m_impl(other.m_impl)
		{}
		aiocp(aiocp<_IOContext> &&other)
			:m_impl(std::move(other.m_impl))
		{}
		~aiocp()=default;
		aiocp &operator=(const aiocp<_IOContext> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		aio_context_t get_context() const
		{
			return m_impl->get_context();
		}
		_IOContext *get(int_64 &res)
		{
			return m_impl->get(res);
		}

		bool operator==(const aiocp &other) const
		{
			return m_impl == other.m_impl;
		}

	private:
		impl_t m_impl;
	};

	struct ev_queue
	{
		ev_queue()
			:m_lock()
			,m_existed(false)
			,m_queue()
		{}
		ev_queue(ev_queue &&other)
			:m_lock(other.m_lock)
			,m_existed(other.m_existed)
			,m_queue(std::move(other.m_queue))
		{}
		~ev_queue() = default;
		ev_queue &operator=(const ev_queue &&other)
		{
			m_lock = other.m_lock;
			m_existed = other.m_existed;
			m_queue = std::move(other.m_queue);
			return *this;
		}
		stdx::spin_lock m_lock;
		bool m_existed;
		std::queue<epoll_event> m_queue;
	};

	class _Reactor
	{
	public:
		_Reactor()
			:m_map()
			,m_poll()
		{}
		~_Reactor()=default;

		void bind(int fd);

		void unbind(int fd);

		template<typename _Finder,typename _Fn>
		void get(_Fn &&execute)
		{
			static_assert(is_arguments_type(_Fn, epoll_event*), "ths input function not be allowed");
			epoll_event ev = m_poll.wait(-1);
			auto *ev_ptr = &ev;
			int fd = _Finder::find(ev_ptr);
			std::function<void(epoll_event*,_Fn)> call = [](epoll_event *ev_ptr,_Fn execute) mutable
			{
				try
				{
					execute(ev_ptr);
				}
				catch (...)
				{
				}
			};
			stdx::threadpool::run([this](epoll_event *ev, _Fn execute, int fd, std::function<void(epoll_event*, _Fn)> call) mutable
			{
				call(ev,execute);
				loop(fd);
			}, ev_ptr,execute, fd,call);
		}

		void push(int fd, epoll_event &ev);

		void loop(int fd);
	private:
		stdx::ev_queue make()
		{
			return ev_queue();
		}
	private:
		std::unordered_map<int,ev_queue> m_map;
		stdx::epoll m_poll;
	};
	
	class reactor
	{
		using impl_t = std::shared_ptr<stdx::_Reactor>;
	public:
		reactor()
			:m_impl(std::make_shared<stdx::_Reactor>())
		{}
		reactor(const reactor &other)
			:m_impl(other.m_impl)
		{}
		~reactor()=default;
		reactor &operator=(const reactor &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		void bind(int fd)
		{
			return m_impl->bind(fd);
		}

		void unbind(int fd)
		{
			return m_impl->unbind(fd);
		}

		template<typename _Finder, typename _Fn>
		void get(_Fn &&execute)
		{
			return m_impl->get<_Finder>(std::move(execute));
		}

		void push(int fd,epoll_event &ev)
		{
			return m_impl->push(fd,ev);
		}

		bool operator==(const reactor &other) const
		{
			return m_impl == other.m_impl;
		}

	private:
		impl_t m_impl;
	};
}

#undef _ThrowLinuxError
#endif