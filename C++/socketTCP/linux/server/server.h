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
#include <fstream>
#include <iostream>
#include "types.h"

#define SOCKET_TCP_PORT 1205		 		// TCPServer Port
#define BIND_ADDR INADDR_ANY		  	// bind address
#define FILE_PATH "receive_file"  	// receive file path

class socketTCPServer
{
	// property
	I4 sockSrv;	// server socket
	I4 sockConn;	// connect socket

	bool isConnected; 	// client connect flag
	bool isRun;		// thread run flag

	void saveData(const I1 *data, const U4 &u4_dataSize);
	void saveFixedStruct(const I1 *data,const U4 &u4_dataSize);
	void saveMutableStruct(const I1 *data,const U4 &u4_dataSize);

public:

	socketTCPServer():
		isConnected(false),
		isRun(true)
	{}

	~socketTCPServer() = default;

	void run();
	void quit();

};

#endif // _SERVER_H_
