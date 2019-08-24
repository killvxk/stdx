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

	enum class package_status
	{
		normal,
		error
	}

	template<typename _Payload>
	class _Package
	{
	public:
		template<typename ..._Args>
		_Package(const package_status &status,_Args &&...args)
			:m_status(status)
			,m_payload(args...)
		{}
		~_Package()=default;
		const package_status &get_status() const
		{
			return m_status;
		}

		_Payload &get_payload()
		{
			return m_req;
		}
	private:
		package_status m_status;
		_Payload m_payload;
	};

	template<typename _Payload>
	class package
	{
		using impl_t = std::shared_ptr<_Package<_Payload>>;
	public:
		template<typename ..._Args>
		package(const package_status &status, _Args &&...args)
			:m_impl(std::make_shared<_Package<_Payload>>(status,args...))
		{}
		package(const package<_Payload> &other)
			:m_impl(other.m_impl)
		{}
		~package() = default;
		package<_Payload> &operator=(const package<_Payload> &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		bool operator==(const package<_Payload> &other) const
		{
			return m_impl == other.m_impl;
		}
		const package_status &get_status() const
		{
			return m_impl->get_status();
		}
		_Payload &get_payload()
		{
			return m_impl->get_payload();
		}
	private:
		impl_t m_impl;
	};

	template<typename _Payload>
	interface_class parser
	{
	public:
		parser() = default;
		virtual ~parser() = default;
		//Õý³£½âÎö
		virtual parse_process parse(stdx::buffer) = 0;
		//Ç¿ÖÆÍê³É
		virtual package<_Payload> complete() = 0;
		virtual package<_Payload> get_package() = 0;
	};

	template<typename _Request>
	using parser_ptr = std::shared_ptr<parser<_Request>>;
}
