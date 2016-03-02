#pragma once
#include "HttpSession.h"
#include <boost/bind.hpp>
#include <iostream>
#include "Logger.h"

class Server : private boost::noncopyable
{
public:
	Server(std::string sIp, int iPort, std::string sDir) :
		m_acceptor(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(sIp), iPort)),
		m_new_service(HttpSession::create(m_io_service, sDir)),
		m_sDir(sDir)
	{	
		try
		{
			SimpleLogger::GetInstance().WriteLog("start server");
			/*boost::asio::ip::tcp::resolver resolver(m_io_service);
			boost::asio::ip::tcp::resolver::query query(sIp, std::to_string(iPort));
			boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
*/
			m_acceptor.async_accept(m_new_service->socket(),
				boost::bind(&Server::handle_accept, this,
				boost::asio::placeholders::error));
		}catch(boost::system::system_error &ex)
		{
			std::string err = "HTTP server creation ERROR:";
			err += ex.what();
			SimpleLogger::GetInstance().WriteLog(err);
		}
	}
	~Server(){}
	void Start()
	{
		m_io_service.run();
	}
	void Stop()
	{
		m_io_service.stop();
	}

private:
	void handle_accept(const boost::system::error_code& e)
	{
		if (!e)
		{
			SimpleLogger::GetInstance().WriteLog("new client accepted");
			m_new_service->start_handling();

			m_new_service = HttpSession::create(m_io_service, m_sDir);

			m_acceptor.async_accept(m_new_service->socket(),
				boost::bind(&Server::handle_accept, this,
				boost::asio::placeholders::error));

		}
	}	
	// Any program that uses asio need to have at least one io_service object
	boost::asio::io_service			m_io_service;
	boost::asio::ip::tcp::acceptor	m_acceptor;				/**< object, that accepts new connections */
	HttpSession::pointer			m_new_service;		/**< pointer to connection, that will proceed next */
	std::string					m_sDir;
};