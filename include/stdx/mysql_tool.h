#pragma once
//#ifdef ENABLE_MYSQL

//#include <stdx/encode_tool.h>
//#include <jdbc/mysql_driver.h>
//#include <jdbc/mysql_connection.h>
//namespace ziran
//{
//	namespace tools
//	{
//		namespace mysql
//		{
//			//创建MYSQL连接
//			std::shared_ptr<sql::Connection> make_mysql_connection(const std::string &host_name, const std::string &user, const std::string &pwd)
//			{
//				//获取MYSQL驱动
//				sql::mysql::MySQL_Driver *driver = sql::mysql::get_driver_instance();
//				//连接MYSQL数据库
//				auto *conn = driver->connect(host_name.c_str(), user.c_str(), pwd.c_str());
//				//创建智能指针
//				std::shared_ptr<sql::Connection> conn_ptr(conn, [](sql::Connection *conn) {
//					if (!conn->isClosed())
//					{
//						conn->close();
//					}
//				});
//				return conn_ptr;
//			}
//			//创建MYSQL连接 (重载版本)
//			std::shared_ptr<sql::Connection> make_mysql_connection(const std::string &host_name, const std::string &user, const std::string &pwd, const std::string &db_name)
//			{
//				//调用之前的版本
//				auto conn = make_mysql_connection(host_name, user, pwd);
//				//设置要使用的数据库
//				conn->setSchema(db_name.c_str());
//				//返回连接
//				return conn;
//			}
//		}
//	}
//}
//#endif // ENABLE_MYSQL