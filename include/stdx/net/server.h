#pragma once
#include <stdx/net/socket.h>
#include <list>
#include <unordered_map>

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
	};

	template<typename _Payload>
	class _Package
	{
	public:
		template<typename ..._Args>
		_Package(const package_status &status, _Args &&...args)
			:m_status(status)
			, m_payload(args...)
		{}
		~_Package() = default;
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
			:m_impl(std::make_shared<_Package<_Payload>>(status, args...))
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
	interface_class basic_parser
	{
	public:
		basic_parser() = default;

		virtual ~basic_parser() = default;

		virtual parse_process parse(stdx::buffer) = 0;

		virtual package<_Payload> complete() = 0;

		virtual package<_Payload> get_package() = 0;

		virtual size_t get_packages_count() = 0;
	};

	template<typename _Payload>
	using parser_ptr = std::shared_ptr<basic_parser<_Payload>>;

	class _ClientManager
	{
	public:

		using client_collection_t = std::list<stdx::socket>;

		_ClientManager();

		virtual ~_ClientManager() noexcept;

		virtual void add_client(const stdx::socket &client);

		virtual void del_client(const stdx::socket &client);

		virtual client_collection_t::iterator begin();

		virtual client_collection_t::iterator end();

		virtual client_collection_t::const_iterator cbegin() const;

		virtual client_collection_t::const_iterator cend() const;

		virtual client_collection_t::reverse_iterator rbegin();

		virtual client_collection_t::reverse_iterator rend();

		virtual client_collection_t::const_reverse_iterator crbegin() const;

		virtual client_collection_t::const_reverse_iterator crend() const;

		virtual client_collection_t::iterator find(const stdx::socket &client);
	private:
		client_collection_t m_clients;
	};

	template<typename _Payload>
	class basic_event_reactor
	{
		using parser_map =  std::unordered_map<stdx::socket,stdx::parser_ptr>;
	public:
		basic_event_reactor()=default;
		virtual ~basic_event_reactor()=default;

		virtual stdx::parser_ptr make_parser() = 0;

		virtual void on_error(stdx::socket client,std::exception_ptr err) = 0;

		virtual void on_recv(stdx::socket client,stdx::package<_Payload> package) noexcept = 0;

		virtual void register_client(stdx::socket &&client)
		{
			auto iterator = m_parser_table.find(client);
			auto end = m_parser_table.end();
			if (iterator != end)
			{
				return;
			}
			auto parser = make_parser();
			m_parser_table->emplace(client,parser);
			client.recv_utill_error(4096, [client,this,parser](stdx::network_recv_event &&ev) mutable
			{
				//handle data
				stdx::parse_process process = parser->parse(ev.buffer);
				if (process == stdx::parse_process::complete)
				{
					on_recv(client,parser->get_package());
					return;
				}
				if (process == stdx::parse_process::over)
				{
					for (size_t i = 0,count = parser->get_packages_count(); i < count; i++)
					{
						on_recv(client, parser->get_package());
					}
					return;
				}
				if (process == stdx::parse_process::error)
				{
					on_recv(client,parser->complete());
					return;
				}

			}, [client,this](std::exception_ptr err) mutable 
			{
				//handle error
				on_error(client, err);
			});
		}

	private:
		parser_map m_parser_table;
	};
}