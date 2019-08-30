#pragma once
#include <stdx/env.h>

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

#include <stdio.h>
#include <memory>


namespace stdx
{
	interface_class basic_logger
	{
	public:
		virtual ~basic_logger() = default;
		virtual void debug(cstring str)=0;
		virtual void info(cstring str) = 0;
		virtual void warn(cstring str) = 0;
		virtual void error(cstring str) = 0;
	};
	class _Logger:public basic_logger
	{
	public:
#ifdef WIN32
		_Logger()
			:basic_logger()
			,m_stdout(GetStdHandle(STD_OUTPUT_HANDLE))
			,m_stderr(GetStdHandle(STD_ERROR_HANDLE))
		{}
#else
		_Logger()
			:basic_logger()
		{}
#endif
		~_Logger()=default;

		void debug(cstring str) override;

		void info(cstring str) override;

		void warn(cstring str) override;

		void error(cstring str) override;

	private:
#ifdef WIN32
		HANDLE m_stdout;
		HANDLE m_stderr;
#endif // WIN32
	};

	
	class logger
	{
		using impl_t = std::shared_ptr<basic_logger>;
	public:
		
		logger(impl_t impl)
			:m_impl(impl)
		{}

		logger(const logger &other)
			:m_impl(other.m_impl)
		{}

		~logger() = default;

		logger &operator=(const logger &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		void debug(cstring str)
		{
			m_impl->debug(str);
		}
		void info(cstring str)
		{
			m_impl->info(str);
		}
		void warn(cstring str)
		{
			m_impl->warn(str);
		}
		void error(cstring str)
		{
			m_impl->error(str);
		}

		bool operator==(const logger &other) const
		{
			return m_impl == other.m_impl;
		}

	private:
		impl_t m_impl;
	};

	template<typename _Logger,typename ..._Args>
	inline stdx::logger make_logger(_Args &&...args)
	{
		return stdx::logger(std::make_shared<_Logger>(args...));
	}

	extern stdx::logger make_default_logger();
}