#pragma once

namespace ziran
{
	namespace net
	{
		//伪首部
		struct pseudo_header
		{
			//源地址
			unsigned int src;

			//目的地址
			unsigned int des;

			//无意义填充
			unsigned char placeholder;

			//协议
			unsigned char protocol;

			//TCP/UDP首部长度
			unsigned short tcp_udp_header_length;
		};
	}
}