#include "server.h"
#include "state_machine.h"

void socketTCPServer::buildSocket()
{
	std::cout << "build socket\n\n";

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

	// http://www.cnblogs.com/qq78292959/archive/2012/03/22/2411390.html
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
}

std::unique_ptr<I1 []> socketTCPServer::saveData(U4 &u4_dataSize,U2 &u2_dataType)
{

	if(!isConnected)
	{
		struct sockaddr_in addrClient;
		socklen_t clientAddrLen = 1;

		sockConn = accept(sockSrv,(struct sockaddr *)&addrClient,&clientAddrLen);
		if(-1 == sockConn)
		{
			perror("accept");
			close(sockSrv);
			return nullptr;
		}

		isConnected = true;
	}

	// receive data from socket and store it in recv_buf
	try
	{
		//std::cout << "received\n";

		char recv_sizeBuf[12] = { 0 };
		U4 u4_recv = recv(sockConn,recv_sizeBuf,sizeof(recv_sizeBuf),0);
		if(u4_recv > 0)
		{
			if(0 == strncmp(recv_sizeBuf,"verify",6))
			{
				memcpy(&u4_dataSize,recv_sizeBuf + 6, 4);
				std::cout << "data size = "
						<< u4_dataSize
						<< std::endl;
				memcpy(&u2_dataType,recv_sizeBuf + 10, 2);

				I1 buffer[] = { "OK" };
				if(-1 == send(sockConn,buffer,sizeof(buffer),0))
				{
					perror("send OK");
					quit();
					return nullptr;
				}

				U4 offset = 0;
				U4 u4_dataSizeTmp = u4_dataSize;

				std::unique_ptr<I1 []> recv_buf(new (std::nothrow)I1[u4_dataSize]);
				if(nullptr == recv_buf.get())
				{
					std::cout << "the size of data is too large!\n";
					quit();
					return nullptr;
				}

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
					close(sockConn);
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
		U2 u2_dataType = 0;

		recv_buf = saveData(u4_dataSize, u2_dataType);
		if(nullptr == recv_buf)
		{
			std::cout << "saveData() error!\n";
			return;
		}

		//save something
		if(SENDFILE == u2_dataType)
		{
			savePocket.send(file_received(recv_buf.get(),u4_dataSize));
		}
		else if(FIXED_STRUCT == u2_dataType)
		{
			savePocket.send(fixed_struct_received(recv_buf.get(),u4_dataSize));
		}
		else if(MUTABLE_STRUCT == u2_dataType)
		{
			savePocket.send(mutable_struct_received(recv_buf.get(),u4_dataSize));
		}
		else
		{
			std::cout << "receive error!\n";
			return;
		}
	}
}

void socketTCPServer::quit()
{
	isRun = false;
	isConnected = false;
	close(sockConn);
	close(sockSrv);
}
