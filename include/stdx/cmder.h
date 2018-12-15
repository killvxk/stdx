#pragma once
#include <string>
#include <stdx/converter.h>
namespace stdx
{
	struct cmder
	{
		static void execute(const std::string &str)
		{
			system(str.c_str());
		}

#ifdef WIN32
		static void open_regedit()
		{
			execute("regedit");
		}

		static void open_cmd()
		{
			execute("cmd");
		}

		static void log_off()
		{
			execute("logoff");
		}

		static void open_explorer()
		{
			execute("explorer");
		}

		static void show_using_memory()
		{
			execute("mem.exe");
		}

		static void show_windows_version()
		{
			execute("winver");
		}

		static void shutdown(int &secound)
		{
			if (secound)
			{
				execute("showdown /s /t " + stdx::converter::to_string(secound));
			}
			else 
			{
				execute("showdown /s /t 0");
			}
		}

		static void reboot(int &secound)
		{
			if (secound)
			{
				execute("shutdown /r /t "+ stdx::converter::to_string(secound));
			}
			else
			{
				execute("shutdown /r /t 0");
			}
		}

		static void pause()
		{
			execute("pause");
		}
#endif // WIN32

	};
}