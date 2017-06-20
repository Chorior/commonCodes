#ifndef _SERVER_H_
#define _SERVER_H_

//#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Winsock2.h>
#include <windows.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <memory>
#include <stdio.h>
#include "types.h"

#pragma  comment(lib,"ws2_32.lib")

#define SOCKET_TCP_PORT 1205		// TCPServer Port
#define BIND_ADDR INADDR_ANY		// bind address
#define FILE_PATH "receive_file"	// receive file path

class socketTCPServer
{
	// property
	SOCKET sockSrv;		// server socket
	SOCKET sockConn;	// connect socket

	bool isConnected; 	// client connect flag
	bool isRun;			// thread run flag

	void buildSocket();
	void saveFile(const I1 *data, const U4 &u4_dataSize);
	std::unique_ptr<I1 []> saveData(U4 &u4_dataSize);

public:

	socketTCPServer():
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
