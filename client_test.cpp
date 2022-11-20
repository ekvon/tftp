#include "client.h"
using namespace net;
using namespace net::tftp;

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
int main(int argc,char * argv[]){
	if (argc < 2) {
		fprintf(stdout,"usage: tftp-client server\n");
		return -1;
	}
	io_context ctx;
	client c(ctx);
	ctx.run();
	//	www.tftp.org (test connection)
	std::string server(argv[1]);
	printf("size=%d\n", c.server().size());
	c.resolve(server);
	while (c.get_state() != net::tftp::client::st_idle) {
		;
	}
	net::tftp::endpoint ep = c.server();
	printf("size=%d,addr=%s,port=%u\n", ep.size(), ep.address().to_string().c_str(), ep.port());
	return 0;
}