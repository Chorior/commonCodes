#include "server.h"

int main()
{
	socketTCPServer server;
	server.run();
	
	return EXIT_SUCCESS;
}
