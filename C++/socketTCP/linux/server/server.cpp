#include "server.h"

void socketTCPServer::run()
{
	std::cout << "server start\n\n";

	sockSrv = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockSrv)
	{
		std::cout << "socket failed!\n";
		return;
	}

	struct sockaddr_in addrSrv;
	memset(&addrSrv,0,sizeof(addrSrv));
	addrSrv.sin_addr.s_addr = htonl(BIND_ADDR);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(SOCKET_TCP_PORT);

	//http://www.cnblogs.com/qq78292959/archive/2012/03/22/2411390.html
	// reuse local address and port
	I4 on = 1;
	if(-1 == setsockopt(sockSrv,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))
	{
		perror("bind");
		close(sockSrv);
		return;
	}

	if(-1 == bind(sockSrv,(struct sockaddr *)&addrSrv,sizeof(struct sockaddr_in)))
	{
		perror("bind");
		close(sockSrv);
		return;
	}

	if(-1 == listen(sockSrv,5))
	{
		perror("listen");
		close(sockSrv);
		return;
	}

	struct sockaddr_in addrClient;
	socklen_t clientAddrLen = 1;

	while(isRun)
	{
		if(!isConnected)
		{
			sockConn = accept(sockSrv,(struct sockaddr *)&addrClient,&clientAddrLen);
			if(-1 == sockConn)
			{
				perror("accept");
				close(sockSrv);
				return;
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
					U4 u4_dataSize = 0;
					memcpy(&u4_dataSize,recv_sizeBuf + 6, 4);
					std::cout << "data size = "
										<< u4_dataSize
										<< std::endl;

					I1 buffer[] = { "OK" };
					if(-1 == send(sockConn,buffer,sizeof(buffer),0))
					{
						perror("send OK");
						quit();
						return;
					}

					U4 offset = 0;
					U4 u4_dataSizeTmp = u4_dataSize;
					I1 *recv_buf = new I1[u4_dataSize];

					u4_recv = recv(sockConn,recv_buf,u4_dataSize,0);
					while(u4_dataSize > u4_recv)
					{
						if(-1 == u4_recv)
						{
							perror("recv");
							quit();
							return;
						}
						else
						{
							offset += u4_recv;
							u4_dataSize -= u4_recv;
							u4_recv = recv(sockConn,recv_buf + offset,u4_dataSize,0);
							std::cout << "whole message = "
												<< recv_buf
												<< std::endl;
						}
					}

					//save data
					isConnected = false;
					saveData(recv_buf,u4_dataSizeTmp);

					delete recv_buf;
				}
				else
				{
					std::cout << "received wrong data!\n";
					quit();
					return;
				}
			}
			else
			{
				perror("recv");
				quit();
				return;
			}
		}catch(...)
		{
			perror("error");
			quit();
			return;
		}
	}
}

void socketTCPServer::saveData(I1 *data, U4 u4_dataSize)
{
	std::cout << "saveData() called!\n";

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
	close(sockConn);
	close(sockSrv);
}
