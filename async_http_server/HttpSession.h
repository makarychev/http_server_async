#pragma once
#include <boost/asio.hpp> 
#include <boost/filesystem.hpp>
#include <boost/assign.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream> 
#include <atomic> 
#include <thread> 
#include <iostream>


class HttpSession : public boost::enable_shared_from_this<HttpSession>
{  
	struct hide_me {};	// Instead of having to make friend boost::make_shared<connection>()
	static const std::map<unsigned int, std::string> http_status_table; 
public:  
	typedef boost::shared_ptr<HttpSession> pointer;
	HttpSession(boost::asio::io_service& io_service, std::string sDir, hide_me) :
		m_io_service(io_service),
		m_sock(io_service),    
		m_request(4096),    
		m_response_status_code(200), // Assume success.    
		m_resource_size_bytes(0),
		m_sDir(sDir)
	{}
	void start_handling();
	boost::asio::ip::tcp::socket& socket() {
		return m_sock;
	}
	static pointer create(boost::asio::io_service& io_service, std::string sDir) {
		return boost::make_shared<HttpSession>(boost::ref(io_service), sDir, hide_me());
	}
private:  
	void on_request_line_received(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void on_headers_received(const boost::system::error_code& ec,    std::size_t bytes_transferred);    
	void process_request();
	void send_response();
	void on_response_sent(const boost::system::error_code& ec, std::size_t bytes_transferred);
	// Here we perform the cleanup.  
	void on_finish() 
	{    
		//delete this;  // todo: is shared_ptr deleted?
	} 
private:  
	boost::asio::io_service&			m_io_service;		/**< reference to io_service, in which work this connection */
	boost::asio::ip::tcp::socket m_sock;  
	boost::asio::streambuf m_request;  
	std::map<std::string, std::string> m_request_headers;  
	std::string m_requested_resource;
	std::unique_ptr<char[]> m_resource_buffer;    
	unsigned int m_response_status_code;  
	std::size_t m_resource_size_bytes; 
	std::string m_response_headers;  
	std::string m_response_status_line; 
	std::string m_sDir;
}; 