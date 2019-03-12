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

#define int32 int
#define byte unsigned char
#define uint32 unsigned int
#define uint unsigned int
#ifdef WIN32
#define int64 __int64
#define uint64 unsigned __int64
#else
#define int64 long long
#define uint64 unsigned long long
#endif // WIN32
