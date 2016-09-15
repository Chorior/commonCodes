#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include "types.h"
#include "state_machine.h"

#define SOCKET_TCP_PORT 1205		// TCPServer Port
#define BIND_ADDR INADDR_ANY		// bind address

class socketTCPServer
{
	messaging::sender savePocket;

	// property
	I4 sockSrv;	// server socket
	I4 sockConn;	// connect socket

	bool isConnected; 	// client connect flag
	bool isRun;		// thread run flag

	void buildSocket();
	std::unique_ptr<I1 []> saveData(U4 &u4_dataSize,U2 &u2_dataType);

public:

	socketTCPServer(messaging::sender savePocket_):
		savePocket(savePocket_),
		isConnected(false),
		isRun(true)
	{
			buildSocket();
	}

	~socketTCPServer()
	{
		quit();
	}

	void run();
	void quit();

};

#endif // _SERVER_H_
