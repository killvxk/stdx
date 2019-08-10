#include <stdx/logger.h>
#include <string>

#ifdef WIN32

void stdx::_Logger::debug(cstring str)
{
	SetConsoleTextAttribute(m_stderr, FOREGROUND_INTENSITY);
	std::string string = "[DEBUG]";
	string.append(str);
	string.append("\n");
	puts(string.c_str());
	SetConsoleTextAttribute(m_stderr, 0x07);
}

void stdx::_Logger::info(cstring str)
{
	SetConsoleTextAttribute(m_stderr, 10);
	std::string string = "[INFO]";
	string.append(str);
	string.append("\n");
	puts(string.c_str());
	SetConsoleTextAttribute(m_stderr, 0x07);
}

void stdx::_Logger::warn(cstring str)
{
	SetConsoleTextAttribute(m_stderr, 14);
	std::string string = "[WARN]";
	string.append(str);
	string.append("\n");
	puts(string.c_str());
	SetConsoleTextAttribute(m_stderr, 0x07);
}

void stdx::_Logger::error(cstring str)
{
	SetConsoleTextAttribute(m_stderr, 12|FOREGROUND_INTENSITY);
	std::string string = "[ERROR]";
	string.append(str);
	string.append("\n");
	fputs(string.c_str(),stderr);
	SetConsoleTextAttribute(m_stderr,0x07);
}
#else
void stdx::_Logger::debug(cstring str)
{
	std::string string = "\033[1m\033[40;37m[DEBUG]";
	string.append(str);
	string.append("\033[39;49;0m");
	string.append("\n");
	puts(string.c_str());
}
void stdx::_Logger::info(cstring str)
{ 
	std::string string = "\033[1m\033[40;32m[INFO]";
	string.append(str);
	string.append("\033[39;49;0m");
	string.append("\n");
	puts(string.c_str());
}

void stdx::_Logger::warn(cstring str)
{
	std::string string = "\033[1m\033[40;33m[WARN]";
	string.append(str);
	string.append("\033[39;49;0m");
	string.append("\n");
	puts(string.c_str());
}

void stdx::_Logger::error(cstring str)
{
	std::string string = "\033[1m\033[40;31m[ERROR]";
	string.append(str);
	string.append("\033[39;49;0m");
	string.append("\n");
	fputs(string.c_str(), stderr);
}
#endif

stdx::logger stdx::make_default_logger()
{
	return stdx::logger(std::make_shared<stdx::_Logger>());
}