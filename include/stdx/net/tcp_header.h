#pragma once

namespace stdx
{
	namespace net
	{
		//TCP首部
		struct tcp_header
		{
			//源端口
			unsigned short src;
			//目的端口
			unsigned short des;
			//序号
			unsigned int seq;
			//确认号
			unsigned int ack;
			//数据偏移
			unsigned char offset_and_res;
			//标志
			//0
			//0
			//SYN
			//ACK
			//FIN
			//URG
			//PSH
			//RST
			unsigned char res_and_flag;
			//窗口
			unsigned short window;
			//校验和
			unsigned short checksum;
			//紧急指针
			unsigned short urg_ptr;
		};
	}
}