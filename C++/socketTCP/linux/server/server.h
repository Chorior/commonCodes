#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <climits>
#include "types.h"

#define SOCKET_TCP_PORT 1205		 		// TCPServer Port
#define BUFFER_SIZE USHRT_MAX			 	// receive buffer size
#define BIND_ADDR INADDR_ANY		  	// bind address
#define FILE_PATH "receive_file"  	// receive file path

class socketTCPServer
{
	// property
	I4 sockSrv;	// server socket
	I4 sockConn;	// connect socket

	I1 recv_buf[BUFFER_SIZE]; // buffer area to receive data

	bool isConnected; 	// client connect flag
	bool isRun;		// thread run flag

	void saveData(I1 *data, U2 dataSize);

public:

	socketTCPServer();
	~socketTCPServer() = default;

	void run();
	void quit();

};

#endif // _SERVER_H_
