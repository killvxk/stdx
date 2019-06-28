#pragma once
#include <stdx/env.h>
#include <string>
#ifdef WIN32
#include <windows.h>
namespace stdx
{
	struct message_button
	{
		enum
		{
			ok = MB_OK,
			yes_no = MB_YESNO,
			abort_retry_ignore = MB_ABORTRETRYIGNORE,
			yes_no_cancel = MB_YESNOCANCEL,
			retry_cancel = MB_RETRYCANCEL,
			ok_cancel = MB_OKCANCEL
		};
	};
	struct message_icon 
	{
		enum
		{
			exclamatton = MB_ICONEXCLAMATION,
			warnning = MB_ICONWARNING,
			question = MB_ICONQUESTION,
			stop = MB_ICONSTOP,
			error = MB_ICONERROR,
			none = NULL
		};
	};
	struct message_answer
	{
		enum 
		{
			ok =IDOK,
			cancel =IDCANCEL,
			abort = IDABORT,
			retry = IDRETRY,
			ignore = IDIGNORE,
			yes = IDYES,
			no = IDNO
		};
	};
	struct message_box
	{
		static int show(HWND window,const std::string &title,const std::string &body,uint button,uint icon)
		{
			return MessageBox(window, body.c_str(), title.c_str(), button | icon);
		}
	};
}
#endif