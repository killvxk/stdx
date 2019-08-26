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

	//一个必须的包解析器
	template<typename _Payload>
	interface_class basic_parser
	{
	public:
		basic_parser() = default;

		virtual ~basic_parser() = default;

		virtual parse_process parse(stdx::buffer buffer,const size_t &size) = 0;

		virtual package<_Payload> complete() = 0;

		virtual package<_Payload> get_package() = 0;

		virtual size_t get_packages_count() = 0;
	};

	template<typename _Payload>
	using parser_ptr = std::shared_ptr<basic_parser<_Payload>>;


	//一个可选的Client管理器
	class basic_client_manager
	{
	public:

		using client_collection_t = std::list<stdx::socket>;

		basic_client_manager();

		virtual ~basic_client_manager() noexcept;

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

		virtual void clear();
	protected:
		client_collection_t m_clients;
	};

	using client_manager_ptr = std::shared_ptr<basic_client_manager>;

	class client_manager
	{
		using impl_t = std::shared_ptr<basic_client_manager>;
		using client_collection_t = typename basic_client_manager::client_collection_t;
	public:
		client_manager(const client_manager_ptr &manager)
			:m_impl(manager)
		{}

		client_manager(const client_manager &other)
			:m_impl(other.m_impl)
		{}

		~client_manager() = default;

		client_manager &operator=(const client_manager &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		bool operator==(const client_manager &other) const
		{
			return m_impl == other.m_impl;
		}

		void add_client(const stdx::socket &client)
		{
			m_impl->add_client(client);
		}

		void del_client(const stdx::socket &client)
		{
			m_impl->del_client(client);
		}

		client_collection_t::iterator begin()
		{
			return m_impl->begin();
		}

		client_collection_t::iterator end()
		{
			return m_impl->end();
		}

		client_collection_t::const_iterator cbegin() const
		{
			return m_impl->cbegin();
		}

		client_collection_t::const_iterator cend() const
		{
			return m_impl->cend();
		}

		client_collection_t::reverse_iterator rbegin()
		{
			return m_impl->rbegin();
		}

		client_collection_t::reverse_iterator rend()
		{
			return m_impl->rend();
		}

		client_collection_t::const_reverse_iterator crbegin() const
		{
			return m_impl->crbegin();
		}

		client_collection_t::const_reverse_iterator crend() const
		{
			return m_impl->crend();
		}

		client_collection_t::iterator find(const stdx::socket &client)
		{
			return m_impl->find(client);
		}

		void clear()
		{
			m_impl->clear();
		}
	private:
		impl_t m_impl;
	};

	//一个必须的事件反应器
	template<typename _Payload>
	class basic_event_reactor
	{
		using parser_map_t =  std::unordered_map<stdx::socket,stdx::parser_ptr>;
	public:

		basic_event_reactor()
			,m_parser_table()
		{}

		virtual ~basic_event_reactor()=default;

		virtual void register_client(stdx::socket &&client)
		{
			auto iterator = m_parser_table.find(client);
			auto end = m_parser_table.end();
			if (iterator != end)
			{
				return;
			}
			auto parser = make_parser();
			m_client_manager.add_client(client);
			m_parser_table->emplace(client,parser);
			client.recv_utill_error(4096, [client,this,parser](stdx::network_recv_event &&ev) mutable
			{
				//handle data
				stdx::parse_process process = parser->parse(ev.buffer,ev.size);
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
				unregiter_client(client);
			});
		}

		virtual void unregiter_client(const stdx::socket &client)
		{
			auto iterator = m_parser_table.find(client);
			auto end = m_parser_table.end();
			if (iterator != end)
			{
				m_parser_table.erase(client);
			}
		}

		virtual void clear()
		{
			m_parser_table.clear();
		}

	protected:
		parser_map_t m_parser_table;

		virtual stdx::parser_ptr make_parser() = 0;

		virtual void on_error(stdx::socket client, std::exception_ptr err) = 0;

		virtual void on_recv(stdx::socket client, stdx::package<_Payload> package) noexcept = 0;

		virtual void on_disconnect(stdx::socket client);

		virtual void on_connect(stdx::socket client);
	};

	template<typename _Payload>
	using event_reactor_ptr = std::shared_ptr<basic_event_reactor<_Payload>>;

	template<typename _Payload>
	class event_reactor
	{
		using impl_t = event_reactor_ptr<_Payload>;
	public:
		event_reactor(const event_reactor_ptr &reactor)
			:m_impl(reactor)
		{}

		event_reactor(const event_reactor &other)
			:m_impl(other.m_impl)
		{}

		~event_reactor() = default;

		event_reactor &operator=(const event_reactor &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		bool operator==(const event_reactor &other)
		{
			return m_impl == other.m_impl;
		}

		void register_client(stdx::socket &&client)
		{
			m_impl->register_client(std::move(client));
		}

		void unregiter_client(const stdx::socket &client)
		{
			m_impl->unregister_client(client);
		}

		void clear()
		{
			m_impl->clear();
		}
	private:
		impl_t m_impl;
	};

	template<typename _Payload>
	class _TcpServer
	{
	public:
		_TcpServer(const stdx::network_io_service &io_service,const event_reactor &reactor)
			:m_running(false)
			,m_server_socket(io_service)
			,m_reactor(reactor)
		{}
		~_TcpServer() noexcept
		{
			close();
		}

		void run(cstring ip,const uint_16 &port)
		{
			if (m_running)
			{
				m_server_socket.close();
			}
			else
			{
				m_running = true;
			}
			m_server_socket.init(stdx::addr_family::ip, stdx::socket_type::stream, stdx::protocol::tcp);
			stdx::network_addr addr(ip,port);
			m_server_socket.bind(addr);
			m_server_socket.listen(5);
			while (m_running)
			{
				auto client = m_server_socket.accept();
				m_reactor.register_client(client);
			}
		}

		void run(const uint_16 &port)
		{
			run("0.0.0.0", port);
		}

		stdx::task<void> start(cstring ip, const uint_16 &port)
		{
			return stdx::async([this,ip,port]()mutable 
			{
				run(ip,port);
			});
		}

		stdx::task<void> start(const uint_16 &port)
		{
			return start("0.0.0.0", port);
		}

		void close() noexcept
		{
			if (m_running)
			{
				m_running = false;
				m_reactor.clear();
				m_server_socket.close();
			}
		}
	private:
		bool m_running;
		stdx::socket m_server_socket;
		event_reactor m_reactor;
	};
}
