#pragma once
#include <stdx/net/socket.h>

namespace stdx
{
	enum class parse_process
	{
		complete,
		over,
		less,
		error
	};

	template<typename _Request>
	class _Package
	{
	public:
		template<typename ..._Args>
		_Package(const parse_process &process,_Args &&...args)
			:m_process(process)
			,m_req(args...)
		{}
		~_Package()=default;
		const parse_process &get_process() const
		{
			return m_process;
		}

		_Request &get_request()
		{
			return m_req;
		}
	private:
		parse_process m_process;
		_Request m_req;
	};

	template<typename _Request>
	class package
	{
		using impl_t = std::shared_ptr<_Package<_Request>>;
	public:
		template<typename ..._Args>
		package(const parse_process &process, _Args &&...args)
			:m_impl(std::make_shared<_Package<_Request>>(process,args...))
		{}
		package(const package<_Request> &other)
			:m_impl(other.m_impl)
		{}
		~package() = default;
		package<_Request> &operator=(const package<_Request> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		bool operator==(const package<_Request> &other) const
		{
			return m_impl == other.m_impl;
		}
		const parse_process &get_process() const
		{
			return m_impl->get_process();
		}
		_Request &get_request()
		{
			return m_impl->get_request();
		}
	private:
		impl_t m_impl;
	};

	template<typename _Request>
	interface_class parser
	{
	public:
		parser() = default;
		virtual ~parser() = default;
		//正常解析
		virtual package<_Request> parse(stdx::buffer) = 0;
		//强制完成
		virtual package<_Request> complete() = 0;
	};

	template<typename _Request>
	using parser_ptr = std::shared_ptr<parser<_Request>>;
}