#pragma once

namespace stdx
{
	namespace net
	{
		struct proto_type
		{
			enum
			{
#ifdef WIN32
				ip = 0,
				tcp = 6,
				udp = 17
#endif // WIN32
#ifdef UNIX
				ip = 1,
				tcp = 6,
				udp = 17
#endif //UNIX
			};
		};
	}
}