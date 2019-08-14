#pragma once
#include <list>
#include <stdx/net/socket.h>

namespace stdx
{
	struct client_connect_event
	{
		stdx::socket client;	//客户端
		bool allow;				//是否允许接入

		client_connect_event(stdx::socket s)
			:client(s)
			,allow(false)
		{}

		client_connect_event(const client_connect_event &other)
			:client(other.client)
			,allow(other.allow)
		{}

		~client_connect_event() = default;

		client_connect_event &operator=(const client_connect_event &other)
		{
			client = other.client;
			allow = other.allow;
			return *this;
		}

		bool operator==(const client_connect_event &other) const
		{
			return (client == other.client)&&(allow == other.allow);
		}
	};

	enum class parser_process
	{
		init,		//初始化
		wait,		//指示未完成
		complete,	//指示已完成
		overload	//指示接受数据过多
	};

	struct parser_model
	{
		stdx::buffer buffer;	//缓存区
		size_t pos;				//分割点

		parser_model(const stdx::buffer &buf)
			,buffer(buf)
			,pos(0)
		{}

		parser_model(const parser_model &other)
			,buffer(other.buffer)
			,pos(other.pos)
		{}

		parser_model(parser_model &&other)
			,buffer(other.buffer)
			,pos(std::move(other.pos))
		{}

		~parser_model() = default;

		parser_model &operator=(const parser_model &other)
		{
			buffer = other.buffer;
			pos = other.pos;
			return *this;
		}

		bool operator==(const parser_model &other) const
		{
			return (buffer == other.buffer) && (pos == other.pos);
		}
	};

	struct package_arrivals_event
	{
		stdx::socket client;	//客户端
		stdx::buffer buffer;	//完整的包缓存区

		package_arrivals_event(stdx::socket c,stdx::buffer buf)
			:client(c)
			,buffer(buf)
		{}

		package_arrivals_event(const package_arrivals_event &other)
			:client(other.client)
			,buffer(other.buffer)
		{}

		~package_arrivals_event() = default;

		package_arrivals_event &operator=(const package_arrivals_event &other)
		{
			client = other.client;
			buffer = other.buffer;
			return *this;
		}

		bool operator==(const package_arrivals_event &other) const
		{
			return (client == other.client) && (buffer == other.buffer);
		}
	};

	//接受连接处理器
	interface_class accept_handler
	{
	public:
		virtual ~accept_handler() = default;
		virtual void handle(stdx::client_connect_event &) = 0;
	};

	//数据解析器
	interface_class parser
	{
	public:
		virtual ~parser() = default;
		virtual void handle(stdx::parser_model &)=0;
		virtual stdx::parser_process process() = 0;
		virtual stdx::buffer get_buffer();
	protected:
	};

	//数据处理器
	interface_class package_handler
	{
	public:
		virtual ~package_handler() = default;
		virtual void handle(stdx::package_arrivals_event &) = 0;
	};

	using accept_handler_ptr = std::shared_ptr<accept_handler>;

	using parser_ptr = std::shared_ptr<parser>;

	using package_handler_ptr = std::shared_ptr<package_handler>;

	struct client_model
	{
		stdx::socket sock;	//客户端
		parser_ptr parser;	//解析器
		client_model(stdx::socket s)
			:sock(s)
			,parser(nullptr)
		{}

		client_model(const client_model &other)
			:sock(other.sock)
			,parser(other.parser)
		{}

		client_model(client_model &&other)
			:sock(other.sock)
			,parser(std::move(other.parser))
		{}

		~client_model() = default;

		client_model &operator=(const client_model &other)
		{
			sock = other.sock;
			parser = other.parser;
			return *this;
		}

		bool operator==(const client_model &other) const
		{
			return (sock == other.sock);
		}
	};

	interface_class client_builder
	{
	public:
		virtual ~client_builder() = default;
		virtual client_model build(stdx::socket) = 0;
	};

	using client_builder_ptr = std::shared_ptr<client_builder>;

	struct client_manager
	{
	public:

		client_manager() = default;
		virtual ~client_manager() = default;
		delete_copy(client_manager);

		virtual std::list<client_model>::iterator begin()
		{
			return m_clients.begin();
		}

		virtual std::list<client_model>::const_iterator begin() const
		{
			return m_clients.begin();
		}

		virtual std::list<client_model>::iterator end()
		{
			return m_clients.end();
		}

		virtual std::list<client_model>::const_iterator end() const
		{
			return m_clients.end();
		}

		virtual void add(const client_model &client)
		{
			m_clients.emplace_back(std::move(client));
		}

		virtual void remove(const client_model &other)
		{
			m_clients.remove(other);
		}

		virtual std::list<client_model>::iterator find(const client_model &client)
		{
			return std::find(begin(), end(), client);
		}

	private:
		std::list<client_model> m_clients;
	};

	using client_manager_ptr = std::shared_ptr<client_manager>;

	class _TcpServer
	{
	public:
		_TcpServer(stdx::network_io_service io_service)
			:m_server(stdx::open_tcpsocket(io_service))
			,m_accept_handler(nullptr)
			,m_client_builder(nullptr)
			, m_client_manager(std::make_shared<client_manager>())
			,m_package_handler(nullptr)
			,m_err_handler()
		{}
		delete_copy(_TcpServer);
		~_TcpServer()=default;

		void bind(const uint_16 &port);

		void bind(cstring ip, const uint_16 &port);

		void set_accept_handler(accept_handler_ptr ptr);

		void set_client_builder(client_builder_ptr ptr);

		void set_client_manager(client_manager_ptr ptr);

		void set_package_handler(package_handler_ptr ptr);

		void set_err_handler(std::function<void(std::exception_ptr)> &&err_handler)
		{
			m_err_handler = err_handler;
		}

		void run();

	private:
		stdx::socket m_server;
		accept_handler_ptr m_accept_handler;
		client_builder_ptr m_client_builder;
		client_manager_ptr m_client_manager;
		package_handler_ptr m_package_handler;
		std::function<void(std::exception_ptr)> m_err_handler;

		void register_listen(client_model &&client);

		void handle_data(client_model &client,stdx::network_recv_event &ev);
	};
}