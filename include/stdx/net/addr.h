#pragma once

namespace stdx
{
	namespace net
	{
		struct addr_families
		{
			enum
			{
#ifdef WIN32
				unspec = 0,
				unix = 1,
				ip = 2,
				ipv6 = 23,
#endif // WIN32
#ifdef UNIX
				unspec = 0,
				unix = 1,
				ip = 2,
				ipv6 = 28,
#endif //UNIX

			};
		};
	}
}