
cmake_minimum_required (VERSION 3.8)
# 将源添加到此项目的可执行文件。
add_executable (ziran "main.cpp")
include_directories(${PROJECT_SOURCE_DIR}/include)
#设置模块路径

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# TODO: 如有需要，请添加测试并安装目标。

if(WIN32)
	#add_definitions("-DUSE_WIN32")
endif()

if(UNIX)
	#add_definitions("-DUSE_UNIX")
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