#include "types.h"

#include <boost/asio/streambuf.hpp>

#include <iostream>

#define BOOST_EXCEPTION_DISABLE
#define BOOST_NO_EXCEPTIONS

using namespace boost;
using namespace boost::asio;

int main(int argc,char * argv[]){
	streambuf data;
	std::ostream os(&data);
	std::string path("test.txt\0");
	const char * mode="netascii\0";
	unsigned short opcode = net::tftp::RRQ;
	size_t output_size = sizeof(opcode) + strlen(path.c_str()) + strlen(mode);
	data.prepare(output_size);
	os << opcode;
	os << path;
	os << mode;
	data.commit(output_size);
	size_t size = data.size();
	printf("input sequence: size=%u; data=\n", size);
	char *p = (char*)data.data().data();
	for (size_t i = 0; i < size; i++) {
		printf("%.2x ", *p++);
	}
	printf("\n");
	printf("%s\n",(char*)data.data().data());
	return 0;
}