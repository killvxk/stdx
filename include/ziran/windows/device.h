#pragma once
#ifdef  WIN32
#include <Windows.h>
#include <cfgmgr32.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <exception>
#include <ziran/exception.h>
template<typename T>
using ptr = std::shared_ptr<T>;
namespace ziran
{
	namespace win
	{
		void uninstall_usb(const std::string &str)
		{
			std::string t("\\\\.\\");
			t = t +str;
			HANDLE device = CreateFile(t.c_str(), GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
			if (device == INVALID_HANDLE_VALUE)
			{
				throw std::invalid_argument("ÎÞÐ§µÄÅÌ·û");
			}
			DWORD junk;
			if (!DeviceIoControl(device,FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &junk, (LPOVERLAPPED)NULL))
			{
				throw ziran::fail_exception("Ð¶ÔØUSBÊ§°Ü");
			}
			junk = NULL;
			if (!DeviceIoControl(device, IOCTL_STORAGE_EJECT_MEDIA,NULL,0,NULL,0,&junk,(LPOVERLAPPED)NULL))
			{
				throw ziran::fail_exception("Ð¶ÔØUSBÊ§°Ü");
			}
			CloseHandle(device);
		}
		class device
		{
		public :
			
		};
	}
}
#endif
