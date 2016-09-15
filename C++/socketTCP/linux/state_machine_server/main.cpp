#include <thread>
#include "server.h"
#include "save_pocket.h"

int main()
{
	save_pocket save_machine;
	socketTCPServer server(save_machine.get_sender());

	//std::thread server_thread(&socketTCPServer::run,&server);
	std::thread save_thread(&save_pocket::run,&save_machine);

	server.run();

	save_machine.done();
	save_thread.join();

	return EXIT_SUCCESS;
}
