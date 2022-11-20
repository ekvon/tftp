#pragma once

// TODO: установите здесь ссылки на дополнительные заголовки, требующиеся для программы.
#include <boost/asio.hpp>
#include <boost/exception/diagnostic_information.hpp>

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
		//	boost-types
		typedef typename basic_endpoint<boost::asio::ip::udp> endpoint;
		typedef basic_datagram_socket<boost::asio::ip::udp> socket_t;
		typedef basic_resolver<boost::asio::ip::tcp> resolver;
	}
}