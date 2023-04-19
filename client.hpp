#ifndef __NET__TFTP__CLIENT__H
#define __NET__TFTP__CLIENT__H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>

#define TFTP_PORT 69
#define TFTP_RECV_BUF_SIZE 1024

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace net
{
	namespace tftp
	{
		enum en_opcode
		{
			RRQ = 1,
			WRQ = 2,
			DATA = 3,
			ACK = 4,
			ERR = 5
		};
		
		enum en_mode
		{
			netascii,
			octet
		};
		
		class client
		{
		public:
			typedef udp::resolver resolver_t;
			typedef udp::endpoint endpoint_t;
			typedef udp::socket socket_t;

			enum error
			{
				err_success = 0,
				err_server_is_not_established = -1,
				err_client_is_busy = -2,
				err_unable_to_send
			};
			enum state
			{
				st_idle = 0,
				st_read = 1,
				st_write = 2,
				st_resolve = 3
			};
			//	Param:
			//	@server - ipv4-address in dot-notation
			client(boost::asio::io_context& ctx,std::string server=std::string()) :m_resolver(ctx), m_socket(ctx), m_state(st_idle) {
				if(server.empty())
					return;
				address addr=address::from_string(server);
				if(addr.is_unspecified()){
					std::cout<<"client::client: invalid address"<<std::endl;
					return;
				}
				m_server=endpoint_t(addr,TFTP_PORT);
				m_socket.open(boost::asio::ip::udp::v4());
			}
			//	check whether the server is specified
			bool is_server_established()const{
				return !m_server.address().is_unspecified();
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
				m_resolver.async_resolve(server, "tftp",
					boost::bind(&client::handle_resolve, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::results));
			}
			//	Read data from tftp-server. Server should be established before.
			int read(const std::string& path, tftp::en_mode mode = octet) {
				if (m_state != st_idle)
					return err_client_is_busy;
				if (!this->is_server_established())
					return err_server_is_not_established;
				m_state = st_read;

				//	create request packet
				std::string str;
				if (mode == octet)
					str = "octet\0";
				else
					str = "netascii\0";
				size_t bytes_to_send = sizeof(unsigned short) + path.size() + str.size();
				std::cout<<"client::read: bytes to send is "<<bytes_to_send<<std::endl;
				m_data.prepare(bytes_to_send);
				std::ostream os(&m_data);
				os << (unsigned short)RRQ;
				os << path;
				os << str;
				m_data.commit(bytes_to_send);

				//	Send read-request. Blocking call is used.
				//	ToDo: use flags to control sending time at blocking call
				int flags = 0;
				size_t bytes_transferred=m_socket.send_to(m_data.data(),m_server,flags);
				if(bytes_transferred!=bytes_to_send){
					std::cerr<<"client::read: unable to send request ("<<bytes_transferred<<")"<<std::endl;
					m_state=st_idle;
					return err_unable_to_send;
				}else{
					//	debug-output
					std::cout<<"client::read: request is sended"<<std::endl;
				}
				//	remove data from input sequence
				m_data.consume(m_data.size());
				m_socket.async_receive_from(m_data.prepare(TFTP_RECV_BUF_SIZE),m_server,flags,
					boost::bind(&client::handle_read,this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
				return err_success;
			}
			//	Write data to tftp-server. Server should be established before.
			int write(const std::string& path, tftp::en_mode mode = octet) {
				int flags = 0;
				//	send write-request (not realized yet)
				m_socket.async_send_to(m_data.data(), m_server, flags,
					boost::bind(&client::handle_read, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
				return err_success;
			}
			//	current tftp-server
			endpoint_t server()const {
				return m_server;
			}
			//	client state
			state get_state()const {
				return m_state;
			}

		private:
			void handle_resolve(const boost::system::error_code& err,
				const resolver_t::results_type& endpoints)
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
					std::cout << "client::handle_read: bytes transferred - " << bytes_transferred << std::endl;
					std::cout << "client::handle_read: data size - " << m_data.size() << std::endl;
				}
				else {
					//	temporary decision (logging)
					std::cerr << "client::handle_read: error is " << err.message() << std::endl;
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
			resolver_t m_resolver;
			endpoint_t m_server;
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
	typedef tftp::client tftp_client;
}
#endif//__NET__TFTP__CLIENT__H
