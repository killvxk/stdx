#pragma once
#include <bitset>

namespace ziran
{
	namespace net
	{
		//UDP首部
		struct udp_header
		{
			//源端口
			unsigned short src;

			//目的端口
			unsigned short des;

			//包长度
			unsigned short length;

			//校验和
			unsigned short checksum;
		};
	}
}