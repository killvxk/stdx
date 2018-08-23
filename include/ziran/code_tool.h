#pragma once
#include <boost/locale.hpp>

//约定:MYSQL使用UTF8字符集
//约定:网页与表单使用UTF8字符集
//约定:Windows使用GBK编码
//定义宏便于UTF8转换GBK
#define UTF_TO_GBK(str) boost::locale::conv::between(str,"gbk","utf8")
#ifdef WIN32 
//WIN32 字符集为GBK
//获取GBK字符串 直接返回
#define GBK(str) str 
//获取YTF8字符串 进行转码
#define UTF8(str) boost::locale::conv::between(str,"utf8","gbk")
//MYSQL,表单输入等转换本地编码 UTF8转GBK
#define LOCAL(str) UTF_TO_GBK(str)
#else
//Linux 字符集为UTF8
//获取UTF8字符串 直接返回
#define UTF8(str) str
//获取GBK字符串 进行转码
#define GBK(str) boost::locale::conv::between(str,"gbk","utf8")
//MYSQL,表单输入等转换本地编码 直接返回
#define LOCAL(str) str
#endif