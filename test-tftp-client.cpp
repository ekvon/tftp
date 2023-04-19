#include "client.h"

using namespace net;
using namespace net::tftp;

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
int main(int argc,char * argv[]){
	if (argc < 2) {
		std::cerr<<"usage: tftp-client server\n"<<std::endl;
		return -1;
	}
	typedef net::tftp_client tftp_client;
	io_context io;
	tftp_client client(io,argv[1]);
	if(!client.is_server_established()){
		std::cerr<<"client-test: invalid server name"<<std::endl;
		return -1;
	}
	tftp_client::endpoint_t ep = client.server();
	printf("size=%ld,addr=%s,port=%u\n", ep.size(), ep.address().to_string().c_str(), ep.port());
	client.read("test.txt");
	io.run();
	return 0;
}
