#ifndef __NET__TFTP__CLIENT__H
#define __NET__TFTP__CLIENT__H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>

#include "types.h"
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
namespace net
{
	namespace tftp
	{
		class client
		{
		public:
			typedef udp::resolver resolver;
			typedef udp::endpoint endpoint;
			typedef udp::socket socket_t;

			enum error
			{
				err_success = 0,
				err_server_is_not_established = -1,
				err_client_is_busy = -2
			};
			enum state
			{
				st_idle = 0,
				st_read = 1,
				st_write = 2,
				st_resolve = 3
			};
			client(boost::asio::io_context& ctx) :m_resolver(ctx), m_socket(ctx), m_state(st_idle) {
			}
			//	Establish tftp-server for future operations.
			//	This operation SHOULD be called if the state of the client is 'idle'.
			//	Params:
			//	@server - IPv4 server address
			//	@path - path to file on the server
			//	@mode - transfer mode
			int resolve(const std::string& server)
			{
				//p	read/write operation is executed
				if (m_state != st_idle)
					return err_client_is_busy;
				m_state = st_resolve;
				//	use local resolver to get server endpoint
				m_resolver.async_resolve(server, "http",
					boost::bind(&client::handle_resolve, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::results));
			}
			//	Read data from tftp-server. Server should be established before.
			int read(const std::string& path, tftp::en_mode mode = octet) {
				if (m_state != st_idle)
					return err_client_is_busy;
				if (!m_server.size())
					return err_server_is_not_established;
				m_state = st_read;

				//	create request packet
				std::string str;
				if (mode == octet)
					str = "octet";
				else
					str = "netascii";
				size_t output_size = sizeof(unsigned short) + path.size() + str.size();
				m_data.prepare(output_size);
				std::ostream os(&m_data);
				os << (unsigned short)RRQ;
				os << path;
				os << str;
				m_data.commit(output_size);

				//	send read-request
				int flags = 0;
				m_socket.async_send_to(m_data.data(), m_server, flags,
					boost::bind(&client::handle_read, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
				return err_success;
			}
			//	Write data to tftp-server. Server should be established before.
			int write(const std::string& path, tftp::en_mode mode = octet) {
				int flags = 0;
				//	send write-request
				m_socket.async_send_to(m_data.data(), m_server, flags,
					boost::bind(&client::handle_read, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
				return err_success;
			}
			//	current tftp-server
			endpoint server()const {
				return m_server;
			}
			//	client state
			state get_state()const {
				return m_state;
			}

		private:
			void handle_resolve(const boost::system::error_code& err,
				const resolver::results_type& endpoints)
			{
				if (!err) {
					//	use only the first endpoint
					m_server = *endpoints.begin();
					std::cout << "client::handle_resolve (ok):" << std::endl;
					m_state = st_idle;
				}
				else {
					//	temporary decision (logging)
					std::cerr << "client::handle_resolve (error): " << err.message() << std::endl;
					m_state = st_idle;
				}
			}
			void handle_read(const boost::system::error_code& err,
				std::size_t bytes_transferred)
			{
				if (!err) {
					std::cout << "client::handle_read (ok): number of transferred bytes is" << bytes_transferred << std::endl;
				}
				else {
					//	temporary decision (logging)
					std::cerr << "client::handle_read (error): " << err.message() << std::endl;
				}
			}
			void handle_write(const boost::system::error_code& err,
				std::size_t bytes_transferred)
			{
				if (!err) {
					std::cout << "client::handle_write (ok): number of transferred bytes is" << bytes_transferred << std::endl;
				}
				else {
					//	temporary decision (logging)
					std::cerr << "client::handle_write (error): " << err.message() << std::endl;
				}
			}
		private:
			resolver m_resolver;
			endpoint m_server;
			streambuf m_data;
			socket_t m_socket;
			en_opcode m_opcode;
			en_mode m_mode;
			//	If operation code is WRQ then '_path' contains path to the file to be sended
			//	to the server. If operation code is RRQ then '_path' contains path to the file
			//	to be received from the server.
			std::string m_path;
			state m_state;
		};
	}	//	tftp
}
#endif//__NET__TFTP__CLIENT__H