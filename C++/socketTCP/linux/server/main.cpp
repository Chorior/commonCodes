#include "server.h"

int main()
{
	//std::cout << sizeof(F4) << std::endl;
	//std::cout << sizeof(FIXED_LENGTH_STRUCT) << std::endl;

	socketTCPServer server;
	server.run();

	return EXIT_SUCCESS;
}
