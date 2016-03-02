#pragma once
#include <fstream>      // std::fstream
#include <string>

class SimpleLogger
{
public:
	SimpleLogger(std::string sFilePath) : m_sFilePath(sFilePath)
	{
		m_fs.open(sFilePath.c_str(), std::fstream::out | std::fstream::app);
	}
	~SimpleLogger()
	{
		if (m_fs.is_open())
			m_fs.close();
	}

	static SimpleLogger& GetInstance()
	{
#ifdef WIN32
		static SimpleLogger m_pLogger("http_server.log");
#elif
		static SimpleLogger m_pLogger("//home//box//http_server.log");
#endif
		return m_pLogger;
	}

	void WriteLog(std::string sMessage)
	{
		m_fs << sMessage << std::endl;
	}

private:
	std::string		m_sFilePath;
	std::fstream	m_fs;

};