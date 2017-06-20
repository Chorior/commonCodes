#include "client.h"

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		socketTCPClient client;
		client.sendFile(argv[1]);
		client.quit();
	}
	return EXIT_SUCCESS;
}
