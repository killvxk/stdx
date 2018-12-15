#pragma once

namespace stdx
{
	namespace net
	{
		//IP包首部
		struct ip_header
		{
			//版本和首部长度
			unsigned char version_and_header_length;;
			//TOS
			unsigned char tos;
			//总长度
			unsigned short total_length;
			//标识
			unsigned short id;
			//标致
			unsigned short flag_and_offset;
			//生存时间
			unsigned char ttl;
			//协议
			unsigned char protocol;
			//校验和
			unsigned short checksum;
			//源地址
			unsigned int src;
			//目的地址
			unsigned int des;
		};
	}
}