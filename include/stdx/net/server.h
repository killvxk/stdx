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

	class _Package
	{
	public:
		_Package(const package_status &status,const stdx::buffer &buf)
			:m_status(status)
			, m_payload(buf)
		{}
		~_Package() = default;
		const package_status &get_status() const
		{
			return m_status;
		}

		stdx::buffer &get_payload()
		{
			return m_payload;
		}
	private:
		package_status m_status;
		stdx::buffer m_payload;
	};

	class package
	{
		using impl_t = std::shared_ptr<_Package>;
	public:
		template<typename ..._Args>
		package(const package_status &status, const stdx::buffer &buf)
			:m_impl(std::make_shared<_Package>(status, buf))
		{}
		package(const package &other)
			:m_impl(other.m_impl)
		{}
		~package() = default;
		package &operator=(const package &other)
		{
			m_impl = other.m_impl;
			return *this;
		}
		bool operator==(const package &other) const
		{
			return m_impl == other.m_impl;
		}
		const package_status &get_status() const
		{
			return m_impl->get_status();
		}
		stdx::buffer &get_payload()
		{
			return m_impl->get_payload();
		}
	private:
		impl_t m_impl;
	};

	//一个必须的包解析器
	interface_class basic_parser
	{
	public:
		basic_parser() = default;

		virtual ~basic_parser() = default;

		virtual parse_process parse(stdx::buffer buf,const size_t &size) = 0;

		virtual void complete() = 0;

		virtual package get_package() = 0;

		virtual size_t get_packages_count() = 0;
	};

	using parser_ptr = std::shared_ptr<basic_parser>;

	class parser
	{
		using impl_t = parser_ptr;
	public:
		parser(const parser_ptr &parser_p)
			:m_impl(parser_p)
		{}

		parser(const parser &other)
			:m_impl(other.m_impl)
		{}

		~parser() = default;

		parser &operator=(const parser &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		bool operator==(const parser &other) const
		{
			return m_impl == other.m_impl;
		}

		parse_process parse(stdx::buffer buf, const size_t &size)
		{
			return m_impl->parse(buf, size);
		}

		package complete()
		{
			return m_impl->complete();
		}

		package get_package()
		{
			return m_impl->get_package();
		}

		size_t get_packages_count()
		{
			return m_impl->get_packages_count();
		}
	private:
		impl_t m_impl;
	};

	template<typename _Parser,typename ..._Args>
	inline stdx::parser make_parser(_Args &&...args)
	{
		return stdx::parser(std::make_shared<_Parser>(args...));
	}

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

	template<typename _ClientManager,typename ..._Args>
	inline stdx::client_manager make_client_manager(_Args &&...args)
	{
		return stdx::client_manager(std::make_shared<_ClientManager>(args...));
	}

	//一个必须的事件反应器
	class basic_event_reactor
	{
		using client_parser_map_t =  std::unordered_map<stdx::socket,stdx::parser>;
	public:

		basic_event_reactor()
			:m_parser_table()
		{}

		virtual ~basic_event_reactor()=default;

		virtual stdx::parser make_parser() = 0;

		virtual bool on_error(stdx::socket client, std::exception_ptr err) noexcept = 0;

		virtual void on_recv(stdx::socket client, stdx::package package) = 0;

		virtual void on_disconnect(stdx::socket client) = 0;

		virtual bool on_connect(stdx::socket client) = 0;

		virtual void register_client(stdx::socket &&client);

		virtual void unregister_client(const stdx::socket &client);

		virtual void clear()
		{
			m_parser_table.clear();
		}

	protected:
		client_parser_map_t m_parser_table;
	};

	using event_reactor_ptr = std::shared_ptr<basic_event_reactor>;

	class event_reactor
	{
		using impl_t = event_reactor_ptr;
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

		bool operator==(const event_reactor &other) const
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

	template<typename _EventReactor,typename ..._Args>
	inline stdx::event_reactor make_event_reactor(_Args &&...args)
	{
		return stdx::event_reactor(std::make_shared<_EventReactor>(args...));
	}

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

		void run(cstring ip, const uint_16 &port);

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

		void close() noexcept;
	private:
		bool m_running;
		stdx::socket m_server_socket;
		event_reactor m_reactor;
	};

	class tcp_server
	{
		using impl_t = std::shared_ptr<_TcpServer>;
	public:

		tcp_server(const stdx::network_io_service &io_service, const event_reactor &reactor)
			:m_impl(std::make_shared<_TcpServer>(io_service,reactor))
		{}

		tcp_server(const tcp_server &other)
			:m_impl(other.m_impl)
		{}

		~tcp_server() = default;

		tcp_server &operator=(const tcp_server &other)
		{
			m_impl = other.m_impl;
			return *this;
		}

		bool operator==(const tcp_server &other) const
		{
			return m_impl == other.m_impl;
		}

		void run(cstring ip, const uint_16 &port)
		{
			return m_impl->run(ip, port);
		}

		void run(const uint_16 &port)
		{
			return m_impl->run(port);
		}

		stdx::task<void> start(cstring ip, const uint_16 &port)
		{
			return m_impl->start(ip, port);
		}

		stdx::task<void> start(const uint_16 &port)
		{
			return m_impl->start(port);
		}

		void close() noexcept
		{
			m_impl->close();
		}
	private:
		impl_t m_impl;
	};
}
