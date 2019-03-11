#pragma once
//在没有定义任何平台符号的情况下
//依照编译器判断平台

#ifndef WIN32
#ifndef LINUX

#ifdef __WIN32
#define WIN32
#endif // __WIN32

#ifdef __linux__
#define LINUX
#endif // __linux__

#endif // !LINUX
#endif // !WIN32
