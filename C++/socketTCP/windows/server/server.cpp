#include "server.h"

void socketTCPServer::buildSocket()
{
	std::cout << "build socket\n";

	WSADATA wsaData;
	int SocketStartRet = WSAStartup(0x0202, &wsaData);
	if (SocketStartRet != 0)
	{
		perror("WSAStartup");
		return;
	}

	sockSrv = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockSrv)
	{
		std::cout << "socket failed!\n";
		return;
	}

	SOCKADDR_IN addrSrv;
	memset(&addrSrv,0,sizeof(addrSrv));
	addrSrv.sin_addr.S_un.S_addr = htonl(BIND_ADDR);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(SOCKET_TCP_PORT);

	// http://www.cnblogs.com/qq78292959/archive/2012/03/22/2411390.html
	// reuse local address and port
	I4 on = 1;
	if(-1 == setsockopt(sockSrv,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on)))
	{
		perror("bind");
		closesocket(sockSrv);
		return;
	}

	if(-1 == bind(sockSrv,(SOCKADDR *)&addrSrv,sizeof(SOCKADDR)))
	{
		perror("bind");
		closesocket(sockSrv);
		return;
	}

	if(-1 == listen(sockSrv,5))
	{
		perror("listen");
		closesocket(sockSrv);
		return;
	}
}

std::unique_ptr<I1 []> socketTCPServer::saveData(U4 &u4_dataSize)
{

	if(!isConnected)
	{
		SOCKADDR_IN addrClient;
		int clientAddrLen = sizeof(SOCKADDR);

		sockConn = accept(sockSrv,(SOCKADDR*)&addrClient,&clientAddrLen);
		if(-1 == sockConn)
		{
			perror("accept");
			closesocket(sockSrv);
			return nullptr;
		}

		isConnected = true;
	}

	// receive data from socket and store it in recv_buf
	try
	{
		std::cout << "receive\n";

		char recv_sizeBuf[10] = { 0 };
		U4 u4_recv = recv(sockConn,recv_sizeBuf,10,0);
		if(u4_recv > 0)
		{
			if(0 == strncmp(recv_sizeBuf,"verify",6))
			{
				memcpy(&u4_dataSize,recv_sizeBuf + 6, 4);
				std::cout << "data size = "
						<< u4_dataSize
						<< std::endl;

				I1 buffer[] = { "OK" };
				if(-1 == send(sockConn,buffer,sizeof(buffer),0))
				{
					perror("send OK");
					quit();
					return nullptr;
				}

				U4 offset = 0;
				U4 u4_dataSizeTmp = u4_dataSize;

				std::unique_ptr<I1 []> recv_buf(new (std::nothrow)I1[u4_dataSize + 1]);
				if(nullptr == recv_buf.get())
				{
					std::cout << "the size of data is too large!\n";
					quit();
					return nullptr;
				}

				memset(recv_buf.get(), 0, u4_dataSize + 1);
				u4_recv = recv(sockConn,recv_buf.get(),u4_dataSizeTmp,0);
				while(u4_dataSizeTmp > u4_recv)
				{
					if(-1 == u4_recv)
					{
						perror("recv");
						quit();
						return nullptr;
					}
					else
					{
						offset += u4_recv;
						u4_dataSizeTmp -= u4_recv;
						u4_recv = recv(sockConn,recv_buf.get() + offset,u4_dataSizeTmp,0);
					}
				}

				{
					isConnected = false;
					closesocket(sockConn);
				}
				return recv_buf;
			}
			else
			{
				std::cout << "received wrong data!\n";
				quit();
				return nullptr;
			}
		}
		else
		{
			perror("recv");
			quit();
			return nullptr;
		}
	}catch(...)
	{
		perror("error");
		quit();
		return nullptr;
	}
}

void socketTCPServer::run()
{
	while(isRun)
	{
		std::unique_ptr<I1 []> recv_buf;
		U4 u4_dataSize = 0;

		recv_buf = saveData(u4_dataSize);
		if(nullptr == recv_buf)
		{
			std::cout << "saveData() error!\n";
			return;
		}

		//save something
		saveFile(recv_buf.get(),u4_dataSize);		
	}
}

void socketTCPServer::saveFile(const I1 *data, const U4 &u4_dataSize)
{
	using namespace std;
	ofstream fout(FILE_PATH,ios_base::out | ios_base::binary);
	if(!fout.is_open())
	{
		perror("ofstream open");
		return;
	}

	fout.write(data,u4_dataSize);
	fout.close();
}

void socketTCPServer::quit()
{
	isRun = false;
	isConnected = false;
	closesocket(sockConn);
	closesocket(sockSrv);
	WSACleanup();
}
