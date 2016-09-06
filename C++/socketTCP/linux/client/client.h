#ifndef _CLIENT_H_
#define _CLIENT_H_

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
#include <arpa/inet.h>
#include "types.h"

#define SOCKET_TCP_PORT 1205		 		// TCPServer Port
#define BUFFER_SIZE USHRT_MAX      	// send buffer size
#define SERVER_ADDR "127.0.0.1"		  // server address

class socketTCPClient
{
  // property
  I4 sockClient;
  I1 send_buf[BUFFER_SIZE];

  bool isConnected; // connect flag
  bool isRun;       // run flag

  bool connectToServer(); // build socket to server

public:
  socketTCPClient();
  ~socketTCPClient() = default;

  void quit();
  void sendFile(const I1 *path);
  void sendData(const I1* data,U2 u2_dataSize);

};

#endif // _CLIENT_H_
