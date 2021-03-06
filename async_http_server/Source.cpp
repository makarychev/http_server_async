#include "Server.h"
#include "Logger.h"
#ifndef WIN32
#include <unistd.h>
#endif
using namespace boost::asio;
using boost::asio::ip::tcp;

#define ARG_IP		0x01
#define ARG_PORT	0x02
#define ARG_DIR		0x04
#define ARG_ALL		(ARG_IP | ARG_PORT | ARG_DIR)

int main(int argc, char** argv)
{
	std::string sIp("127.0.0.456");
	int iPort = 0;
	std::string sDir;
	unsigned char params = 0;
	for (int i = 0; i < argc; i++) // todo: getopt()
	{
		if (strcmp(*argv, "-h") == 0) {
			sIp = *(argv + 1);
			params |= ARG_IP;
		} else if (strcmp(*argv, "-p") == 0) {
			iPort = std::stoi(*(argv + 1));
			params |= ARG_PORT;
		} else if (strcmp(*argv, "-d") == 0) {
			sDir = *(argv + 1);
			params |= ARG_DIR;
		}
		argv++;
	}

	if (params != ARG_ALL)
	{
		std::cout << "Invalid arguments" << std::endl;
		return -1;
	}

#ifndef WIN32
	if (!fork())
#endif
	{
		SimpleLogger::GetInstance().WriteLog("HTTP server started");
		Server server(sIp, iPort, sDir);
		server.Start();
		SimpleLogger::GetInstance().WriteLog("HTTP server stopped");
	}	
	
	return 0;
}