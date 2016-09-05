#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <iostream>

#define SOCKET_TCP_PORT 1205				// TCPServer Port
#define BUFFER_SIZE 1024 * 64 - 1		// receive buffer size
#define BIND_ADDR INADDR_ANY				// bind ip address
#define FILE_PATH "receive_file"		// receive file path

class socketTCPServer
{
	// property
	int sockSrv;	// server socket
	int sockConn;	// connect socket

	char recv_buf[BUFFER_SIZE]; // buffer area to receive data

	bool isConnected; 	// client connect flag
	bool isRun;		// thread run flag

	void saveData(char *data, unsigned int dataSize);

public:

	socketTCPServer();
	~socketTCPServer() = default;

	void run();
	void quit();

};

#endif
