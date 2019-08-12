#include <stdx/net/server.h>

void stdx::_TcpServer::bind(const uint_16 &port)
{
	stdx::network_addr addr("0.0.0.0", port);
	m_server.bind(addr);
}

void stdx::_TcpServer::bind(cstring ip, const uint_16 &port)
{
	stdx::network_addr addr(ip, port);
	m_server.bind(addr);
}

void stdx::_TcpServer::set_accept_handler(stdx::accept_handler_ptr ptr)
{
	if (ptr)
	{
		m_accept_handler = ptr;
	}
}

void stdx::_TcpServer::set_client_builder(stdx::client_builder_ptr ptr)
{
	if (ptr)
	{
		m_client_builder = ptr;
	}
}

void stdx::_TcpServer::set_package_handler(stdx::package_handler_ptr ptr)
{
	if (ptr)
	{
		m_package_handler = ptr;
	}
}

void stdx::_TcpServer::run()
{
	m_server.listen(1024);
	while (true)
	{
		auto sock = m_server.accept();
		if (!m_client_builder)
		{
			sock.close();
			throw std::logic_error("you should set client_builder first!");
		}
		stdx::client_model model = m_client_builder->build(sock);
	}
}