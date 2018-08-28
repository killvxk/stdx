
cmake_minimum_required (VERSION 3.8)
# 将源添加到此项目的可执行文件。
add_executable (ziran "main.cpp")
include_directories(${PROJECT_SOURCE_DIR}/include)
#设置模块路径

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# TODO: 如有需要，请添加测试并安装目标。

if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif()

#Boost
find_package(Boost REQUIRED COMPONENTS 

	system
	locale
	random
	thread
	filesystem
	chrono
	atomic
	date_time
	regex
)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(ziran PRIVATE ${Boost_LIBRARIES})

#Mysql
include_directories(${PROJECT_SOURCE_DIR}/lib/mysql-connector/include)
if(UNIX)
	target_link_libraries(ziran PRIVATE ${PROJECT_SOURCE_DIR}/lib/mysql-connector/linux/lib64/libsqlcppconn8.so ${PROJECT_SOURCE_DIR}/lib/mysql-connector/linux/lib64/libsqlcppconn.so)
else()
	target_link_libraries(ziran PRIVATE ${PROJECT_SOURCE_DIR}/lib/mysql-connector/windows/lib64/vs14/libeay32.lib ${PROJECT_SOURCE_DIR}/lib/mysql-connector/windows/lib64/vs14/mysqlcppconn8.lib ${PROJECT_SOURCE_DIR}/lib/mysql-connector/windows/lib64/vs14/ssleay32.lib ${PROJECT_SOURCE_DIR}/lib/mysql-connector/windows/lib64/vs14/mysqlcppconn.lib)
endif()
if(UNIX)
	#OpenSSL
	find_package(OpenSSL REQUIRED COMPONENTS
		SSL
		Crypto
	)
	target_link_libraries(ziran  PRIVATE ${OPENSSL_LIBRARIES})
	#pthread
	find_package(Threads REQUIRED)
	if(THREADS_HAVE_PTHREAD_ARG)
  		set_property(TARGET ziran  PROPERTY COMPILE_OPTIONS "-pthread")
  		set_property(TARGET ziran  PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
	endif()
	if(CMAKE_THREAD_LIBS_INIT)
 		 target_link_libraries(ziran  PRIVATE "${CMAKE_THREAD_LIBS_INIT}")
	endif()
endif()