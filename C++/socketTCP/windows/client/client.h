#ifndef _CLIENT_H_
#define _CLIENT_H_

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Winsock2.h>
#include <windows.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <climits>
#include <algorithm>
#include <memory>
#include "types.h"

#pragma  comment(lib,"ws2_32.lib")

#define SOCKET_TCP_PORT 1205		// TCPServer Port
#define SERVER_ADDR "127.0.0.1"		// server address

class socketTCPClient
{
	// property
	SOCKET sockClient;

	bool isConnected; // connect flag
	bool isRun;       // run flag

	bool connectToServer(); // build socket to server

	U4 arrangeString(I1 *buffer, const std::string &str);

public:
	socketTCPClient() :
		isConnected(false),
		isRun(true)
	{}

	~socketTCPClient()
	{
		quit();
	}

	void quit();
	void sendFile(const I1 *path);
	void sendData(const I1* data, const U4 &u4_dataSize);

};

#endif // _CLIENT_H_
