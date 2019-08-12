#pragma once
#include <list>
#include <stdx/net/socket.h>

namespace stdx
{
	struct client_connect_event
	{
		stdx::socket client;	//客户端
		bool allow;				//是否允许接入
	};

	enum class parser_process
	{
		wait,		//指示未完成
		complete,	//指示已完成
		overload	//指示接受数据过多
	};

	struct parser_model
	{
		parser_process process;	//解析进度
		stdx::buffer buffer;	//缓存区
		size_t pos;				//分割点
	};

	struct package_arrivals_event
	{
		stdx::socket client;	//客户端
		stdx::buffer buffer;	//完整的包缓存区
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
	};

	//数据处理器
	interface_class package_handler
	{
	public:
		virtual ~package_handler() = default;
		virtual void handle(stdx::package_arrivals_event &) = 0;
	};

	using accept_handler_ptr = std::shared_ptr<accept_handler>;

	using parser_ptr = std::shared_ptr<parser_ptr>;

	using package_handler_ptr = std::shared_ptr<package_handler>;

	struct client_model
	{
		stdx::socket sock;	//客户端
		parser_ptr parser;	//解析器
	};

	interface_class client_builder
	{
	public:
		virtual ~client_builder() = default;
		virtual client_model build(stdx::socket) = 0;
	};

	using client_builder_ptr = std::shared_ptr<client_builder>;

	class _TcpServer
	{
	public:
		_TcpServer(stdx::network_io_service io_service)
			:m_server(stdx::open_tcpsocket(io_service))
			,m_accept_handler(nullptr)
			,m_client_builder(nullptr)
			,m_package_handler(nullptr)
		{}
		delete_copy(_TcpServer);
		~_TcpServer()=default;

		void bind(const uint_16 &port);

		void bind(cstring ip, const uint_16 &port);

		void set_accept_handler(accept_handler_ptr ptr);

		void set_client_builder(client_builder_ptr ptr);

		void set_package_handler(package_handler_ptr ptr);

		void run();

	private:
		stdx::socket m_server;
		accept_handler_ptr m_accept_handler;
		client_builder_ptr m_client_builder;
		package_handler_ptr m_package_handler;
	};
}