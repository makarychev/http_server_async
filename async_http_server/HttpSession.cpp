#include "HttpSession.h"
#include <boost/bind.hpp>
#include "Logger.h"
#include <string>



// since C++11
//const std::map<unsigned int, std::string>  Service::http_status_table = {  { 200, "200 OK" },  { 404, "404 Not Found" },  { 413, "413 Request Entity Too Large" },  { 500, "500 Server Error" },  { 501, "501 Not Implemented" },  { 505, "505 HTTP Version Not Supported" } }; 
const std::map<unsigned int, std::string>  HttpSession::http_status_table = boost::assign::map_list_of(200, "200 OK") (404, "404 Not Found") (413, "413 Request Entity Too Large") (500, "500 Server Error") (501, "501 Not Implemented") (505, "505 HTTP Version Not Supported"); 
using namespace boost;

void HttpSession::start_handling() 
{    
	boost::asio::async_read_until(socket(),
		m_request, "\r\n",
		boost::bind(&HttpSession::on_request_line_received, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
} 

void HttpSession::on_request_line_received(const boost::system::error_code& ec, std::size_t bytes_transferred) 
{    
	if (ec != 0) 
	{   
		SimpleLogger::GetInstance().WriteLog(std::string("on_request_line_received | Error occured! Error code = " + std::to_string(ec.value()) + ". Message: " + ec.message()));
		if (ec == asio::error::not_found) 
		{        
			// No delimiter has been found in the        
			// request message.
			SimpleLogger::GetInstance().WriteLog("on_request_line_received | Error occured! m_response_status_code = 413");
			m_response_status_code = 413;        
			send_response();
			return;      
		} else 
		{        
			// In case of any other error �        
			// close the socket and clean up.        
			SimpleLogger::GetInstance().WriteLog("on_request_line_received | Error occured! on_finish()");
			on_finish();        
			return;
		}
	}
	SimpleLogger::GetInstance().WriteLog("on_request_line_received() received");
	// Parse the request line.    
	std::string request_line;    
	std::istream request_stream(&m_request);    
	std::getline(request_stream, request_line, '\r');    
	// Remove symbol '\n' from the buffer.    
	request_stream.get();
	// Parse the request line.    
	std::string request_method;    
	std::istringstream request_line_stream(request_line);    
	request_line_stream >> request_method;
	// We only support GET method.    
	if (request_method.compare("GET") != 0) 
	{      // Unsupported method.      
		SimpleLogger::GetInstance().WriteLog("Error occured! request_method.compare(GET) != 0 //// Unsupported method.   ");
		m_response_status_code = 501;      
		send_response();
		return;    
	}
	request_line_stream >> m_requested_resource;
	std::size_t found = m_requested_resource.find('?');
	if (found != std::string::npos)
	{
		char buffer[255] = {0};
		m_requested_resource.copy(buffer, found, 0);
		m_requested_resource = buffer;
	}
	SimpleLogger::GetInstance().WriteLog(request_line);
	std::string request_http_version;    
	request_line_stream >> request_http_version;
	if ((request_http_version.compare("HTTP/1.0") != 0) && (request_http_version.compare("HTTP/1.1") != 0))  // todo: 
	{      // Unsupported HTTP version or bad request.      
		SimpleLogger::GetInstance().WriteLog("Error occured! HTTP/1.0 //// Unsupported HTTP version or bad request..   ");
		m_response_status_code = 505;      
		send_response();
		return;    
	}
	// At this point the request line is successfully    
	// received and parsed. Now read the request headers.    
	asio::async_read_until(socket(), m_request, "\r\n\r\n",      
		boost::bind(&HttpSession::on_headers_received, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

	return;  
}

void HttpSession::on_headers_received(const boost::system::error_code& ec, std::size_t bytes_transferred)    
{    
	if (ec != 0) 
	{      
		SimpleLogger::GetInstance().WriteLog(std::string("on_headers_received | Error occured! Error code = " + std::to_string(ec.value()) + ". Message: " + ec.message()));
		if (ec == asio::error::not_found) 
		{        
			// No delimiter has been fonud in the        
			// request message.
			SimpleLogger::GetInstance().WriteLog("on_request_line_received | Error occured! m_response_status_code = 413");
			m_response_status_code = 413;        
			send_response();        
			return;      
		} else {        
			// In case of any other error - close the        
			// socket and clean up.        
			SimpleLogger::GetInstance().WriteLog("on_request_line_received | Error occured! on_finish()");
			on_finish();        
			return;      
		}    
	}
	std::istream request_stream(&m_request);    
	std::string header_name, header_value;
	while (!request_stream.eof()) 
	{      
		std::getline(request_stream, header_name, ':');      
		if (!request_stream.eof()) 
		{        
			std::getline(request_stream, header_value, '\r');
			// Remove symbol \n from the stream.        
			request_stream.get();        
			m_request_headers[header_name] = header_value;      
		}    
	}
	// Now we have all we need to process the request.    
	process_request();    
	send_response();
	return;  
} 

void HttpSession::process_request() 
{    
	SimpleLogger::GetInstance().WriteLog("HttpSession::process_request()");
	// Read file.    
	std::string resource_file_path = m_sDir + m_requested_resource;
	SimpleLogger::GetInstance().WriteLog("HttpSession::process_request() - " + resource_file_path);
	if (!boost::filesystem::exists(resource_file_path)) 
	{      
		// Resource not found.      
		SimpleLogger::GetInstance().WriteLog("HttpSession::process_request() - Resource not found");
		m_response_status_code = 404;
		return;    
	}
	std::ifstream resource_fstream(resource_file_path, std::ifstream::binary);
	if (!resource_fstream.is_open()) 
	{      
		// Could not open file.       
		// Something bad has happened.      
		SimpleLogger::GetInstance().WriteLog("HttpSession::process_request() - Could not open file");
		m_response_status_code = 500;
		return;    
	}
	// Find out file size.    
	resource_fstream.seekg(0, std::ifstream::end);    
	m_resource_size_bytes = static_cast<std::size_t>(resource_fstream.tellg());
	m_resource_buffer.reset(new char[m_resource_size_bytes]);
	resource_fstream.seekg(std::ifstream::beg);    
	resource_fstream.read(m_resource_buffer.get(), m_resource_size_bytes);
	m_response_headers += std::string("content-length") + ": " + std::to_string(m_resource_size_bytes) + "\r\n";  
} 

void HttpSession::send_response()  
{    
	SimpleLogger::GetInstance().WriteLog("HttpSession::send_response()");
	m_sock.shutdown(asio::ip::tcp::socket::shutdown_receive);
	auto status_line = http_status_table.at(m_response_status_code);
	m_response_status_line = std::string("HTTP/1.0 ") + status_line + "\r\n";
	m_response_headers += "\r\n";
	std::vector<asio::const_buffer> response_buffers;    
	response_buffers.push_back(asio::buffer(m_response_status_line)); 
	if (m_response_headers.length() > 0) 
	{      
		response_buffers.push_back(asio::buffer(m_response_headers));    
	}
	if (m_resource_size_bytes > 0) 
	{      
		response_buffers.push_back(asio::buffer(m_resource_buffer.get(), m_resource_size_bytes));    
	}
	// Initiate asynchronous write operation. 
	asio::async_write(socket(), response_buffers,       
		boost::bind(&HttpSession::on_response_sent, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void HttpSession::on_response_sent(const boost::system::error_code& ec, std::size_t bytes_transferred) 
{    
	if (ec != 0) 
	{      
		SimpleLogger::GetInstance().WriteLog(std::string("on_response_sent | Error occured! Error code = " + std::to_string(ec.value()) + ". Message: " + ec.message()));
	}
	m_sock.shutdown(asio::ip::tcp::socket::shutdown_both);
	on_finish();  
}