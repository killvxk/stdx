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

void stdx::_TcpServer::set_client_manager(stdx::client_manager_ptr ptr)
{
	if (ptr)
	{
		m_client_manager = ptr;
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
		stdx::client_connect_event cev(sock);
		if (!m_accept_handler)
		{
			sock.close();
			throw std::logic_error("you should set accept_handler first!");
		}
		m_accept_handler->handle(cev);
		if (!cev.allow)
		{
			sock.close();
			return;
		}
		if (!m_client_builder)
		{
			sock.close();
			throw std::logic_error("you should set client_builder first!");
		}
		stdx::client_model model = m_client_builder->build(sock);
		register_listen(std::move(model));
	}
}

void stdx::_TcpServer::register_listen(stdx::client_model && client)
{
	if (!m_client_manager)
	{
		sock.close();
		throw std::logic_error("client_manager should not be null");
	}
	m_client_manager->add(client);
	client.sock.recv_utill_error(4096, [this,client](stdx::network_recv_event ev) mutable
	{
		handle_data(client, ev);
	}, [this,client](std::exception_ptr err) mutable
	{
		m_client_manager->remove(client);
	});
}

void stdx::_TcpServer::handle_data(stdx::client_model & client, stdx::network_recv_event & ev)
{
	try
	{
		parser_ptr parser = client.parser;
		stdx::parser_model model(ev.buffer);
		parser->handle(model);
		auto process = parser->process();
		if (process == stdx::parser_process::overload)
		{
			if (m_package_handler)
			{
				m_package_handler->handle(parser->get_buffer());
			}
		}
		if (process == stdx::parser_process::complete)
		{
			if (m_package_handler)
			{
				m_package_handler->handle(parser->get_buffer());
			}
		}
	}
	catch (const std::exception&)
	{
		if (m_err_handler)
		{
			m_err_handler(std::current_exception());
		}
	}
}