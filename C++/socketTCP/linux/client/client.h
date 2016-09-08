#ifndef _CLIENT_H_
#define _CLIENT_H_

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
#include <climits>
#include <arpa/inet.h>
#include <algorithm>
#include <memory>
#include "types.h"

#define SOCKET_TCP_PORT 1205		 		// TCPServer Port
#define SERVER_ADDR "127.0.0.1"		  // server address

class socketTCPClient
{
  // property
  I4 sockClient;

  bool isConnected; // connect flag
  bool isRun;       // run flag

  bool connectToServer(); // build socket to server

  U4 arrangeString(I1 *buffer,const std::string &str);

public:
  socketTCPClient():
    isConnected(false),
    isRun(true)
  {}

  ~socketTCPClient() = default;

  void quit();
  void sendFile(const I1 *path);
  void sendData(const I1* data,const U4 &u4_dataSize);
  void sendFixedStruct(const FIXED_LENGTH_STRUCT &fixedStruct);
  void sendMutableStruct(const MUTABLE_LENGTH_STRUCT &mutableStruct);

};

#endif // _CLIENT_H_
