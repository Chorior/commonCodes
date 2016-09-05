#include "server.h"

socketTCPServer::socketTCPServer():
	isConnected(false),
	isRun(true)
{
	memset(recv_buf,0,sizeof(recv_buf));
}

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
	int on = 1;
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

			int i4_recv = recv(sockConn,recv_buf,8,0);
			if(i4_recv > 0)
			{
				if(0 == strncmp(recv_buf,"verify",6))
				{
					unsigned int dataSize = 0;
					memcpy(&dataSize,recv_buf + 6, 2);
					if(dataSize > BUFFER_SIZE)
					{
						std::cout << "data size is too large!\n";
						quit();
						return;
					}
					std::cout << "data size = "
										<< dataSize
										<< std::endl;

					char buffer[] = { "OK" };
					if(-1 == send(sockConn,buffer,sizeof(buffer),0))
					{
						perror("send OK");
						quit();
						return;
					}

					int offset = 0;
					unsigned int dataSizeTmp = dataSize;
					memset(recv_buf,0,sizeof(recv_buf));
					i4_recv = recv(sockConn,recv_buf,dataSize,0);
					while(dataSize > i4_recv)
					{
						if(-1 == i4_recv)
						{
							perror("recv");
							quit();
							return;
						}
						else
						{
							offset += i4_recv;
							dataSize -= i4_recv;
							i4_recv = recv(sockConn,recv_buf + offset,dataSize,0);
							std::cout << "whole message = "
												<< recv_buf
												<< std::endl;
						}
					}

					//save data
					isConnected = false;
					saveData(recv_buf,dataSizeTmp);
				}
				else
				{
					std::cout << "receive wrong data!\n";
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

void socketTCPServer::saveData(char *data, unsigned int dataSize)
{
	std::cout << "saveData() called!\n";

	FILE *fp = fopen(FILE_PATH,"rb");
	if(NULL == fp)
	{
		perror("fopen");
		return;
	}

	fwrite(data,sizeof(char),dataSize,fp);
	fclose(fp);
}

void socketTCPServer::quit()
{
	isRun = false;
	close(sockConn);
	close(sockSrv);
	isConnected = false;
}
